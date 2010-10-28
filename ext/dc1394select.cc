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
#include "rubyinc.hh"
#include "dc1394select.hh"

DC1394Select::DC1394Select(void) throw (Error):
  m_rbArray(rb_ary_new())
{
}

DC1394Select::~DC1394Select(void)
{
}

void DC1394Select::add( dc1394color_coding_t coding, unsigned int width,
                        unsigned int height )
{
  rb_ary_push( m_rbArray, rb_ary_new3( 3, INT2NUM( coding ), INT2NUM( width ),
                                       INT2NUM( height ) ) );
}

static VALUE yield( VALUE arg )
{
  return rb_yield( arg );
}

unsigned int DC1394Select::make(void) throw (Error)
{
  int error;
  VALUE rbRetVal = rb_protect( yield, m_rbArray, &error );
  if ( error ) {
    VALUE rbError = rb_funcall( rb_gv_get( "$!" ), rb_intern( "message" ), 0 );
    ERRORMACRO( false, Error, , "Error in block to \"DC1394Input.new\": "
                << StringValuePtr( rbError ) );
  };
  ERRORMACRO( TYPE( rbRetVal ) == T_FIXNUM, Error, , "Block must return a value of "
              "type 'Fixnum'" );
  return NUM2UINT(rbRetVal);
}

VALUE DC1394Select::wrapRescue( VALUE rbValue )
{
  return rbValue;
}
