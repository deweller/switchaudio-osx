/*
 *  audio_switch.c
 *  AudioSwitcher

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

 */

#include "audio_switch.h"


void showUsage(const char * appName) {
    printf("Usage: %s [-a] [-c] [-t type] [-n] [-m] [-l] -s device_name | -i device_id | -u device_uid\n"
           "  -a             : shows all devices\n"
           "  -c             : shows current device\n\n"
           "  -f format      : output format (cli/human/json). Defaults to human.\n"
           "  -t type        : device type (input/output/system/all).  Defaults to output.\n"
           "  -l             : sets the speaker layout (5_1_4/7_1_2/9_1_6).  For output only.\n"
           "  -p             : comma delimited list of channel values starting with 0.  For output only.\n"
           "  -m mute        : sets the mute status (mute/unmute/toggle).  For input/output only.\n"
           "  -n             : cycles the audio device to the next one\n"
           "  -i device_id   : sets the audio device to the given device by id\n"
           "  -u device_uid  : sets the audio device to the given device by uid or a substring of the uid\n"
           "  -s device_name : sets the audio device to the given device by name\n"
           "\n"
           "5.1.4 Atmos L R C LFE Ls Rs Vhl Vhr Ltr Rtr\n"
           "7.1.2 Atmos L R C LFE Ls Rs Rls Rrs Ltm Rtm\n"
           "9.1.6 Atmos L R C LFE Ls Rs Rls Rrs Lw Rw Vhl Vhr Ltm Rtm Ltr Rtr\n"
           "\n"
           "Examples\n"
           "./AudioSwitcher -l 5_1_4 -p 0,1,5,4,2,3,6,7,8,9 -t output\n"
           "\n"
           ,appName);
}

