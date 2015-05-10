/*
 *  audio_switch.c
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

#include "audio_switch.h"

#define DEVICE_NAME_LEN 256
#define VOL_ARG_LEN 1024
#define VOL_DELIMITERS "=,:"

void showUsage(const char * appName) {
  printf("Usage: %s [-a] [-c] [-t type] [-n] -s device_name\n"
      "  -a     : shows all devices\n"
      "  -c     : shows current device\n\n"

      "  -t type  : device type (input/output/system).  Defaults to output.\n"
      "  -n     : cycles the audio device to the next one\n"
      "  -s device_name : sets the audio device to the given device by name\n"
      "  -e device_id1=vol1,vol2:device_id2=vol1,vol2 : sets audio device volume for 1st and 2nd channel. Multiple device can be separated with ':'.\n\n", appName);
}

int runAudioSwitch(int argc, const char * argv[]) {
  char requestedDeviceName[DEVICE_NAME_LEN];
  char volume_arg[VOL_ARG_LEN];
  AudioDeviceID chosenDeviceID = kAudioDeviceUnknown;
  ASDeviceType typeRequested = kAudioTypeUnknown;
  int function = 0;

  int c;
  while ((c = getopt(argc, (char **)argv, "hacnt:s:e:")) != -1) {
    switch (c) {
      case 'a':
        // show all
        function = kFunctionShowAll;
        break;
      case 'c':
        // get current device
        function = kFunctionShowCurrent;
        break;

      case 'e':
        // set device volume
        function = kFunctionSetVolume;
        strncpy(volume_arg, optarg, VOL_ARG_LEN);
        break;

      case 'h':
        // show help
        function = kFunctionShowHelp;
        break;

      case 'n':
        // cycle to the next audio device
        function = kFunctionCycleNext;
        break;

      case 's':
        // set the requestedDeviceName
        function = kFunctionSetDevice;
        strncpy(requestedDeviceName, optarg, DEVICE_NAME_LEN);
        break;

      case 't':
        // set the requestedDeviceName
        if (strcmp(optarg, "input") == 0) {
          typeRequested = kAudioTypeInput;
        }
        else if (strcmp(optarg, "output") == 0) {
          typeRequested = kAudioTypeOutput;
        }
        else if (strcmp(optarg, "system") == 0) {
          typeRequested = kAudioTypeSystemOutput;
        }
        else {
          printf("Invalid device type \"%s\" specified.\n", optarg);
          showUsage(argv[0]);
          return 1;
        }
        break;
    }
  }

  if (function == kFunctionShowAll) {
    switch(typeRequested) {
      case kAudioTypeInput:
      case kAudioTypeOutput:
        showAllDevices(typeRequested);
        break;
      case kAudioTypeSystemOutput:
        showAllDevices(kAudioTypeOutput);
        break;
      default:
        showAllDevices(kAudioTypeInput);
        showAllDevices(kAudioTypeOutput);
    }
    return 0;
  }
  if (function == kFunctionShowHelp) {
    showUsage(argv[0]);
    return 0;
  }
  if (function == kFunctionShowCurrent) {
    if (typeRequested == kAudioTypeUnknown) typeRequested = kAudioTypeOutput;
    showCurrentlySelectedDeviceID(typeRequested);
    return 0;
  }
  if (function == kFunctionSetVolume) {
    double vol1, vol2;
    int dev_id;
    int i = 0;
    char *pch = strtok(volume_arg, VOL_DELIMITERS);

    while (pch != NULL) {
      if (i % 3 == 0) {
        dev_id = atoi(pch);
      }
      else if (i % 3 == 2) {
        i = -1;
        vol2 = atof(pch);
        setDeviceVolume(dev_id, vol1, vol2);
      }
      else {
        vol1 = atof(pch);
      }

      pch = strtok(NULL, VOL_DELIMITERS);
      i++;
    }

    return 0;
  }

  if (typeRequested == kAudioTypeUnknown) typeRequested = kAudioTypeOutput;

  if (function == kFunctionCycleNext) {
    // get current device of requested type
    chosenDeviceID = getCurrentlySelectedDeviceID(typeRequested);
    if (chosenDeviceID == kAudioDeviceUnknown) {
      printf("Could not find current audio device of type %s.  Nothing was changed.\n", deviceTypeName(typeRequested));
      return 1;
    }

    // find next device to current device
    chosenDeviceID = getNextDeviceID(chosenDeviceID, typeRequested);
    if (chosenDeviceID == kAudioDeviceUnknown) {
      printf("Could not find next audio device of type %s.  Nothing was changed.\n", deviceTypeName(typeRequested));
      return 1;
    }

    // choose the requested audio device
    setDevice(chosenDeviceID, typeRequested);
    getDeviceName(chosenDeviceID, requestedDeviceName);
    printf("%s audio device set to \"%s\"\n", deviceTypeName(typeRequested), requestedDeviceName);

    return 0;
  }

  if (function != kFunctionSetDevice) {
    printf("Please specify audio device.\n");
    showUsage(argv[0]);
    return 1;
  }

  // find the id of the requested device
  chosenDeviceID = getRequestedDeviceID(requestedDeviceName, typeRequested);
  if (chosenDeviceID == kAudioDeviceUnknown) {
    printf("Could not find an audio device named \"%s\" of type %s.  Nothing was changed.\n",requestedDeviceName, deviceTypeName(typeRequested));
    return 1;
  }

  // choose the requested audio device
  setDevice(chosenDeviceID, typeRequested);
  printf("%s audio device set to \"%s\"\n", deviceTypeName(typeRequested), requestedDeviceName);
  return 0;

}

AudioDeviceID getCurrentlySelectedDeviceID(ASDeviceType typeRequested) {
  UInt32 propertySize;
  AudioDeviceID deviceID = kAudioDeviceUnknown;

  AudioObjectPropertyAddress pa;
  pa.mScope = kAudioObjectPropertyScopeGlobal;
  pa.mElement = kAudioObjectPropertyElementMaster;

  // get the default output device
  propertySize = sizeof(deviceID);
  switch(typeRequested) {
    case kAudioTypeInput:
      pa.mSelector = kAudioHardwarePropertyDefaultInputDevice;
      break;
    case kAudioTypeOutput:
      pa.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
      break;
    case kAudioTypeSystemOutput:
      pa.mSelector = kAudioHardwarePropertyDefaultSystemOutputDevice;
      break;
    default: break;
  }
  AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize, &deviceID);
  return deviceID;
}

void getDeviceName(AudioDeviceID deviceID, char *deviceName) {
  UInt32 propertySize = 256;
  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyDeviceName;
  pa.mScope = kAudioObjectPropertyScopeGlobal;
  pa.mElement = kAudioObjectPropertyElementMaster;

  AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &propertySize, deviceName);
}

void getDeviceVolume(AudioDeviceID deviceID, float *vol_left, float *vol_right) {
  OSStatus err;
  UInt32 size;
  UInt32 channel[2];

  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyPreferredChannelsForStereo;
  pa.mScope = kAudioDevicePropertyScopeOutput;
  pa.mElement = kAudioObjectPropertyElementMaster;

  size = sizeof(channel);
  err = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, &channel);
  if (err != noErr) {
    return;
  }

  *vol_right = -1.0;
  *vol_left = -1.0;
  pa.mSelector = kAudioDevicePropertyVolumeScalar;
  size = sizeof(float);

  pa.mElement = channel[0];
  err = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, vol_left);

  pa.mElement = channel[1];
  err |= AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, vol_right);

  if (err != noErr) {
    return;
  }
}

void setDeviceVolume(AudioDeviceID deviceID, float vol_left, float vol_right) {
  OSStatus err;
  UInt32 size;
  UInt32 channel[2];

  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyPreferredChannelsForStereo;
  pa.mScope = kAudioDevicePropertyScopeOutput;
  pa.mElement = kAudioObjectPropertyElementMaster;

  size = sizeof(channel);
  err = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, &size, &channel);
  if (err != noErr) {
    return;
  }

  pa.mSelector = kAudioDevicePropertyVolumeScalar;
  size = sizeof(Float32);

  pa.mElement = channel[0];
  err = AudioObjectSetPropertyData(deviceID, &pa, 0, NULL, size, &vol_left);

  pa.mElement = channel[1];
  err |= AudioObjectSetPropertyData(deviceID, &pa, 0, NULL, size, &vol_right);

  if (err != noErr) {
    printf("error setting volume of channel %u or %u\n", (unsigned int)channel[0], (unsigned int)channel[1]);
    return;
  }
}

typedef  UInt8  CAAudioHardwareDeviceSectionID;
#define  kAudioDeviceSectionGlobal  ((CAAudioHardwareDeviceSectionID)0x00)
void getAggregateDeviceSubDeviceList(AudioDeviceID deviceID, AudioObjectID *sub_device, UInt32 *outSize) {
  OSStatus err;

  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioAggregateDevicePropertyActiveSubDeviceList;
  pa.mScope = kAudioObjectPropertyScopeGlobal;
  pa.mElement = kAudioObjectPropertyElementMaster;

  err = AudioObjectGetPropertyData(deviceID, &pa, 0, NULL, outSize, sub_device);
  if (err != noErr) {
    printf("error getting subdevice list of an aggregate device - %u\n", (unsigned int)deviceID);
    return;
  }
}

// returns kAudioTypeInput or kAudioTypeOutput
ASDeviceType getDeviceType(AudioDeviceID deviceID) {
  UInt32 propertySize = 256;

  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyStreams;
  pa.mElement = kAudioObjectPropertyElementMaster;

  // if there are any output streams, then it is an output
  pa.mScope = kAudioDevicePropertyScopeOutput;
  AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
  if (propertySize > 0) return kAudioTypeOutput;

  // if there are any input streams, then it is an input
  pa.mScope = kAudioDevicePropertyScopeInput;
  AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
  if (propertySize > 0) return kAudioTypeInput;

  return kAudioTypeUnknown;
}

void getDeviceTransportType(AudioDeviceID deviceID, AudioDevicePropertyID *transportType) {
  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyTransportType;
  pa.mScope = kAudioObjectPropertyScopeGlobal;
  pa.mElement = kAudioObjectPropertyElementMaster;

  UInt32 size = sizeof(transportType);

  AudioObjectGetPropertyData(deviceID, &pa, 0, 0, &size, transportType);
}

bool isAnOutputDevice(AudioDeviceID deviceID) {
  UInt32 propertySize = 256;

  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyStreams;
  pa.mScope = kAudioDevicePropertyScopeOutput;
  pa.mElement = kAudioObjectPropertyElementMaster;

  // if there are any output streams, then it is an output
  AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
  if (propertySize > 0) return true;

  return false;
}

bool isAnInputDevice(AudioDeviceID deviceID) {
  UInt32 propertySize = 256;

  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioDevicePropertyStreams;
  pa.mScope = kAudioDevicePropertyScopeInput;
  pa.mElement = kAudioObjectPropertyElementMaster;

  // if there are any input streams, then it is an input
  AudioObjectGetPropertyDataSize(deviceID, &pa, 0, NULL, &propertySize);
  if (propertySize > 0) return kAudioTypeInput;

  return false;
}

char *deviceTypeName(ASDeviceType device_type) {
  switch(device_type) {
    case kAudioTypeInput: return "input";
    case kAudioTypeOutput: return "output";
    case kAudioTypeSystemOutput: return "system";
    default: return "unknown";
  }
}

bool hasSubdevices(AudioDevicePropertyID device_type) {
  switch(device_type) {
    case kAudioDeviceTransportTypeAggregate: return true;
    default: return false;
  }
}

void showCurrentlySelectedDeviceID(ASDeviceType typeRequested) {
  AudioDeviceID currentDeviceID = kAudioDeviceUnknown;
  currentDeviceID = getCurrentlySelectedDeviceID(typeRequested);

  printProperties(currentDeviceID, typeRequested);
}

UInt32 getNumberOfDevices(AudioDeviceID *dev_array) {
  UInt32 propertySize;
  AudioObjectPropertyAddress pa;
  pa.mSelector = kAudioHardwarePropertyDevices;
  pa.mScope = kAudioObjectPropertyScopeGlobal;
  pa.mElement = kAudioObjectPropertyElementMaster;

  AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize);
  //printf("propertySize1: %d\n", propertySize);

  AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, NULL, &propertySize, dev_array);
  //printf("numberOfDevices: %lu\n", (propertySize / sizeof(AudioDeviceID)));

  return (propertySize / sizeof(AudioDeviceID));
}

AudioDeviceID getRequestedDeviceID(char * requestedDeviceName, ASDeviceType typeRequested) {
  AudioDeviceID dev_array[64];
  int numberOfDevices = 0;
  char deviceName[DEVICE_NAME_LEN];

  numberOfDevices = getNumberOfDevices(dev_array);

  for(int i = 0; i < numberOfDevices; ++i) {
    switch(typeRequested) {
      case kAudioTypeInput:
        if (!isAnInputDevice(dev_array[i])) continue;
        break;

      case kAudioTypeOutput:
        if (!isAnOutputDevice(dev_array[i])) continue;
        break;

      case kAudioTypeSystemOutput:
        if (getDeviceType(dev_array[i]) != kAudioTypeOutput) continue;
        break;
      default: break;
    }

    getDeviceName(dev_array[i], deviceName);
    // printf("For device %d, id = %d and name is %s\n",i,dev_array[i],deviceName);
    if (strcmp(requestedDeviceName, deviceName) == 0) {
      return dev_array[i];
    }
  }

  return kAudioDeviceUnknown;
}

AudioDeviceID getNextDeviceID(AudioDeviceID currentDeviceID, ASDeviceType typeRequested) {
  AudioDeviceID dev_array[64];
  int numberOfDevices = 0;
  AudioDeviceID first_dev = kAudioDeviceUnknown;
  int found = -1;

  numberOfDevices = getNumberOfDevices(dev_array);

  for(int i = 0; i < numberOfDevices; ++i) {
    switch(typeRequested) {
      case kAudioTypeInput:
        if (!isAnInputDevice(dev_array[i])) continue;
        break;

      case kAudioTypeOutput:
        if (!isAnOutputDevice(dev_array[i])) continue;
        break;

      case kAudioTypeSystemOutput:
        if (getDeviceType(dev_array[i]) != kAudioTypeOutput) continue;
        break;
      default: break;
    }

    if (first_dev == kAudioDeviceUnknown) {
      first_dev = dev_array[i];
    }
    if (found >= 0) {
      return dev_array[i];
    }
    if (dev_array[i] == currentDeviceID) {
      found = i;
    }
  }

  return first_dev;
}

void setDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested) {
  UInt32 propertySize = sizeof(UInt32);

  AudioObjectPropertyAddress pa;
  pa.mScope = kAudioObjectPropertyScopeGlobal;
  pa.mElement = kAudioObjectPropertyElementMaster;

  switch(typeRequested) {
    case kAudioTypeInput:
      pa.mSelector = kAudioHardwarePropertyDefaultInputDevice;
      break;
    case kAudioTypeOutput:
      pa.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
      break;
    case kAudioTypeSystemOutput:
      pa.mSelector = kAudioHardwarePropertyDefaultSystemOutputDevice;
      break;
    default: break;
  }
  AudioObjectSetPropertyData(kAudioObjectSystemObject, &pa, 0, NULL, propertySize, &newDeviceID);
}

void printProperties(AudioDeviceID deviceID, ASDeviceType typeRequested) {
  char deviceName[DEVICE_NAME_LEN];
  float vol_left, vol_right;
  ASDeviceType device_type;
  UInt32 transportType;

  switch(typeRequested) {
    case kAudioTypeInput:
      if (!isAnInputDevice(deviceID)) return;
      device_type = kAudioTypeInput;
      break;

    case kAudioTypeOutput:
      if (!isAnOutputDevice(deviceID)) return;
      device_type = kAudioTypeOutput;
      break;

    case kAudioTypeSystemOutput:
      device_type = getDeviceType(deviceID);
      if (device_type != kAudioTypeOutput) return;
      break;
    default: break;
  }

  getDeviceName(deviceID, deviceName);
  getDeviceVolume(deviceID, &vol_left, &vol_right);

  printf("[%3u] - %6s %-26s", (unsigned int)deviceID, deviceTypeName(device_type), deviceName);
  if (vol_left < -0.1 || vol_right < -0.1) {
    printf("\n");
  }
  else {
    printf(" :: [%.3f:%.3f]\n", vol_left, vol_right);
  }

  getDeviceTransportType(deviceID, &transportType);
  if (transportType == kAudioDeviceTransportTypeAggregate) {
    AudioObjectID sub_device[32];
    UInt32 outSize = sizeof(sub_device);
    getAggregateDeviceSubDeviceList(deviceID, sub_device, &outSize);
    for (int j=0; j<outSize / sizeof(AudioObjectID); j++) {
      printf("\t");
      printProperties(sub_device[j], device_type);
    }
  }
}

void showAllDevices(ASDeviceType typeRequested) {
  AudioDeviceID dev_array[64];
  int numberOfDevices = 0;

  numberOfDevices = getNumberOfDevices(dev_array);

  for(int i = 0; i < numberOfDevices; ++i) {
    printProperties(dev_array[i], typeRequested);
  }
}
