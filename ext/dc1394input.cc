/* HornetsEye - Computer Vision with Ruby
 Copyright (C) 2006, 2007, 2008, 2009 Jan Wedekind

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#ifndef NDEBUG
#include <iostream>
#endif
#include <iomanip>
#include "rubytools.hh"
#include "dc1394input.hh"

using namespace boost;
using namespace std;

VALUE DC1394Input::cRubyClass = Qnil;

DC1394Input::DC1394Input( DC1394Ptr dc1394, int node, dc1394speed_t speed,
                          DC1394SelectPtr select )
  throw (Error):
  m_dc1394( dc1394 ), m_node( node ), m_camera( NULL )
{
  dc1394camera_list_t *list = NULL;
  try {
    dc1394error_t err;
    err = dc1394_camera_enumerate( dc1394->get(), &list );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Failed to enumerate cameras: "
                << dc1394_error_get_string( err ) );
    ERRORMACRO( list->num > 0, Error, , "Could not find a single digital camera on "
                "the firewire bus. Please check, whether the kernel modules "
                "'ieee1394','raw1394' and 'ohci1394' are loaded and whether you "
                "have read/write permission on \"/dev/raw1394\". Also make sure "
                "that the camera is connected and powered up." );
    ERRORMACRO( node >= 0 && node < list->num, Error, ,
                "Camera node number " << node << " out of range. The raange is "
                "[ 0; " << list->num << " )" );
    m_camera = dc1394_camera_new( dc1394->get(), list->ids[ node ].guid );
    dc1394_camera_free_list( list ); list = NULL;
    ERRORMACRO( m_camera != NULL, Error, , "Failed to initialise camera node "
               << node << " (guid 0x" << setbase( 16 ) << list->ids[ node ].guid
               << setbase( 10 ) << ")" );
    dc1394video_modes_t videoModes;
    err = dc1394_video_get_supported_modes( m_camera, &videoModes );
    for ( int i=0; i<videoModes.num; i++ ) {
      if ( !dc1394_is_video_mode_scalable( videoModes.modes[i] ) ) {
        dc1394video_mode_t videoMode = videoModes.modes[i];
        dc1394color_coding_t coding;
        dc1394_get_color_coding_from_video_mode( m_camera, videoMode, &coding );
        unsigned int width, height;
        dc1394_get_image_size_from_video_mode( m_camera, videoMode, &width, &height );
        select->add( coding, width, height );
      } else {
        ERRORMACRO( false, Error, , "Scalable video modes not implemented yet" );
      };
    };
    int selection = select->make();
    ERRORMACRO( selection >= 0 && selection < videoModes.num, Error, ,
                "Index of selected video mode out of range" );
    dc1394video_mode_t videoMode = videoModes.modes[ selection ];
    err = dc1394_video_set_iso_speed( m_camera, speed );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error setting iso speed: "
                << dc1394_error_get_string( err ) );
    err = dc1394_video_set_mode( m_camera, videoMode );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Failure setting video mode: "
                << dc1394_error_get_string( err ) );
    // frame rate ...
    err = dc1394_capture_setup( m_camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Could not setup camera: "
                << dc1394_error_get_string( err ) );
    err = dc1394_video_set_transmission( m_camera, DC1394_ON );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Could not start camera iso "
                "transmission: " << dc1394_error_get_string( err ) );
  } catch ( Error &e ) {
    if ( list != NULL ) dc1394_camera_free_list( list );
    close();
    throw e;
  };
}

DC1394Input::~DC1394Input(void)
{
  close();
}

void DC1394Input::close(void)
{
  if ( m_camera != NULL ) {
    dc1394_video_set_transmission( m_camera, DC1394_OFF );
    dc1394_capture_stop( m_camera );
    dc1394_camera_set_power( m_camera, DC1394_OFF );
    dc1394_camera_free( m_camera );
    m_camera = NULL;
  };
  m_dc1394.reset();
}

string DC1394Input::inspect(void) const
{
  ostringstream s;
  s << "DC1394Input( '" << m_node << "' )";
  return s.str();
}

bool DC1394Input::status(void) const
{
  return true;
}

VALUE DC1394Input::registerRubyClass( VALUE module )
{
  cRubyClass = rb_define_class_under( module, "DC1394Input", rb_cObject );
  rb_define_const( cRubyClass,  "SPEED_100",  INT2NUM(  DC1394_ISO_SPEED_100 ) );
  rb_define_const( cRubyClass,  "SPEED_200",  INT2NUM(  DC1394_ISO_SPEED_200 ) );
  rb_define_const( cRubyClass,  "SPEED_400",  INT2NUM(  DC1394_ISO_SPEED_400 ) );
  rb_define_const( cRubyClass,  "SPEED_800",  INT2NUM(  DC1394_ISO_SPEED_800 ) );
  rb_define_const( cRubyClass,  "SPEED_1600", INT2NUM( DC1394_ISO_SPEED_1600 ) );
  rb_define_const( cRubyClass,  "SPEED_3200", INT2NUM( DC1394_ISO_SPEED_3200 ) );
  rb_define_const( cRubyClass,   "MONO8", INT2NUM(   DC1394_COLOR_CODING_MONO8 ) );
  rb_define_const( cRubyClass,  "YUV411", INT2NUM(  DC1394_COLOR_CODING_YUV411 ) );
  rb_define_const( cRubyClass,  "YUV422", INT2NUM(  DC1394_COLOR_CODING_YUV422 ) );
  rb_define_const( cRubyClass,  "YUV444", INT2NUM(  DC1394_COLOR_CODING_YUV444 ) );
  rb_define_const( cRubyClass,    "RGB8", INT2NUM(    DC1394_COLOR_CODING_RGB8 ) );
  rb_define_const( cRubyClass,  "MONO16", INT2NUM(  DC1394_COLOR_CODING_MONO16 ) );
  rb_define_const( cRubyClass,   "RGB16", INT2NUM(   DC1394_COLOR_CODING_RGB16 ) );
  rb_define_const( cRubyClass, "MONO16S", INT2NUM( DC1394_COLOR_CODING_MONO16S ) );
  rb_define_const( cRubyClass,  "RGB16S", INT2NUM(  DC1394_COLOR_CODING_RGB16S ) );
  rb_define_const( cRubyClass,    "RAW8", INT2NUM(    DC1394_COLOR_CODING_RAW8 ) );
  rb_define_const( cRubyClass,   "RAW16", INT2NUM(   DC1394_COLOR_CODING_RAW16 ) );
  rb_define_const( cRubyClass, "FRAMERATE_1_875", INT2NUM( DC1394_FRAMERATE_1_875 ) );
  rb_define_const( cRubyClass, "FRAMERATE_3_75" , INT2NUM( DC1394_FRAMERATE_3_75  ) );
  rb_define_const( cRubyClass, "FRAMERATE_7_5"  , INT2NUM( DC1394_FRAMERATE_7_5   ) );
  rb_define_const( cRubyClass, "FRAMERATE_15"   , INT2NUM( DC1394_FRAMERATE_15    ) );
  rb_define_const( cRubyClass, "FRAMERATE_30"   , INT2NUM( DC1394_FRAMERATE_30    ) );
  rb_define_const( cRubyClass, "FRAMERATE_60"   , INT2NUM( DC1394_FRAMERATE_60    ) );
  rb_define_const( cRubyClass, "FRAMERATE_120"  , INT2NUM( DC1394_FRAMERATE_120   ) );
  rb_define_const( cRubyClass, "FRAMERATE_240"  , INT2NUM( DC1394_FRAMERATE_240   ) );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 3 );
  rb_define_method( cRubyClass, "close",
                    RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "read",
                    RUBY_METHOD_FUNC( wrapRead ), 0 );
  rb_define_method( cRubyClass, "status?",
                    RUBY_METHOD_FUNC( wrapStatus ), 0 );
  return cRubyClass;
}

void DC1394Input::deleteRubyObject( void *ptr )
{
  delete (DC1394InputPtr *)ptr;
}

VALUE DC1394Input::wrapNew( VALUE rbClass, VALUE rbDC1394, VALUE rbNode,
                            VALUE rbSpeed )
{
  VALUE retVal = Qnil;
  try {
    DC1394Ptr *dc1394; Data_Get_Struct( rbDC1394, DC1394Ptr, dc1394 );
    DC1394SelectPtr select( new DC1394Select );
    DC1394InputPtr ptr( new DC1394Input( *dc1394, NUM2INT( rbNode ),
                                         (dc1394speed_t)NUM2INT( rbSpeed ),
                                         select ) );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new DC1394InputPtr( ptr ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE DC1394Input::wrapClose( VALUE rbSelf )
{
  DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
  (*self)->close();
  return rbSelf;
}

VALUE DC1394Input::wrapRead( VALUE rbSelf )
{
  VALUE retVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    FramePtr frame( (*self)->read() );
    retVal = frame->rubyObject();
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE DC1394Input::wrapStatus( VALUE rbSelf )
{
  DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
  return (*self)->status() ? Qtrue : Qfalse;
}

