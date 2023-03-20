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
	printf("Usage: %s [-a] [-c] [-t type] [-n] -s device_name | -i device_id | -u device_uid\n"
           "  -a             : shows all devices\n"
           "  -c             : shows current device\n\n"
           "  -f format      : output format (cli/human/json). Defaults to human.\n"
           "  -t type        : device type (input/output/system).  Defaults to output.\n"
           "  -n             : cycles the audio device to the next one\n"
           "  -i device_id   : sets the audio device to the given device by id\n"
           "  -u device_uid  : sets the audio device to the given device by uid or a substring of the uid\n"
           "  -s device_name : sets the audio device to the given device by name\n\n",appName);
}

int runAudioSwitch(int argc, const char * argv[]) {
	char requestedDeviceName[256];
    char printableDeviceName[256];
    int requestedDeviceID;
    char requestedDeviceUID[256];
	AudioDeviceID chosenDeviceID = kAudioDeviceUnknown;
	ASDeviceType typeRequested = kAudioTypeUnknown;
	ASOutputType outputRequested = kFormatHuman;
	int function = 0;

	int c;
	while ((c = getopt(argc, (char **)argv, "hacnt:f:i:u:s:")) != -1) {
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

    if (function == kFunctionSetDeviceByID) {
        chosenDeviceID = (AudioDeviceID)requestedDeviceID;
        sprintf(printableDeviceName, "Device with ID: %d", chosenDeviceID);
    }

    if (function == kFunctionSetDeviceByName) {
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

    if (!chosenDeviceID) {
        printf("Please specify audio device.\n");
        showUsage(argv[0]);
        return 1;
    }
	
	// choose the requested audio device
	setDevice(chosenDeviceID, typeRequested);
	printf("%s audio device set to \"%s\"\n", deviceTypeName(typeRequested), printableDeviceName);

    return 0;
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
    
    AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &dataSize, &deviceUID);
            
    const char * deviceUID_string = CFStringGetCStringPtr(deviceUID, kCFStringEncodingASCII);
    
    CFRelease(deviceUID);
    
    return deviceUID_string;
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
        propertyAddress.mSelector = kAudioDevicePropertyDeviceUID;
        propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
        propertyAddress.mElement = dev_array[i];
        propertySize = sizeof(deviceUID);
        AudioObjectGetPropertyData(dev_array[i], &propertyAddress, 0, NULL, &propertySize, &deviceUID);

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

void setDevice(AudioDeviceID newDeviceID, ASDeviceType typeRequested) {
    AudioObjectPropertyAddress addr;
    UInt32 propertySize = sizeof(UInt32);
    addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
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
            break;
    }
    addr.mScope = kAudioObjectPropertyScopeGlobal;
    addr.mElement = kAudioObjectPropertyElementMaster;
    AudioObjectSetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, propertySize, &newDeviceID);
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
