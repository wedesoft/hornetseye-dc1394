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
#include "dc1394select.hh"
#include "frame.hh"

class DC1394Input
{
public:
  DC1394Input( DC1394Ptr dc1394, int node, dc1394speed_t speed,
               DC1394SelectPtr select, bool forceFrameRate,
               dc1394framerate_t frameRate ) throw (Error);
  virtual ~DC1394Input(void);
  void close(void);
  FramePtr read(void) throw (Error);
  bool status(void) const;
  std::string inspect(void) const;
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE module );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbDC1394, VALUE rbNode, VALUE rbSpeed,
                        VALUE rbForceFrameRate, VALUE rbFrameRate );
  static VALUE wrapClose( VALUE rbSelf );
  static VALUE wrapRead( VALUE rbSelf );
  static VALUE wrapStatus( VALUE rbSelf );
protected:
  DC1394Ptr m_dc1394;
  int m_node;
  dc1394camera_t *m_camera;
  dc1394video_frame_t *m_frame;
};

typedef boost::shared_ptr< DC1394Input > DC1394InputPtr;

#endif

