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

      # Firewire bus speed
      SPEED_100 = nil

      # Firewire bus speed
      SPEED_200 = nil

      # Firewire bus speed
      SPEED_400 = nil

      # Firewire bus speed
      SPEED_800 = nil

      # Firewire bus speed
      SPEED_1600 = nil

      # Firewire bus speed
      SPEED_3200 = nil

      # Video mode
      #
      # @private
      MODE_MONO8 = nil

      # Video mode
      #
      # @private
      MODE_YUV411 = nil

      # Video mode
      #
      # @private
      MODE_YUV422 = nil

      # Video mode
      #
      # @private
      MODE_YUV444 = nil

      # Video mode
      #
      # @private
      MODE_RGB8 = nil

      # Video mode
      #
      # @private
      MODE_MONO16 = nil

      # Video mode
      #
      # @private
      MODE_RGB16 = nil

      # Video mode
      #
      # @private
      MODE_MONO16S = nil

      # Video mode
      #
      # @private
      MODE_RGB16S = nil

      # Video mode
      #
      # @private
      MODE_RAW8 = nil

      # Video mode
      #
      # @private
      MODE_RAW16 = nil

      # Frame rate
      FRAMERATE_1_875 = nil

      # Frame rate
      FRAMERATE_3_75 = nil

      # Frame rate
      FRAMERATE_7_5 = nil

      # Frame rate
      FRAMERATE_15 = nil

      # Frame rate
      FRAMERATE_30 = nil

      # Frame rate
      FRAMERATE_60 = nil

      # Frame rate
      FRAMERATE_120 = nil

      # Frame rate
      FRAMERATE_240 = nil

      # First feature
      FEATURE_MIN = nil

      # A feature
      FEATURE_BRIGHTNESS = nil

      # A feature
      FEATURE_EXPOSURE = nil

      # A feature
      FEATURE_SHARPNESS = nil

      # A feature
      FEATURE_WHITE_BALANCE = nil

      # A feature
      FEATURE_HUE = nil

      # A feature
      FEATURE_SATURATION = nil

      # A feature
      FEATURE_GAMMA = nil

      # A feature
      FEATURE_SHUTTER = nil

      # A feature
      FEATURE_GAIN = nil

      # A feature
      FEATURE_IRIS = nil

      # A feature
      FEATURE_FOCUS = nil

      # A feature
      FEATURE_TEMPERATURE = nil

      # A feature
      FEATURE_TRIGGER = nil

      # A feature
      FEATURE_TRIGGER_DELAY = nil

      # A feature
      FEATURE_WHITE_SHADING = nil

      # A feature
      FEATURE_FRAME_RATE = nil

      # A feature
      FEATURE_ZOOM = nil

      # A feature
      FEATURE_PAN = nil

      # A feature
      FEATURE_TILT = nil

      # A feature
      FEATURE_OPTICAL_FILTER = nil

      # A feature
      FEATURE_CAPTURE_SIZE = nil

      # A feature
      FEATURE_CAPTURE_QUALITY = nil

      # Last feature
      FEATURE_MAX = nil

      # Feature mode
      FEATURE_MODE_MANUAL = nil

      # Feature mode
      FEATURE_MODE_AUTO = nil

      # Feature mode
      FEATURE_MODE_ONE_PUSH_AUTO = nil

    end

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

    # Get value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Current value of feature.
    def feature_read( id )
    end

    # Set value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Returns +value+.
    def feature_write( id, value )
    end

    # Check for existence of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Boolean] Returns +true+ if this feature is supported.
    def feature_exist?( id )
    end

    # Check whether feature can be read
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Boolean] Returns +true+ if this feature can be read.
    def feature_readable?( id )
    end

    # Check whether feature can be switched on and off
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Boolean] Returns +true+ if this feature can be switched on and off.
    def feature_switchable?( id )
    end

    # Check whether feature is switched on
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Boolean] Returns +true+ if this feature is switched on.
    def feature_on?( id )
    end

    # Switch feature on or off
    #
    # @param [Integer] id Feature identifier.
    # @param [Boolean] value +true+ for on and +false+ for off.
    #
    # @return [Boolean] Returns +value+.
    def feature_on( id, value )
    end

    # Get supported modes of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Array<Integer>] Returns array with supported modes.
    def feature_modes( id )
    end

    # Get current mode of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Returns the current mode.
    def feature_mode_read( id )
    end

    # Set mode of feature
    #
    # @param [Integer] id Feature identifier.
    # @param [Integer] mode Mode of feature.
    #
    # @return [Integer] Returns +mode+.
    def feature_mode_write( id, mode )
    end

    # Get minimum value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Minimum value of feature.
    def feature_min( id )
    end

    # Get maximum value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Maximum value of feature.
    def feature_max( id )
    end

  end

end

