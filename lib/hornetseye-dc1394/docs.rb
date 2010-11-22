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

      # Close the video device
      #
      # @return [DC1394Input] Returns +self+.
      def close
      end

      # Read a video frame
      #
      # @return [MultiArray,Frame_] The video frame.
      def read
      end

      # Check whether device is not closed
      #
      # @return [Boolean] Returns +true+ as long as device is open.
      def status?
      end

      # Width of video input
      #
      # @return [Integer] Width of video frames.
      def width
      end

      # Height of video input
      #
      # @return [Integer] Width of video frames.
      def height
      end

      def feature_read( id )
      end

      def feature_write( id, value )
      end

      def feature_exist?( id )
      end

      def feature_readable?( id )
      end

      def feature_switchable?( id )
      end

      def feature_on?( id )
      end

      def feature_on( id, value )
      end

      def feature_modes( id )
      end

      def feature_mode_read( id )
      end

      def feature_mode_write( id, mode )
      end

      def feature_min( id )
      end

      def feature_max( id )
      end

    end

  end

end

