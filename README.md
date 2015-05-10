switchaudio-osx
===============

A command-line utility to switch the audio source on Mac OS X.

Description
-----------

This utility for Mac OS X switches the audio source and/or adjusts volume of input/output devices.

You specify the name of the audio source, such as Built-in Digital Output, and the utility switches the source immediately without any GUI interaction.

This is a command-line utility only and has no graphical user interface.  Works with Mac OS 10.7 Lion and 10.8 Mountain Lion.

Installing Xcode Command Line Tools
-----------------------------------
switchaudio-osx requires command line tools to be installed from OS X. To install command line tools in Xcode 4.4, from the menubar go to Xcode > Preferences and click the Downloads tab. Click "Install" on the line that reads "Command Line Tools". 

Usage
-----

AudioSwitcher [-a] [-c] [-t type] [-n] -s device_name  
or  
AudioSwitcher -e device_id1=0.5,0.5:device_id2=0.7,0.8

 - **-a**               : Shows all devices.
 - **-c**               : Shows current device.
 - **-t** _type_        : device type (input/output/system). Defaults to output.
 - **-n**               : Cycles the audio device to the next one.
 - **-s** _device_name_ : Sets the audio device to the given device by name.
 - **-e** _device_id1_=_vol1_,_vol2_:_device_id2_=_vol1_,_vol2_ : Sets the volume of audio device given by id, followed by volume of first and second channel respectively. Multiple device can be separated with colon.


Thanks
-------

Thanks to Christian Zuckschwerdt for migrating this to github and adding the next option.

License
-------

MIT License, see license.txt
Copyright (c) 2008-2011 Devon Weller <wellerco@gmail.com>  
Copyright (c) 2011 Christian Zuckschwerdt <zany@triq.net>  
Copyright (c) 2015 [Ziga Zupanec](https://github.com/agiz/) <ziga.zupanec@gmail.com>

Imported from SVN at http://code.google.com/p/switchaudio-osx/
