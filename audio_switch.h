/*
 *  audio_switch.h
 *  AudioSwitcher
 *

Copyright (c) 2008 Devon Weller <wellerco@gmail.com>
Copyright (c) 2015 Ziga Zupanec <ziga.zupanec@gmail.com>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

*/

#include <unistd.h>
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardwareBase.h>

typedef enum {
  kAudioTypeUnknown = 0,
  kAudioTypeInput   = 1,
  kAudioTypeOutput  = 2,
  kAudioTypeSystemOutput = 3
} ASDeviceType;

enum {
  kFunctionSetDevice   = 1,
  kFunctionShowHelp  = 2,
  kFunctionShowAll   = 3,
  kFunctionShowCurrent = 4,
  kFunctionCycleNext   = 5,
  kFunctionSetVolume   = 6
};

void showUsage(const char * appName);
int runAudioSwitch(int argc, const char * argv[]);
void printProperties(AudioDeviceID deviceID, ASDeviceType device_type);
AudioDeviceID getCurrentlySelectedDeviceID(ASDeviceType typeRequested);
void getDeviceName(AudioDeviceID deviceID, char * deviceName);
ASDeviceType getDeviceType(AudioDeviceID deviceID);
bool isAnInputDevice(AudioDeviceID deviceID);
bool isAnOutputDevice(AudioDeviceID deviceID);
char *deviceTypeName(ASDeviceType device_type);
void showCurrentlySelectedDeviceID(ASDeviceType typeRequested);
AudioDeviceID getRequestedDeviceID(char * requestedDeviceName, ASDeviceType typeRequested);
AudioDeviceID getNextDeviceID(AudioDeviceID currentDeviceID, ASDeviceType typeRequested);
void setDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested);
void showAllDevices(ASDeviceType typeRequested);
void setDeviceVolume(AudioDeviceID deviceID, float vol_left, float vol_right);