int runAudioSwitch(int argc, const char * argv[]) {
    char requestedDeviceName[256];
    char printableDeviceName[256];
    int requestedDeviceID;
    char requestedDeviceUID[256];
    AudioDeviceID chosenDeviceID = kAudioDeviceUnknown;
    ASDeviceType typeRequested = kAudioTypeUnknown;
    ASOutputType outputRequested = kFormatHuman;
    ASMuteType muteRequested = kToggleMute;
    ASLayoutType layoutRequested = kL5_1_4;
    int function = 0;
    int result = 0;
    const char s[2] = ",";
    char *token;
    int channelNumber[16] = {0, 1, 2, 3, 4, 5 ,6 ,7, 8, 9, 10, 11, 12, 13, 14, 15};
    int i;

    int c;
    while ((c = getopt(argc, (char **)argv, "hacl:m:nt:f:i:u:s:p:")) != -1) {
        switch (c) {
            case 'f':
                // format
                if (strcmp(optarg, "cli") == 0) {
                    outputRequested = kFormatCLI;
                } else if (strcmp(optarg, "json") == 0) {
                    outputRequested = kFormatJSON;
                } else if (strcmp(optarg, "human") == 0) {
                    outputRequested = kFormatHuman;
                } else {
                    printf("Unknown format %s\n", optarg);
                    showUsage(argv[0]);
                    return 1;
                }
                break;
            case 'a':
                // show all
                function = kFunctionShowAll;
                break;
            case 'c':
                // get current device
                function = kFunctionShowCurrent;
                break;

            case 'h':
                // show help
                function = kFunctionShowHelp;
                break;
                
            case 'm':
                // control the mute status of the interface selected with -t
                function = kFunctionMute;
                // set the mute mode
                if (strcmp(optarg, "mute") == 0) {
                    muteRequested = kMute;
                } else if (strcmp(optarg, "unmute") == 0) {
                    muteRequested = kUnmute;
                } else if (strcmp(optarg, "toggle") == 0) {
                    muteRequested = kToggleMute;
                } else {
                    printf("Invalid mute operation type \"%s\" specified.\n", optarg);
                    showUsage(argv[0]);
                    return 1;
                }
                break;
                
            case 'l':
                // set the speaker layout to 5.1.4
                function = kFunctionLayout;
                if (strcmp(optarg, "5_1_4") == 0) {
                    layoutRequested = kL5_1_4;
                } else if (strcmp(optarg, "7_1_2") == 0) {
                    layoutRequested = kL7_1_2;
                } else if (strcmp(optarg, "9_1_6") == 0) {
                    layoutRequested = kL9_1_6;
                } else {
                    printf("Invalid Layout operation type \"%s\" specified.\n", optarg);
                    showUsage(argv[0]);
                    return 1;
                }
                break;

            case 'p':
                token = strtok(optarg, s);
                i=0;
                while( token != NULL ) {
                    channelNumber[i] = atoi(token);
                    i++;
                    if (i>15) {
                        break;
                    }
                    token = strtok(NULL, s);
                }
                break;

            case 'n':
                // cycle to the next audio device
                function = kFunctionCycleNext;
                break;
                
            case 'i':
                // set the requestedDeviceID
                function = kFunctionSetDeviceByID;
                requestedDeviceID = atoi(optarg);
                break;

            case 'u':
                // set the requestedDeviceUID
                function = kFunctionSetDeviceByUID;
                strcpy(requestedDeviceUID, optarg);
                break;

            case 's':
                // set the requestedDeviceName
                function = kFunctionSetDeviceByName;
                strcpy(requestedDeviceName, optarg);
                break;

            case 't':
                // set the requestedDeviceName
                if (strcmp(optarg, "input") == 0) {
                    typeRequested = kAudioTypeInput;
                } else if (strcmp(optarg, "output") == 0) {
                    typeRequested = kAudioTypeOutput;
                } else if (strcmp(optarg, "system") == 0) {
                    typeRequested = kAudioTypeSystemOutput;
                } else if (strcmp(optarg, "all") == 0) {
                    typeRequested = kAudioTypeAll;
                } else {
                    printf("Invalid device type \"%s\" specified.\n",optarg);
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
                showAllDevices(typeRequested, outputRequested);
                break;
            case kAudioTypeSystemOutput:
                showAllDevices(kAudioTypeOutput, outputRequested);
                break;
            default:
                showAllDevices(kAudioTypeInput, outputRequested);
                showAllDevices(kAudioTypeOutput, outputRequested);
        }
        return 0;
    }
    if (function == kFunctionShowHelp) {
        showUsage(argv[0]);
        return 0;
    }
    if (function == kFunctionShowCurrent) {
        if (typeRequested == kAudioTypeUnknown) typeRequested = kAudioTypeOutput;
        showCurrentlySelectedDeviceID(typeRequested, outputRequested);
        return 0;
    }

    if (typeRequested == kAudioTypeUnknown) typeRequested = kAudioTypeOutput;

    if (function == kFunctionCycleNext) {
        result = cycleNext(typeRequested);
        return result;
    }

    if (function == kFunctionSetDeviceByID) {
        chosenDeviceID = (AudioDeviceID)requestedDeviceID;
        sprintf(printableDeviceName, "Device with ID: %d", chosenDeviceID);
    }

    if (function == kFunctionSetDeviceByName && typeRequested != kAudioTypeAll) {
        // find the id of the requested device
        chosenDeviceID = getRequestedDeviceID(requestedDeviceName, typeRequested);
        if (chosenDeviceID == kAudioDeviceUnknown) {
            printf("Could not find an audio device named \"%s\" of type %s.  Nothing was changed.\n",requestedDeviceName, deviceTypeName(typeRequested));
            return 1;
        }
        strcpy(printableDeviceName, requestedDeviceName);
    }

    if (function == kFunctionSetDeviceByUID) {
        // find the id of the requested device
        chosenDeviceID = getRequestedDeviceIDFromUIDSubstring(requestedDeviceUID, typeRequested);
        if (chosenDeviceID == kAudioDeviceUnknown) {
            printf("Could not find an audio device with UID \"%s\" of type %s.  Nothing was changed.\n", requestedDeviceUID, deviceTypeName(typeRequested));
            return 1;
        }
        sprintf(printableDeviceName, "Device with UID: %s", getDeviceUID(chosenDeviceID));
    }

    if (function == kFunctionLayout) {
        OSStatus status;
        if (typeRequested == kAudioTypeUnknown) typeRequested = kAudioTypeInput;

        switch(typeRequested) {
            case kAudioTypeOutput:
                status = setSpeakerLayout(typeRequested, layoutRequested, channelNumber);
                if(status != noErr) {
                    printf("Failed setting speaker layout. Error: %d (%s)", status, GetMacOSStatusErrorString(status));
                    return 1;
                }
                break;
            case kAudioTypeInput:
                printf("audio device \"%s\" is not an output device\n", deviceTypeName(typeRequested));
                return 0;
                break;
            case kAudioTypeAll:
                printf("audio device \"%s\" is not an output device\n", deviceTypeName(typeRequested));
                return 0;
                break;
            case kAudioTypeSystemOutput:
                printf("audio device \"%s\" is not an output device\n", deviceTypeName(typeRequested));
                return 0;
                break;
            case kAudioTypeUnknown:
                printf("audio device \"%s\" is unknown\n", deviceTypeName(typeRequested));
                return 0;
                break;
        }
        return 0;
    }


    if (function == kFunctionMute) {
        OSStatus status;
        bool anyStatusError = false;
        if (typeRequested == kAudioTypeUnknown) typeRequested = kAudioTypeInput;
        
        switch(typeRequested) {
            case kAudioTypeInput: 
            case kAudioTypeOutput:
                status = setMute(typeRequested, muteRequested);
                if(status != noErr) {
                    printf("Failed setting mute state. Error: %d (%s)", status, GetMacOSStatusErrorString(status));
                    return 1;
                }
                break;
            case kAudioTypeAll:
                status = setMute(kAudioTypeInput, muteRequested);
                if(status != noErr) {
                    printf("Failed setting mute state for input. Error: %d (%s)", status, GetMacOSStatusErrorString(status));
                    anyStatusError = true;
                }
                status = setMute(kAudioTypeOutput, muteRequested);
                if(status != noErr) {
                    printf("Failed setting mute state for output. Error: %d (%s)", status, GetMacOSStatusErrorString(status));
                    anyStatusError = true;
                }
                if (anyStatusError) {
                    return 1;
                }
                break;
            case kAudioTypeSystemOutput:
                printf("audio device \"%s\" may not be muted\n", deviceTypeName(typeRequested));
                return 1;
                break;
            case kAudioTypeUnknown:
                printf("audio device \"%s\" is unknown\n", deviceTypeName(typeRequested));
                return 0;
                break;
        }
        return 0;
    }

    
    if (typeRequested == kAudioTypeAll && function == kFunctionSetDeviceByName) {
        // special case for all - process each one separately
        result = setAllDevicesByName(requestedDeviceName);
    } else {
        // require a chose
        if (!chosenDeviceID) {
            printf("Please specify audio device.\n");
            showUsage(argv[0]);
            return 1;
        }

        // choose the requested audio device
        result = setDevice(chosenDeviceID, typeRequested);
        printf("%s audio device set to \"%s\"\n", deviceTypeName(typeRequested), printableDeviceName);
    }


    return result;
}

const char * getDeviceUID(AudioDeviceID deviceID) {
    CFStringRef deviceUID = NULL;
    UInt32 dataSize = sizeof(deviceUID);
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    
    propertyAddress.mSelector = kAudioDevicePropertyDeviceUID;
    
    OSStatus err = AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &dataSize, &deviceUID);
    if (err != 0) {
        // Handle error
        return "";
    }

    const char * deviceUID_string = NULL;
    CFIndex length = CFStringGetLength(deviceUID);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    deviceUID_string = (char *)malloc(maxSize);

    if (deviceUID_string) {
        if (CFStringGetCString(deviceUID, deviceUID_string, maxSize, kCFStringEncodingUTF8)) {
            CFRelease(deviceUID);
            return deviceUID_string;
        }
        free((void *)deviceUID_string);
    }
    
    CFRelease(deviceUID);
    return "";
}

