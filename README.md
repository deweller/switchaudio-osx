switchaudio-osx
===============

A command-line utility to switch the audio source on Mac OS X.

Description
-----------

This utility switches the audio source for Mac OS X.

You specify the name of the audio source, such as Built-in Digital Output, and the utility switches the source immediately without any GUI interaction.

This is a command-line utility only and has no graphical user interface.  Works with Mac OS 10.7 Lion and 10.8 Mountain Lion.

Installing from homebrew
------------------------
```
$ brew install switchaudio-osx
```

Installing Xcode Command Line Tools
-----------------------------------
switchaudio-osx requires command line tools to be installed from OS X. To install command line tools in Xcode 4.4, from the menubar go to Xcode > Preferences and click the Downloads tab. Click "Install" on the line that reads "Command Line Tools". 

Usage
-----

SwitchAudioSource [-a] [-c] [-t type] [-n] -s device_name  

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
Copyright (c) 2008-2011 Devon Weller <wellerco@gmail.com>  
Copyright (c) 2011 Christian Zuckschwerdt <zany@triq.net>

Imported from SVN at http://code.google.com/p/switchaudio-osx/
