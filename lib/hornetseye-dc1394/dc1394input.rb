# hornetseye-dc1394 - Capture from DC1394 compatible firewire camera
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

  # Class for handling a DC1394-compatible firewire camera
  #
  # This Ruby-extension is based on libdc1394.
  #
  # @see http://damien.douxchamps.net/ieee1394/libdc1394/
  class DC1394Input

    class << self

      # DC1394 handle
      #
      # @private
      @@dc1394 = nil

      # Alias for overriding native method
      #
      # @private
      alias_method :orig_new, :new

      # Open the firewire camera
      #
      # @param [Integer] node Camera node to open.
      # @param [Integer] speed Firewire bus speed.
      # @param [Integer,NilClass] frame_rate Desired frame rate.
      # @param [Proc] action Optional block for selecting the desired video mode.
      #
      # return [DC1394Input] An object for accessing the firewire camera.
      def new( node = 0, speed = SPEED_400, frame_rate = nil, &action )
        dc1394 = @@dc1394 || DC1394.new
        begin
          retval = orig_new dc1394, node, speed, frame_rate != nil,
                   frame_rate || FRAMERATE_240 do |modes|
            map = { MODE_MONO8  => UBYTE,
                    MODE_YUV422 => UYVY,
                    MODE_RGB8   => UBYTERGB,
                    MODE_MONO16 => USINT }
            frame_types, index = [], []
            modes.each do |mode|
              unless map[mode.first]
                warn "Unsupported video mode #{"0x%08x" % mode.first} #{mode[1]}x#{mode[2]}"
              end
            end
            modes.collect { |mode| [map[mode.first], *mode[1 .. 2]] }.
              each_with_index do |mode,i|
              if mode.first
                frame_types.push mode
                index.push i
              end
            end
            if action
              desired = action.call frame_types
            else
              preference = [ UBYTERGB, UYVY, USINT, UBYTE ]
              desired = frame_types.sort_by do |mode|
                [-preference.index(mode.first), mode[1] * mode[2]]
              end.last
              raise "Device does not support a known video mode" unless desired
            end
            unless frame_types.member? desired
              raise "Frame type #{desired.inspect} not supported by camera" 
            end
            index[frame_types.index(desired)]
          end
          @@dc1394 = dc1394
          retval
        ensure
          dc1394.close unless @@dc1394
        end
      end

    end

    include ReaderConversion

  end

end

