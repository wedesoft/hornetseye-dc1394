/* HornetsEye - Computer Vision with Ruby
 Copyright (C) 2006, 2007, 2008, 2009, 2010 Jan Wedekind

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

DC1394Input::DC1394Input( DC1394Ptr dc1394, unsigned int node, dc1394speed_t speed,
                          DC1394SelectPtr select, bool forceFrameRate,
                          dc1394framerate_t frameRate )
  throw (Error):
  m_dc1394( dc1394 ), m_node( node ), m_camera( NULL ), m_frame( NULL )
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
    ERRORMACRO( node < list->num, Error, ,
                "Camera node number " << node << " out of range. The range is "
                "[ 0; " << list->num << " )" );
    m_camera = dc1394_camera_new( dc1394->get(), list->ids[ node ].guid );
    dc1394_camera_free_list( list ); list = NULL;
    ERRORMACRO( m_camera != NULL, Error, , "Failed to initialise camera node "
               << node << " (guid 0x" << setbase( 16 ) << list->ids[ node ].guid
               << setbase( 10 ) << ")" );
    dc1394video_modes_t videoModes;
    err = dc1394_video_get_supported_modes( m_camera, &videoModes );
    for ( unsigned int i=0; i<videoModes.num; i++ ) {
      dc1394video_mode_t videoMode = videoModes.modes[i];
      dc1394color_coding_t coding;
      dc1394_get_color_coding_from_video_mode( m_camera, videoMode, &coding );
      unsigned int width, height;
      dc1394_get_image_size_from_video_mode( m_camera, videoMode, &width, &height );
      select->add( coding, width, height );
    };
    unsigned int selection = select->make();
    ERRORMACRO( selection < videoModes.num, Error, ,
                "Index of selected video mode out of range" );
    dc1394video_mode_t videoMode = videoModes.modes[ selection ];
    err = dc1394_video_set_iso_speed( m_camera, speed );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error setting iso speed: "
                << dc1394_error_get_string( err ) );
    err = dc1394_video_set_mode( m_camera, videoMode );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Failure setting video mode: "
                << dc1394_error_get_string( err ) );
    dc1394color_coding_t coding;
    dc1394_get_color_coding_from_video_mode( m_camera, videoMode, &coding );
    switch ( coding ) {
    case DC1394_COLOR_CODING_MONO8:
      m_typecode = "UBYTE";
      break;
    case DC1394_COLOR_CODING_YUV422:
      m_typecode = "UYVY";
      break;
    case DC1394_COLOR_CODING_RGB8:
      m_typecode = "UBYTERGB";
      break;
    case DC1394_COLOR_CODING_MONO16:
      m_typecode = "USINT";
      break;
    default:
      ERRORMACRO( false, Error, , "Conversion for DC1394 colorspace " << coding
                  << " not implemented yet" );
    };
    dc1394_get_image_size_from_video_mode( m_camera, videoMode, &m_width, &m_height );
    if ( dc1394_is_video_mode_scalable( videoMode ) ) {
      ERRORMACRO( !forceFrameRate, Error, , "Cannot set framerate in format6 or "
                  "format7 mode" );
    } else {
      if ( forceFrameRate ) {
        err = dc1394_video_set_framerate( m_camera, frameRate );
        ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error setting framerate: "
                    << dc1394_error_get_string( err ) );
      } else {
        dc1394framerates_t frameRates;
        err = dc1394_video_get_supported_framerates( m_camera, videoMode, &frameRates );
        ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error querying supported frame "
                    "rates: " << dc1394_error_get_string( err ) );
        err = dc1394_video_set_framerate( m_camera,
                                          frameRates.framerates[ frameRates.num - 1 ] );
        ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error setting framerate: "
                    << dc1394_error_get_string( err ) );
      };
    };
    err = dc1394_capture_setup( m_camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT );
    ERRORMACRO( err == DC1394_SUCCESS, Error, , "Could not setup camera (video mode "
                "and framerate not supported?): "
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

FramePtr DC1394Input::read(void) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  if ( m_frame != NULL ) {
    dc1394_capture_enqueue( m_camera, m_frame );
    m_frame = NULL;
  };
  dc1394error_t err = dc1394_capture_dequeue( m_camera, DC1394_CAPTURE_POLICY_WAIT,
                                              &m_frame );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error capturing frame: "
              << dc1394_error_get_string( err ) );
  return FramePtr( new Frame( m_typecode, m_width, m_height,
                              (char *)m_frame->image ) );
}

bool DC1394Input::status(void) const
{
  return m_camera != NULL;
}

string DC1394Input::inspect(void) const
{
  ostringstream s;
  s << "DC1394Input( '" << m_node << "' )";
  return s.str();
}

unsigned int DC1394Input::featureGetValue( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  uint32_t value;
  dc1394error_t err = dc1394_feature_get_value( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error reading feature value: "
              << dc1394_error_get_string( err ) );
  return value;
}

void DC1394Input::featureSetValue( dc1394feature_t feature, unsigned int value ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394error_t err = dc1394_feature_set_value( m_camera, feature, value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error writing feature value: "
              << dc1394_error_get_string( err ) );
}

bool DC1394Input::featureIsPresent( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394bool_t value;
  dc1394error_t err = dc1394_feature_is_present( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error checking presence of feature: "
              << dc1394_error_get_string( err ) );
  return value != DC1394_FALSE;
}

bool DC1394Input::featureIsReadable( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394bool_t value;
  dc1394error_t err = dc1394_feature_is_readable( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error checking whether feature is "
              "readable: " << dc1394_error_get_string( err ) );
  return value != DC1394_FALSE;
}

bool DC1394Input::featureIsSwitchable( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394bool_t value;
  dc1394error_t err = dc1394_feature_is_switchable( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error checking whether feature is "
              "switchable: " << dc1394_error_get_string( err ) );
  return value != DC1394_FALSE;
}

dc1394switch_t DC1394Input::featureGetPower( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394switch_t value;
  dc1394error_t err = dc1394_feature_get_power( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error checking power status of "
              "feature: " << dc1394_error_get_string( err ) );
  return value;
}

void DC1394Input::featureSetPower( dc1394feature_t feature, dc1394switch_t value )
  throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394error_t err = dc1394_feature_set_power( m_camera, feature, value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error setting power status of "
              "feature: " << dc1394_error_get_string( err ) );
}

dc1394feature_modes_t DC1394Input::featureModes( dc1394feature_t feature )
  throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394feature_modes_t value;
  dc1394error_t err = dc1394_feature_get_modes( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error querying list of control modes "
              "for a feature: " << dc1394_error_get_string( err ) );
  return value;
}

dc1394feature_mode_t DC1394Input::featureModeGet( dc1394feature_t feature )
  throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394feature_mode_t value;
  dc1394error_t err = dc1394_feature_get_mode( m_camera, feature, &value );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error querying current mode of "
              "feature: " << dc1394_error_get_string( err ) );
  return value;
}

void DC1394Input::featureModeSet( dc1394feature_t feature, dc1394feature_mode_t mode )
 throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394error_t err = dc1394_feature_set_mode( m_camera, feature, mode );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error setting mode of feature: "
              << dc1394_error_get_string( err ) );
}

unsigned int DC1394Input::featureMin( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394feature_info_t info;
  info.id = feature;
  dc1394error_t err = dc1394_feature_get( m_camera, &info );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error querying minimum value of "
              "feature: " << dc1394_error_get_string( err ) );
  return info.min;
}

unsigned int DC1394Input::featureMax( dc1394feature_t feature ) throw (Error)
{
  ERRORMACRO( m_camera != NULL, Error, , "Camera device not open any more. Did you "
              "call \"close\" before?" );
  dc1394feature_info_t info;
  info.id = feature;
  dc1394error_t err = dc1394_feature_get( m_camera, &info );
  ERRORMACRO( err == DC1394_SUCCESS, Error, , "Error querying minimum value of "
              "feature: " << dc1394_error_get_string( err ) );
  return info.max;
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
  rb_define_const( cRubyClass,   "MODE_MONO8", 
                   INT2NUM(   DC1394_COLOR_CODING_MONO8 ) );
  rb_define_const( cRubyClass,  "MODE_YUV411", 
                   INT2NUM(  DC1394_COLOR_CODING_YUV411 ) );
  rb_define_const( cRubyClass,  "MODE_YUV422", 
                   INT2NUM(  DC1394_COLOR_CODING_YUV422 ) );
  rb_define_const( cRubyClass,  "MODE_YUV444", 
                   INT2NUM(  DC1394_COLOR_CODING_YUV444 ) );
  rb_define_const( cRubyClass,    "MODE_RGB8", 
                   INT2NUM(    DC1394_COLOR_CODING_RGB8 ) );
  rb_define_const( cRubyClass,  "MODE_MONO16", 
                   INT2NUM(  DC1394_COLOR_CODING_MONO16 ) );
  rb_define_const( cRubyClass,   "MODE_RGB16", 
                   INT2NUM(   DC1394_COLOR_CODING_RGB16 ) );
  rb_define_const( cRubyClass, "MODE_MONO16S", 
                   INT2NUM( DC1394_COLOR_CODING_MONO16S ) );
  rb_define_const( cRubyClass,  "MODE_RGB16S", 
                   INT2NUM(  DC1394_COLOR_CODING_RGB16S ) );
  rb_define_const( cRubyClass,    "MODE_RAW8", 
                   INT2NUM(    DC1394_COLOR_CODING_RAW8 ) );
  rb_define_const( cRubyClass,   "MODE_RAW16", 
                   INT2NUM(   DC1394_COLOR_CODING_RAW16 ) );
  rb_define_const( cRubyClass, "FRAMERATE_1_875", INT2NUM( DC1394_FRAMERATE_1_875 ) );
  rb_define_const( cRubyClass, "FRAMERATE_3_75" , INT2NUM( DC1394_FRAMERATE_3_75  ) );
  rb_define_const( cRubyClass, "FRAMERATE_7_5"  , INT2NUM( DC1394_FRAMERATE_7_5   ) );
  rb_define_const( cRubyClass, "FRAMERATE_15"   , INT2NUM( DC1394_FRAMERATE_15    ) );
  rb_define_const( cRubyClass, "FRAMERATE_30"   , INT2NUM( DC1394_FRAMERATE_30    ) );
  rb_define_const( cRubyClass, "FRAMERATE_60"   , INT2NUM( DC1394_FRAMERATE_60    ) );
  rb_define_const( cRubyClass, "FRAMERATE_120"  , INT2NUM( DC1394_FRAMERATE_120   ) );
  rb_define_const( cRubyClass, "FRAMERATE_240"  , INT2NUM( DC1394_FRAMERATE_240   ) );
  rb_define_const( cRubyClass, "FEATURE_MIN",
                   INT2NUM( DC1394_FEATURE_MIN ) );
  rb_define_const( cRubyClass, "FEATURE_BRIGHTNESS",
                   INT2NUM( DC1394_FEATURE_BRIGHTNESS ) );
  rb_define_const( cRubyClass, "FEATURE_EXPOSURE",
                   INT2NUM( DC1394_FEATURE_EXPOSURE ) );
  rb_define_const( cRubyClass, "FEATURE_SHARPNESS",
                   INT2NUM( DC1394_FEATURE_SHARPNESS ) );
  rb_define_const( cRubyClass, "FEATURE_WHITE_BALANCE",
                   INT2NUM( DC1394_FEATURE_WHITE_BALANCE ) );
  rb_define_const( cRubyClass, "FEATURE_HUE",
                   INT2NUM( DC1394_FEATURE_HUE ) );
  rb_define_const( cRubyClass, "FEATURE_SATURATION",
                   INT2NUM( DC1394_FEATURE_SATURATION ) );
  rb_define_const( cRubyClass, "FEATURE_GAMMA",
                   INT2NUM( DC1394_FEATURE_GAMMA ) );
  rb_define_const( cRubyClass, "FEATURE_SHUTTER",
                   INT2NUM( DC1394_FEATURE_SHUTTER ) );
  rb_define_const( cRubyClass, "FEATURE_GAIN",
                   INT2NUM( DC1394_FEATURE_GAIN ) );
  rb_define_const( cRubyClass, "FEATURE_IRIS",
                   INT2NUM( DC1394_FEATURE_IRIS ) );
  rb_define_const( cRubyClass, "FEATURE_FOCUS",
                   INT2NUM( DC1394_FEATURE_FOCUS ) );
  rb_define_const( cRubyClass, "FEATURE_TEMPERATURE",
                   INT2NUM( DC1394_FEATURE_TEMPERATURE ) );
  rb_define_const( cRubyClass, "FEATURE_TRIGGER",
                   INT2NUM( DC1394_FEATURE_TRIGGER ) );
  rb_define_const( cRubyClass, "FEATURE_TRIGGER_DELAY",
                   INT2NUM( DC1394_FEATURE_TRIGGER_DELAY ) );
  rb_define_const( cRubyClass, "FEATURE_WHITE_SHADING",
                   INT2NUM( DC1394_FEATURE_WHITE_SHADING ) );
  rb_define_const( cRubyClass, "FEATURE_FRAME_RATE",
                   INT2NUM( DC1394_FEATURE_FRAME_RATE ) );
  rb_define_const( cRubyClass, "FEATURE_ZOOM",
                   INT2NUM( DC1394_FEATURE_ZOOM ) );
  rb_define_const( cRubyClass, "FEATURE_PAN",
                   INT2NUM( DC1394_FEATURE_PAN ) );
  rb_define_const( cRubyClass, "FEATURE_TILT",
                   INT2NUM( DC1394_FEATURE_TILT ) );
  rb_define_const( cRubyClass, "FEATURE_OPTICAL_FILTER",
                   INT2NUM( DC1394_FEATURE_OPTICAL_FILTER ) );
  rb_define_const( cRubyClass, "FEATURE_CAPTURE_SIZE",
                   INT2NUM( DC1394_FEATURE_CAPTURE_SIZE ) );
  rb_define_const( cRubyClass, "FEATURE_CAPTURE_QUALITY",
                   INT2NUM( DC1394_FEATURE_CAPTURE_QUALITY ) );
  rb_define_const( cRubyClass, "FEATURE_MAX",
                   INT2NUM( DC1394_FEATURE_MAX ) );
  rb_define_const( cRubyClass, "FEATURE_MODE_MANUAL",
                   INT2NUM( DC1394_FEATURE_MODE_MANUAL ) );
  rb_define_const( cRubyClass, "FEATURE_MODE_AUTO",
                   INT2NUM( DC1394_FEATURE_MODE_AUTO ) );
  rb_define_const( cRubyClass, "FEATURE_MODE_ONE_PUSH_AUTO",
                   INT2NUM( DC1394_FEATURE_MODE_ONE_PUSH_AUTO ) );
  rb_define_singleton_method( cRubyClass, "new", RUBY_METHOD_FUNC( wrapNew ), 5 );
  rb_define_method( cRubyClass, "close", RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "read", RUBY_METHOD_FUNC( wrapRead ), 0 );
  rb_define_method( cRubyClass, "status?", RUBY_METHOD_FUNC( wrapStatus ), 0 );
  rb_define_method( cRubyClass, "feature_read",
                    RUBY_METHOD_FUNC( wrapFeatureGetValue ), 1 );
  rb_define_method( cRubyClass, "feature_write",
                    RUBY_METHOD_FUNC( wrapFeatureSetValue ), 2 );
  rb_define_method( cRubyClass, "feature_exist?",
                    RUBY_METHOD_FUNC( wrapFeatureIsPresent ), 1 );
  rb_define_method( cRubyClass, "feature_readable?",
                    RUBY_METHOD_FUNC( wrapFeatureIsReadable ), 1 );
  rb_define_method( cRubyClass, "feature_switchable?",
                    RUBY_METHOD_FUNC( wrapFeatureIsSwitchable ), 1 );
  rb_define_method( cRubyClass, "feature_on?",
                    RUBY_METHOD_FUNC( wrapFeatureGetPower ), 1 );
  rb_define_method( cRubyClass, "feature_on",
                    RUBY_METHOD_FUNC( wrapFeatureSetPower ), 2 );
  rb_define_method( cRubyClass, "feature_modes",
                    RUBY_METHOD_FUNC( wrapFeatureModes ), 1 );
  rb_define_method( cRubyClass, "feature_mode_read",
                    RUBY_METHOD_FUNC( wrapFeatureModeGet ), 1 );
  rb_define_method( cRubyClass, "feature_mode_write",
                    RUBY_METHOD_FUNC( wrapFeatureModeSet ), 2 );
  rb_define_method( cRubyClass, "feature_min",
                    RUBY_METHOD_FUNC( wrapFeatureMin ), 1 );
  rb_define_method( cRubyClass, "feature_max",
                    RUBY_METHOD_FUNC( wrapFeatureMax ), 1 );
  return cRubyClass;
}

void DC1394Input::deleteRubyObject( void *ptr )
{
  delete (DC1394InputPtr *)ptr;
}

VALUE DC1394Input::wrapNew( VALUE rbClass, VALUE rbDC1394, VALUE rbNode,
                            VALUE rbSpeed, VALUE rbForceFrameRate, VALUE rbFrameRate )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394Ptr *dc1394; Data_Get_Struct( rbDC1394, DC1394Ptr, dc1394 );
    DC1394SelectPtr select( new DC1394Select );
    DC1394InputPtr ptr( new DC1394Input( *dc1394, NUM2UINT( rbNode ),
                                         (dc1394speed_t)NUM2INT( rbSpeed ),
                                         select, rbForceFrameRate != Qfalse,
                                         (dc1394framerate_t)NUM2INT( rbFrameRate ) ) );
    rbRetVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                                 new DC1394InputPtr( ptr ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapClose( VALUE rbSelf )
{
  DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
  (*self)->close();
  return rbSelf;
}

VALUE DC1394Input::wrapRead( VALUE rbSelf )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    FramePtr frame( (*self)->read() );
    rbRetVal = frame->rubyObject();
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapStatus( VALUE rbSelf )
{
  DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
  return (*self)->status() ? Qtrue : Qfalse;
}

VALUE DC1394Input::wrapFeatureGetValue( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    rbRetVal = UINT2NUM( (*self)->
                         featureGetValue( (dc1394feature_t)NUM2INT( rbFeature ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureSetValue( VALUE rbSelf, VALUE rbFeature, VALUE rbValue )
{
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    (*self)->featureSetValue( (dc1394feature_t)NUM2INT( rbFeature ),
                              NUM2UINT( rbValue ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbValue;
}

VALUE DC1394Input::wrapFeatureIsPresent( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    bool retVal = (*self)->featureIsPresent( (dc1394feature_t)NUM2INT( rbFeature  ) );
    rbRetVal = retVal ? Qtrue : Qfalse;
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureIsReadable( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    bool retVal = (*self)->featureIsReadable( (dc1394feature_t)NUM2INT( rbFeature  ) );
    rbRetVal = retVal ? Qtrue : Qfalse;
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureIsSwitchable( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    bool retVal = (*self)->
      featureIsSwitchable( (dc1394feature_t)NUM2INT( rbFeature ) );
    rbRetVal = retVal ? Qtrue : Qfalse;
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureGetPower( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    dc1394switch_t retVal = (*self)->
      featureGetPower( (dc1394feature_t)NUM2INT( rbFeature  ) );
    rbRetVal = retVal != DC1394_OFF ? Qtrue : Qfalse;
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureSetPower( VALUE rbSelf, VALUE rbFeature, VALUE rbValue )
{
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    (*self)->featureSetPower( (dc1394feature_t)NUM2INT( rbFeature ),
                              rbValue == Qtrue ? DC1394_ON : DC1394_OFF );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbValue;
}

VALUE DC1394Input::wrapFeatureModes( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    dc1394feature_modes_t retVal = (*self)->
      featureModes( (dc1394feature_t)NUM2INT( rbFeature ) );
    rbRetVal = rb_ary_new();
    for ( unsigned int i=0; i<retVal.num; i++ )
      rb_ary_push( rbRetVal, INT2NUM( retVal.modes[i] ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureModeGet( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    rbRetVal = UINT2NUM( (*self)->
                         featureModeGet( (dc1394feature_t)NUM2INT( rbFeature ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureModeSet( VALUE rbSelf, VALUE rbFeature, VALUE rbMode )
{
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    (*self)->featureModeSet( (dc1394feature_t)NUM2INT( rbFeature ),
                             (dc1394feature_mode_t)NUM2UINT( rbMode ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbMode;
}

VALUE DC1394Input::wrapFeatureMin( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    rbRetVal = UINT2NUM( (*self)->
                         featureMin( (dc1394feature_t)NUM2INT( rbFeature ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE DC1394Input::wrapFeatureMax( VALUE rbSelf, VALUE rbFeature )
{
  VALUE rbRetVal = Qnil;
  try {
    DC1394InputPtr *self; Data_Get_Struct( rbSelf, DC1394InputPtr, self );
    rbRetVal = UINT2NUM( (*self)->
                         featureMax( (dc1394feature_t)NUM2INT( rbFeature ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}