AudioDeviceID getRequestedDeviceIDFromUIDSubstring(char * requestedDeviceUID, ASDeviceType typeRequested) {
    AudioObjectPropertyAddress propertyAddress = {kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
    UInt32 propertySize;
    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;

    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, dev_array);
    numberOfDevices = (propertySize / sizeof(AudioDeviceID));

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

        char deviceUID[256];
        CFStringRef deviceUIDRef = NULL;
        propertyAddress.mSelector = kAudioDevicePropertyDeviceUID;
        propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
        propertyAddress.mElement = dev_array[i];
        propertySize = sizeof(deviceUIDRef);
        AudioObjectGetPropertyData(dev_array[i], &propertyAddress, 0, NULL, &propertySize, &deviceUIDRef);
        CFStringGetCString(deviceUIDRef, deviceUID, sizeof(deviceUID), CFStringGetSystemEncoding());
        if (strstr(deviceUID, requestedDeviceUID) != NULL) {
            return dev_array[i];
        }
    }

    return kAudioDeviceUnknown;
}

AudioDeviceID getCurrentlySelectedDeviceID(ASDeviceType typeRequested) {
    AudioObjectPropertyAddress address;
    address.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    address.mScope = kAudioObjectPropertyScopeGlobal;
    address.mElement = kAudioObjectPropertyElementMaster;

    switch (typeRequested) {
        case kAudioTypeInput:
            address.mSelector = kAudioHardwarePropertyDefaultInputDevice;
            break;
        case kAudioTypeOutput:
            address.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
            break;
        case kAudioTypeSystemOutput:
            address.mSelector = kAudioHardwarePropertyDefaultSystemOutputDevice;
            break;
        default:
            break;
    }

    AudioDeviceID deviceID = kAudioDeviceUnknown;
    UInt32 dataSize = sizeof(AudioDeviceID);
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &dataSize, &deviceID);
    if (status != noErr) {
        // handle error
    }

    return deviceID;
}

