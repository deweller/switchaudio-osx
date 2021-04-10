switchaudio-osx
===============

A command-line utility to switch the audio source on Mac OS X.

Description
-----------

This utility switches the audio source for Mac OS X.

You specify the name of the audio source, such as Built-in Digital Output, and the utility switches the source immediately without any GUI interaction.

This is a command-line utility only and has no graphical user interface.  Tested on OS 10.7 - 11.2.

Installing from homebrew
------------------------
```
$ brew install switchaudio-osx
```

Usage
-----

SwitchAudioSource [-a] [-c] [-f format] [-t type] [-n] -s device\_name | -i device\_id | -u device\_uid 

 - **-a**               : shows all devices
 - **-c**               : shows current device
 - **-f** _format_      : output format (cli/human/json). Defaults to human.
 - **-t** _type_        : device type (input/output/system).  Defaults to output.
 - **-n**               : cycles the audio device to the next one
 - **-i** _device_id_   : sets the audio device to the given device by id
 - **-u** _device_uid_  : sets the audio device to the given device by uid or a substring of the uid
 - **-s** _device_name_ : sets the audio device to the given device by name


Thanks
-------

Thanks to Christian Zuckschwerdt for migrating this to github and adding the next option.

License
-------

MIT License, see license.txt  

Copyright (c) 2008-2021 Devon Weller <wellerco@gmail.com>  
Copyright (c) 2011 Christian Zuckschwerdt <zany@triq.net>
