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
// #include <boost/array.hpp>
#ifndef NDEBUG
#include <iostream>
#endif
#include "rubytools.hh"
#include "dc1394.hh"

using namespace boost;
using namespace std;

VALUE DC1394::cRubyClass = Qnil;

DC1394::DC1394(void) throw (Error):
  m_dc1394(NULL)
{
  try {
    m_dc1394 = dc1394_new();
    ERRORMACRO( m_dc1394 != NULL, Error, , "Error initialising DC1394 library" );
    
  } catch ( Error &e ) {
    close();
    throw e;
  };
}

DC1394::~DC1394(void)
{
  close();
}

void DC1394::close(void)
{
  if ( m_dc1394 != NULL ) {
    dc1394_free( m_dc1394 );
    m_dc1394 = NULL;
  };
}

string DC1394::inspect(void) const
{
  ostringstream s;
  s << "DC1394()";
  return s.str();
}

dc1394_t *DC1394::get(void) throw (Error)
{
  ERRORMACRO( m_dc1394 != NULL, Error, , "DC1394 library is not initialised. "
              "Did you call \"DC1394#close\" before?" );
  return m_dc1394;
}

bool DC1394::status(void) const
{
  return true;
}

VALUE DC1394::registerRubyClass( VALUE module )
{
  cRubyClass = rb_define_class_under( module, "DC1394", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 0 );
  rb_define_method( cRubyClass, "close",
                    RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "status?",
                    RUBY_METHOD_FUNC( wrapStatus ), 0 );
  return cRubyClass;
}

void DC1394::deleteRubyObject( void *ptr )
{
  delete (DC1394Ptr *)ptr;
}

VALUE DC1394::wrapNew( VALUE rbClass )
{
  VALUE retVal = Qnil;
  try {
    DC1394Ptr ptr( new DC1394 );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new DC1394Ptr( ptr ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE DC1394::wrapClose( VALUE rbSelf )
{
  DC1394Ptr *self; Data_Get_Struct( rbSelf, DC1394Ptr, self );
  (*self)->close();
  return rbSelf;
}

VALUE DC1394::wrapStatus( VALUE rbSelf )
{
  DC1394Ptr *self; Data_Get_Struct( rbSelf, DC1394Ptr, self );
  return (*self)->status() ? Qtrue : Qfalse;
}