void getDeviceName(AudioDeviceID deviceID, char* deviceName) {
    AudioObjectPropertyAddress address = {
        kAudioDevicePropertyDeviceNameCFString,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    CFStringRef cfDeviceName = NULL;
    UInt32 dataSize = sizeof(CFStringRef);
    OSStatus result = AudioObjectGetPropertyData(deviceID, &address, 0, NULL, &dataSize, &cfDeviceName);
    if (result == noErr && cfDeviceName != NULL) {
        CFStringGetCString(cfDeviceName, deviceName, 256, kCFStringEncodingUTF8);
        CFRelease(cfDeviceName);
    }
}

// returns kAudioTypeInput or kAudioTypeOutput
ASDeviceType getDeviceType(AudioDeviceID deviceID) {
    AudioObjectPropertyAddress address = {
        kAudioDevicePropertyStreams,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    UInt32 dataSize = 0;
    OSStatus result = AudioObjectGetPropertyDataSize(deviceID, &address, 0, NULL, &dataSize);
    if (result == noErr && dataSize > 0) {
        return kAudioTypeOutput;
    }
    address.mElement = kAudioObjectPropertyElementMaster + 1;
    result = AudioObjectGetPropertyDataSize(deviceID, &address, 0, NULL, &dataSize);
    if (result == noErr && dataSize > 0) {
        return kAudioTypeInput;
    }
    return kAudioTypeUnknown;
}

bool isAnOutputDevice(AudioDeviceID deviceID) {
    AudioObjectPropertyAddress propertyAddress = {kAudioDevicePropertyStreams, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster};
    UInt32 dataSize = 0;
    OSStatus result = AudioObjectGetPropertyDataSize(deviceID, &propertyAddress, 0, NULL, &dataSize);
    if (result == noErr && dataSize > 0) {
        return true;
    }
    return false;
}

bool isAnInputDevice(AudioDeviceID deviceID) {
    AudioObjectPropertyAddress propertyAddress = {kAudioDevicePropertyStreams, kAudioDevicePropertyScopeInput, kAudioObjectPropertyElementMaster};
    UInt32 dataSize = 0;
    OSStatus result = AudioObjectGetPropertyDataSize(deviceID, &propertyAddress, sizeof(AudioClassDescription), kAudioDevicePropertyScopeInput, &dataSize);
    if (result == noErr && dataSize > 0) {
        return kAudioTypeInput;
    }
    return false;
}

char *deviceTypeName(ASDeviceType device_type) {
    switch(device_type) {
        case kAudioTypeInput: return "input";
        case kAudioTypeOutput: return "output";
        case kAudioTypeSystemOutput: return "system";
        case kAudioTypeAll: return "all";
        default: return "unknown";
    }
    
}

void showCurrentlySelectedDeviceID(ASDeviceType typeRequested, ASOutputType outputRequested) {
    AudioDeviceID currentDeviceID = kAudioDeviceUnknown;
    char currentDeviceName[256];
    
    currentDeviceID = getCurrentlySelectedDeviceID(typeRequested);
    getDeviceName(currentDeviceID, currentDeviceName);

    switch(outputRequested) {
        case kFormatHuman:
            printf("%s\n",currentDeviceName);
            break;
        case kFormatCLI:
            printf("%s,%s,%u,%s\n",currentDeviceName,deviceTypeName(typeRequested),currentDeviceID,getDeviceUID(currentDeviceID));
            break;
        case kFormatJSON:
            printf("{\"name\": \"%s\", \"type\": \"%s\", \"id\": \"%u\", \"uid\": \"%s\"}\n",currentDeviceName,deviceTypeName(typeRequested),currentDeviceID,getDeviceUID(currentDeviceID));
            break;
        default:
            break;
    }
}

AudioDeviceID getRequestedDeviceID(char * requestedDeviceName, ASDeviceType typeRequested) {
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDevices;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;
    
    UInt32 propertySize;
    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;
    char deviceName[256];
    
    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize);
    if (status != noErr) {
        printf("Error getting size of property data: %d\n", status);
        return kAudioDeviceUnknown;
    }
    
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, dev_array);
    if (status != noErr) {
        printf("Error getting property data: %d\n", status);
        return kAudioDeviceUnknown;
    }
    
    numberOfDevices = (propertySize / sizeof(AudioDeviceID));
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
        if (strcmp(requestedDeviceName, deviceName) == 0) {
            return dev_array[i];
        }
    }
    
    return kAudioDeviceUnknown;
}

