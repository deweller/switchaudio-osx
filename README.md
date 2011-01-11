switchaudio-osx
===============

A command-line utility to switch the audio source on Mac OS X.

Description
-----------

This utility switches the audio source for Mac OS X.

You specify the name of the audio source, such as Built-in Digital Output, and the utility switches the source immediately without any GUI interaction.

This is a command-line utility only and has no graphical user interface.

Usage
-----

AudioSwitcher [-a] [-c] [-t type] [-n] -s device_name  

 - **-a**               : shows all devices
 - **-c**               : shows current device
 - **-t** _type_        : device type (input/output/system).  Defaults to output.
 - **-n**               : cycles the audio device to the next one
 - **-s** _device_name_ : sets the audio device to the given device by name


Thanks
-------

Thanks to Christian Zuckschwerdt for migrating this to github and adding the next option.

License
-------

MIT License, see license.txt  
Copyright (c) 2008 Devon Weller <wellerco@gmail.com>  
Copyright (c) 2011 Christian Zuckschwerdt <zany@triq.net>

Imported from SVN at http://code.google.com/p/switchaudio-osx/
