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
#include "rubytools.hh"
#include "dc1394input.hh"

using namespace boost;
using namespace std;

VALUE DC1394Input::cRubyClass = Qnil;

DC1394Input::DC1394Input( DC1394 *dc1394, int node ) throw (Error)
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
    dc1394_camera_free_list( list ); list = NULL;
    //ERRORMACRO( m_dc1394 != NULL, Error, , "Unable to acquire raw1394 handle. Please "
    //            "check, whether the kernel modules 'ieee1394', 'raw1394', and "
    //            "'ohci1394' are loaded and whether you have read/write permission on "
    //            "\"/dev/raw1394\"" );
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
}

string DC1394Input::inspect(void) const
{
  ostringstream s;
  s << "DC1394Input( '" << m_device << "' )";
  return s.str();
}

bool DC1394Input::status(void) const
{
  return true;
}

VALUE DC1394Input::registerRubyClass( VALUE module )
{
  cRubyClass = rb_define_class_under( module, "DC1394Input", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 2 );
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

VALUE DC1394Input::wrapNew( VALUE rbClass, VALUE rbDC1394, VALUE rbNode )
{
  VALUE retVal = Qnil;
  try {
    DC1394Ptr *dc1394; Data_Get_Struct( rbDC1394, DC1394Ptr, dc1394 );
    DC1394InputPtr ptr( new DC1394Input( dc1394->get(), NUM2INT( rbNode ) ) );
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