AudioDeviceID getNextDeviceID(AudioDeviceID currentDeviceID, ASDeviceType typeRequested) {
    AudioObjectPropertyAddress addr;
    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;
    AudioDeviceID first_dev = kAudioDeviceUnknown;
    int found = -1;

    addr.mSelector = kAudioHardwarePropertyDevices;
    addr.mScope = kAudioObjectPropertyScopeGlobal;
    addr.mElement = kAudioObjectPropertyElementMaster;
    UInt32 propertySize = 0;
    OSStatus err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, NULL, &propertySize);
    if (err != noErr) {
        return kAudioDeviceUnknown;
    }

    err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &propertySize, dev_array);
    if (err != noErr) {
        return kAudioDeviceUnknown;
    }
    numberOfDevices = (propertySize / sizeof(AudioDeviceID));

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

            default: 
                break;
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

int setDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested) {
    return setOneDevice(newDeviceID, typeRequested);
}

int setOneDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested) {
    AudioObjectPropertyAddress addr;
    UInt32 propertySize = sizeof(UInt32);
    OSStatus status;

    addr.mScope = kAudioObjectPropertyScopeGlobal;
    addr.mElement = kAudioObjectPropertyElementMaster;

    switch(typeRequested) {
        case kAudioTypeInput:
            addr.mSelector = kAudioHardwarePropertyDefaultInputDevice;
            break;
        case kAudioTypeOutput:
            addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
            break;
        case kAudioTypeSystemOutput:
            addr.mSelector = kAudioHardwarePropertyDefaultSystemOutputDevice;
            break;
        default:
            addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
            break;
    }
    status = AudioObjectSetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, propertySize, &newDeviceID);
    if(status != noErr) {
        printf("Failed to set %s", deviceTypeName(typeRequested));
    }

    return 0;
}

