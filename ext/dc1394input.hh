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
#ifndef HORNETSEYE_DC1394INPUT_HH
#define HORNETSEYE_DC1394INPUT_HH

#include <errno.h>
#include "error.hh"
#include "dc1394.hh"
#include "frame.hh"

class DC1394Input
{
public:
  DC1394Input( DC1394 *dc1394, int node ) throw (Error); // two cameras?
  virtual ~DC1394Input(void);
  void close(void);
  FramePtr read(void) throw (Error);
  bool status(void) const;
  std::string inspect(void) const;
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE module );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbDC1394, VALUE rbNode );
  static VALUE wrapClose( VALUE rbSelf );
  static VALUE wrapRead( VALUE rbSelf );
  static VALUE wrapStatus( VALUE rbSelf );
protected:
  std::string m_device;
};
  
typedef boost::shared_ptr< DC1394Input > DC1394InputPtr;

#endif

