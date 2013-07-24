hornetseye-dc1394
=================
This Ruby extension provides camera input for DC1394 compatible firewire cameras using libdc1394 2.x.

**Author**:       Jan Wedekind
**Copyright**:    2010
**License**:      GPL

Synopsis
--------

This Ruby extension provides the class {Hornetseye::DC1394Input} for capturing video frames using a DC1394-compatible firewire camera.

Installation
------------
*hornetseye-dc1394* requires the DC1394 headers. If you are running Debian or (K)ubuntu, you can install them like this:

    $ sudo aptitude install libdc1394-22-dev libswscale-dev libboost-dev

To install this Ruby extension, use the following command:

    $ sudo gem install hornetseye-dc1394

Alternatively you can build and install the Ruby extension from source as follows:

    $ rake
    $ sudo rake install

Usage
-----

Simply run Interactive Ruby:

    $ irb

You can open a DC1394-compatible firewire camera as shown below. This example will open the camera and switch to a resolution selected by the user. Finally the camera input is displayed in a window. This example requires *hornetseye-xorg* in addition to this Ruby extension.

    require 'rubygems'
    require 'hornetseye_dc1394'
    require 'hornetseye_xorg'
    include Hornetseye
    camera = DC1394Input.new 0 do |modes|
      modes.each_with_index { |mode,i| puts "#{i + 1}: #{mode}" }
      modes[ STDIN.readline.to_i - 1 ]
    end
    X11Display.show { camera.read }