int setAllDevicesByName(char * requestedDeviceName) {
    int result;
    bool anyStatusError = false;
    AudioDeviceID newDeviceID;
    
    // input
    newDeviceID = getRequestedDeviceID(requestedDeviceName, kAudioTypeInput);
    if (newDeviceID != nil) {
        result = setOneDevice(newDeviceID, kAudioTypeInput);
        if (result != 0) {
            anyStatusError = true;
        } else {
            printf("%s audio device set to \"%s\"\n", deviceTypeName(kAudioTypeInput), requestedDeviceName);
        }
    }

    // output
    newDeviceID = getRequestedDeviceID(requestedDeviceName, kAudioTypeOutput);
    if (newDeviceID != nil) {
        result = setOneDevice(newDeviceID, kAudioTypeOutput);
        if (result != 0) {
            anyStatusError = true;
        } else {
            printf("%s audio device set to \"%s\"\n", deviceTypeName(kAudioTypeOutput), requestedDeviceName);
        }
    }

    // system
    newDeviceID = getRequestedDeviceID(requestedDeviceName, kAudioTypeSystemOutput);
    if (newDeviceID != nil) {
        result = setOneDevice(newDeviceID, kAudioTypeSystemOutput);
        if (result != 0) {
            anyStatusError = true;
        } else {
            printf("%s audio device set to \"%s\"\n", deviceTypeName(kAudioTypeSystemOutput), requestedDeviceName);
        }
    }

    if (anyStatusError) {
        return 1;
    }

    return 0;
}

int cycleNext(ASDeviceType typeRequested) {
    int result;
    bool anyStatusError = false;
    if (typeRequested == kAudioTypeAll) {
        result = cycleNextForOneDevice(kAudioTypeInput);
        if (result != 0) {
            anyStatusError = true;
        }
        result = cycleNextForOneDevice(kAudioTypeOutput);
        if (result != 0) {
            anyStatusError = true;
        }
        result = cycleNextForOneDevice(kAudioTypeSystemOutput);
        if (result != 0) {
            anyStatusError = true;
        }

        if (anyStatusError) {
            return 1;
        }
        return 0;

    } else {
        return cycleNextForOneDevice(typeRequested);
    }
}

int cycleNextForOneDevice(ASDeviceType typeRequested) {
    char requestedDeviceName[256];

    // get current device of requested type
    AudioDeviceID chosenDeviceID = getCurrentlySelectedDeviceID(typeRequested);
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
    int result = setDevice(chosenDeviceID, typeRequested);
    if (result == 0) {
        getDeviceName(chosenDeviceID, requestedDeviceName);
        printf("%s audio device set to \"%s\"\n", deviceTypeName(typeRequested), requestedDeviceName);
    }
    return result;

}

