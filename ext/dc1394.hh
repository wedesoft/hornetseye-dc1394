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
#ifndef HORNETSEYE_DC1394_HH
#define HORNETSEYE_DC1394_HH

#include <boost/smart_ptr.hpp>
#include <dc1394/dc1394.h>
#include "error.hh"

class DC1394
{
public:
  DC1394(void) throw (Error);
  virtual ~DC1394(void);
  std::string inspect(void) const;
  void close(void);
  dc1394_t *get(void) throw (Error);
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE module );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass );
  static VALUE wrapClose( VALUE rbSelf );
protected:
  dc1394_t *m_dc1394;
};
  
typedef boost::shared_ptr< DC1394 > DC1394Ptr;

#endif

