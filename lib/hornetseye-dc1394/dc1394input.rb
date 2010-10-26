# hornetseye-xorg - Graphical output under X.Org
# Copyright (C) 2010 Jan Wedekind
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Namespace of Hornetseye computer vision library
module Hornetseye

  class DC1394Input

    class << self

      @@dc1394 = nil

      alias_method :orig_new, :new

      def new( node = 0, speed = SPEED_400, frame_rate = nil, &action )
        @@dc1394 = DC1394.new unless @@dc1394
        orig_new @@dc1394, node, speed, frame_rate != nil,
                 frame_rate || FRAMERATE_240 do |modes|
          map = { MONO8  => UBYTE,
                  YUV422 => UYVY,
                  RGB8   => UBYTERGB,
                  MONO16 => USINT }
          frame_types, index = [], []
          modes.each_with_index do |mode,i|
            target = map[ mode.first ]
            if target
              frame_types.push Hornetseye::Frame( target, *mode[ 1 .. 2 ] )
              index.push i
            end
          end
          desired = action.call frame_types
          unless frame_types.member? desired
            raise "Frame type #{desired} not supported by camera" 
          end
          index[ frame_types.index( desired ) ]
        end
      end

    end

  end

end