OSStatus setSpeakerLayout(ASDeviceType typeRequested, ASLayoutType layoutRequested, int *channelNumber) {
    AudioDeviceID currentDeviceID = kAudioDeviceUnknown;
    char currentDeviceName[256];
    UInt32 propertySize;
    char strLayoutRequested[256];

    currentDeviceID = getCurrentlySelectedDeviceID(typeRequested);
    getDeviceName(currentDeviceID, currentDeviceName);

    AudioObjectPropertyAddress propertyAddress = {
        .mSelector  = kAudioDevicePropertyPreferredChannelLayout,
        .mScope     = kAudioDevicePropertyScopeOutput,
        .mElement   = kAudioObjectPropertyElementMaster,
    };

    if (AudioObjectHasProperty(currentDeviceID, &propertyAddress)) {
        propertySize = 0;

        AudioObjectGetPropertyDataSize(currentDeviceID, &propertyAddress, 0, NULL, &propertySize);
        AudioChannelLayout* layout = (AudioChannelLayout*)malloc(propertySize);

        //layout->mChannelLayoutTag = kAudioChannelLayoutTag_Atmos_5_1_4;
        switch (layoutRequested) {
            case kL5_1_4:
                sprintf(strLayoutRequested, "5.1.4 Atmos L R C LFE Ls Rs Vhl Vhr Ltr Rtr");
                layout->mNumberChannelDescriptions = 10;

                layout->mChannelDescriptions[channelNumber[0]].mChannelLabel = kAudioChannelLabel_Left;
                layout->mChannelDescriptions[channelNumber[1]].mChannelLabel = kAudioChannelLabel_Right;
                layout->mChannelDescriptions[channelNumber[2]].mChannelLabel = kAudioChannelLabel_Center;
                layout->mChannelDescriptions[channelNumber[3]].mChannelLabel = kAudioChannelLabel_LFEScreen;
                layout->mChannelDescriptions[channelNumber[4]].mChannelLabel = kAudioChannelLabel_LeftSurround;
                layout->mChannelDescriptions[channelNumber[5]].mChannelLabel = kAudioChannelLabel_RightSurround;
                layout->mChannelDescriptions[channelNumber[6]].mChannelLabel = kAudioChannelLabel_VerticalHeightLeft;
                layout->mChannelDescriptions[channelNumber[7]].mChannelLabel = kAudioChannelLabel_VerticalHeightRight;
                layout->mChannelDescriptions[channelNumber[8]].mChannelLabel = kAudioChannelLabel_LeftTopRear;
                layout->mChannelDescriptions[channelNumber[9]].mChannelLabel = kAudioChannelLabel_RightTopRear;
                break;

            case kL7_1_2:
                sprintf(strLayoutRequested, "7.1.2 Atmos L R C LFE Ls Rs Rls Rrs Ltm Rtm");
                layout->mNumberChannelDescriptions = 10;

                layout->mChannelDescriptions[channelNumber[0]].mChannelLabel = kAudioChannelLabel_Left;
                layout->mChannelDescriptions[channelNumber[1]].mChannelLabel = kAudioChannelLabel_Right;
                layout->mChannelDescriptions[channelNumber[2]].mChannelLabel = kAudioChannelLabel_Center;
                layout->mChannelDescriptions[channelNumber[3]].mChannelLabel = kAudioChannelLabel_LFEScreen;
                layout->mChannelDescriptions[channelNumber[4]].mChannelLabel = kAudioChannelLabel_LeftSurround;
                layout->mChannelDescriptions[channelNumber[5]].mChannelLabel = kAudioChannelLabel_RightSurround;
                layout->mChannelDescriptions[channelNumber[6]].mChannelLabel = kAudioChannelLabel_RearSurroundLeft;
                layout->mChannelDescriptions[channelNumber[7]].mChannelLabel = kAudioChannelLabel_RearSurroundRight;
                layout->mChannelDescriptions[channelNumber[8]].mChannelLabel = kAudioChannelLabel_LeftTopMiddle;
                layout->mChannelDescriptions[channelNumber[9]].mChannelLabel = kAudioChannelLabel_RightTopMiddle;
                break;
            case kL9_1_6:
                sprintf(strLayoutRequested, "9.1.6 Atmos L R C LFE Ls Rs Rls Rrs Lw Rw Vhl Vhr Ltm Rtm Ltr Rtr");
                layout->mNumberChannelDescriptions = 16;

                layout->mChannelDescriptions[channelNumber[0]].mChannelLabel = kAudioChannelLabel_Left;
                layout->mChannelDescriptions[channelNumber[1]].mChannelLabel = kAudioChannelLabel_Right;
                layout->mChannelDescriptions[channelNumber[2]].mChannelLabel = kAudioChannelLabel_Center;
                layout->mChannelDescriptions[channelNumber[3]].mChannelLabel = kAudioChannelLabel_LFEScreen;
                layout->mChannelDescriptions[channelNumber[4]].mChannelLabel = kAudioChannelLabel_LeftSurround;
                layout->mChannelDescriptions[channelNumber[5]].mChannelLabel = kAudioChannelLabel_RightSurround;
                layout->mChannelDescriptions[channelNumber[6]].mChannelLabel = kAudioChannelLabel_RearSurroundLeft;
                layout->mChannelDescriptions[channelNumber[7]].mChannelLabel = kAudioChannelLabel_RearSurroundRight;
                layout->mChannelDescriptions[channelNumber[8]].mChannelLabel = kAudioChannelLabel_LeftWide;
                layout->mChannelDescriptions[channelNumber[9]].mChannelLabel = kAudioChannelLabel_RightWide;
                layout->mChannelDescriptions[channelNumber[10]].mChannelLabel = kAudioChannelLabel_VerticalHeightLeft;
                layout->mChannelDescriptions[channelNumber[11]].mChannelLabel = kAudioChannelLabel_VerticalHeightRight;
                layout->mChannelDescriptions[channelNumber[12]].mChannelLabel = kAudioChannelLabel_LeftTopMiddle;
                layout->mChannelDescriptions[channelNumber[13]].mChannelLabel = kAudioChannelLabel_RightTopMiddle;
                layout->mChannelDescriptions[channelNumber[14]].mChannelLabel = kAudioChannelLabel_LeftTopRear;
                layout->mChannelDescriptions[channelNumber[15]].mChannelLabel = kAudioChannelLabel_RightTopRear;
                break;
            default:
                return 0;
        }

        for (UInt32 i = 0; i < layout->mNumberChannelDescriptions; i++) {
            layout->mChannelDescriptions[i].mChannelFlags = kAudioChannelFlags_AllOff;
        }

        printf("Setting device %s to %s\n", currentDeviceName, strLayoutRequested);
        return AudioObjectSetPropertyData(currentDeviceID, &propertyAddress, 0, NULL, propertySize, layout);
    }
    return 0;
}


