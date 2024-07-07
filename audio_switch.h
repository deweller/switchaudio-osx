/*
 *  audio_switch.h
 *  AudioSwitcher
 *

Copyright (c) 2008 Devon Weller <wellerco@gmail.com>

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

 *
 */

#include <unistd.h>
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>
#include <CoreAudio/AudioHardwareBase.h>


typedef enum {
	kAudioTypeUnknown = 0,
	kAudioTypeInput   = 1,
	kAudioTypeOutput  = 2,
	kAudioTypeSystemOutput = 3,
	kAudioTypeAll = 4
} ASDeviceType;

typedef enum {
	kFormatHuman = 0,
	kFormatCLI = 1,
	kFormatJSON = 2,
} ASOutputType;

typedef enum {
	kUnmute = 0,
	kMute = 1,
	kToggleMute = 2,
} ASMuteType;

typedef enum {
    kL5_1_4 = 0,
    kL7_1_2 = 1,
    kL9_1_6 = 2,
}
ASLayoutType;

enum {
	kFunctionSetDeviceByName = 1,
	kFunctionShowHelp        = 2,
	kFunctionShowAll         = 3,
	kFunctionShowCurrent     = 4,
	kFunctionCycleNext       = 5,
    kFunctionSetDeviceByID   = 6,
    kFunctionSetDeviceByUID  = 7,
	kFunctionMute            = 8,
    kFunctionLayout          = 9,
};



void showUsage(const char * appName);
int runAudioSwitch(int argc, const char * argv[]);
const char * getDeviceUID(AudioDeviceID deviceID);
AudioDeviceID getRequestedDeviceIDFromUIDSubstring(char * requestedDeviceUID, ASDeviceType typeRequested);
AudioDeviceID getCurrentlySelectedDeviceID(ASDeviceType typeRequested);
void getDeviceName(AudioDeviceID deviceID, char * deviceName);
ASDeviceType getDeviceType(AudioDeviceID deviceID);
bool isAnInputDevice(AudioDeviceID deviceID);
bool isAnOutputDevice(AudioDeviceID deviceID);
char *deviceTypeName(ASDeviceType device_type);
void showCurrentlySelectedDeviceID(ASDeviceType typeRequested, ASOutputType outputRequested);
AudioDeviceID getRequestedDeviceID(char * requestedDeviceName, ASDeviceType typeRequested);
AudioDeviceID getNextDeviceID(AudioDeviceID currentDeviceID, ASDeviceType typeRequested);
int setDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested);
int setOneDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested);
int setAllDevicesByName(char * requestedDeviceName);
int cycleNext(ASDeviceType typeRequested);
int cycleNextForOneDevice(ASDeviceType typeRequested);
OSStatus setMute(ASDeviceType typeRequested, ASMuteType mute);
OSStatus setSpeakerLayout(ASDeviceType typeRequested, ASLayoutType layoutRequested, int * channelNumber);
void showAllDevices(ASDeviceType typeRequested, ASOutputType outputRequested);