OSStatus setMute(ASDeviceType typeRequested, ASMuteType muteRequested) {
    AudioDeviceID currentDeviceID = kAudioDeviceUnknown;
    char currentDeviceName[256];
    
    currentDeviceID = getCurrentlySelectedDeviceID(typeRequested);
    getDeviceName(currentDeviceID, currentDeviceName);
    
    UInt32 scope = kAudioObjectPropertyScopeInput;
    
    switch(typeRequested) {
        case kAudioTypeInput:
            scope = kAudioObjectPropertyScopeInput;
            break;
        case kAudioTypeOutput:
            scope = kAudioObjectPropertyScopeOutput;
            break;
        case kAudioTypeSystemOutput:
            scope = kAudioObjectPropertyScopeGlobal;
            break;
    }

    AudioObjectPropertyAddress propertyAddress = {
        .mSelector  = kAudioDevicePropertyMute,
        .mScope     = scope,
        .mElement   = kAudioObjectPropertyElementMain,
    };

    UInt32 muted = (UInt32)muteRequested;
    UInt32 propertySize = sizeof(muted);

    OSStatus status;
    if (muteRequested == kToggleMute) {
        UInt32 dataSize;
        status = AudioObjectGetPropertyDataSize(currentDeviceID, &propertyAddress, 0, NULL, &dataSize);
        if (status != noErr) {
            return status;
        }
        status = AudioObjectGetPropertyData(currentDeviceID, &propertyAddress, 0, NULL, &propertySize, &muted);
        if (status != noErr) {
            return status;
        }
        muted = !muted;
    }

    printf("Setting device %s to %s\n", currentDeviceName, muted ? "muted": "unmuted");

    return AudioObjectSetPropertyData(currentDeviceID, &propertyAddress, 0, NULL, propertySize, &muted);
}

void showAllDevices(ASDeviceType typeRequested, ASOutputType outputRequested) {
    UInt32 propertySize;
    AudioObjectPropertyAddress propertyAddress;
    AudioDeviceID dev_array[64];
    int numberOfDevices = 0;
    ASDeviceType device_type;
    char deviceName[256];

    propertyAddress.mSelector = kAudioHardwarePropertyDevices;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    propertySize = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize);
    numberOfDevices = propertySize / sizeof(AudioDeviceID);

    AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, dev_array);

    for (int i = 0; i < numberOfDevices; ++i) {
        switch (typeRequested) {
            case kAudioTypeInput:
                if (!isAnInputDevice(dev_array[i]))
                    continue;
                device_type = kAudioTypeInput;
                break;
            case kAudioTypeOutput:
                if (!isAnOutputDevice(dev_array[i]))
                    continue;
                device_type = kAudioTypeOutput;
                break;
            case kAudioTypeSystemOutput:
                device_type = getDeviceType(dev_array[i]);
                if (device_type != kAudioTypeOutput)
                    continue;
                break;
            default:
                break;
        }

        getDeviceName(dev_array[i], deviceName);

        switch (outputRequested) {
            case kFormatHuman:
                printf("%s\n", deviceName);
                break;
            case kFormatCLI:
                printf("%s,%s,%u,%s\n", deviceName, deviceTypeName(device_type), dev_array[i], getDeviceUID(dev_array[i]));
                break;
            case kFormatJSON:
                printf("{\"name\": \"%s\", \"type\": \"%s\", \"id\": \"%u\", \"uid\": \"%s\"}\n", deviceName, deviceTypeName(device_type), dev_array[i], getDeviceUID(dev_array[i]));
                break;
            default:
                break;
        }
    }
}
