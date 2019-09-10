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
	printf("Usage: %s [-a] [-c] [-t type] [-n] -s device_name\n"
           "  -a             : shows all devices\n"
           "  -c             : shows current device\n\n"
           "  -f format      : output format (cli/human/json). Defaults to human.\n"
           "  -t type        : device type (input/output/system).  Defaults to output.\n"
           "  -n             : cycles the audio device to the next one\n"
           "  -s device_name : sets the audio device to the given device by name\n\n",appName);
}

int runAudioSwitch(int argc, const char * argv[]) {
	char requestedDeviceName[256];
	AudioDeviceID chosenDeviceID = kAudioDeviceUnknown;
	ASDeviceType typeRequested = kAudioTypeUnknown;
    ASOutputType outputRequested = kFormatHuman;
    char * FORMAT_CLI = "cli";
    char * FORMAT_JSON = "json";
    char * format;
	int function = 0;

	int c;
	while ((c = getopt(argc, (char **)argv, "hacntf:s:")) != -1) {
		switch (c) {
            case 'f':
                // format
                format = strdup(optarg);
                if (strncmp(format, FORMAT_CLI, 6) == 0) {
                    outputRequested = kFormatCLI;
                } else if (strncmp(format, FORMAT_JSON, 6) == 0) {
                    outputRequested = kFormatJSON;
                } else {
                    outputRequested = kFormatHuman;
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
				
			case 's':
				// set the requestedDeviceName
				function = kFunctionSetDevice;
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
		showCurrentlySelectedDeviceID(typeRequested);
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
	
	// get the default output device
	propertySize = sizeof(deviceID);
	switch(typeRequested) {
		case kAudioTypeInput: 
			AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propertySize, &deviceID);
			break;
		case kAudioTypeOutput:
			AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propertySize, &deviceID);
			break;
		case kAudioTypeSystemOutput:
			AudioHardwareGetProperty(kAudioHardwarePropertyDefaultSystemOutputDevice, &propertySize, &deviceID);
			break;
        default: break;
			
	}
	
	return deviceID;
}

void getDeviceName(AudioDeviceID deviceID, char * deviceName) {
	UInt32 propertySize = 256;
	AudioDeviceGetProperty(deviceID, 0, false, kAudioDevicePropertyDeviceName, &propertySize, deviceName);  
}

// returns kAudioTypeInput or kAudioTypeOutput
ASDeviceType getDeviceType(AudioDeviceID deviceID) {
	UInt32 propertySize = 256;
	
	// if there are any output streams, then it is an output
	AudioDeviceGetPropertyInfo(deviceID, 0, false, kAudioDevicePropertyStreams, &propertySize, NULL);
	if (propertySize > 0) return kAudioTypeOutput;
	
	// if there are any input streams, then it is an input
	AudioDeviceGetPropertyInfo(deviceID, 0, true, kAudioDevicePropertyStreams, &propertySize, NULL);
	if (propertySize > 0) return kAudioTypeInput;
	
	return kAudioTypeUnknown;
}

bool isAnOutputDevice(AudioDeviceID deviceID) {
	UInt32 propertySize = 256;
	
	// if there are any output streams, then it is an output
	AudioDeviceGetPropertyInfo(deviceID, 0, false, kAudioDevicePropertyStreams, &propertySize, NULL);
	if (propertySize > 0) return true;
    
    return false;
}

bool isAnInputDevice(AudioDeviceID deviceID) {
	UInt32 propertySize = 256;
	
	// if there are any input streams, then it is an input
	AudioDeviceGetPropertyInfo(deviceID, 0, true, kAudioDevicePropertyStreams, &propertySize, NULL);
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

void showCurrentlySelectedDeviceID(ASDeviceType typeRequested) {
	AudioDeviceID currentDeviceID = kAudioDeviceUnknown;
	char currentDeviceName[256];
	
	currentDeviceID = getCurrentlySelectedDeviceID(typeRequested);
	getDeviceName(currentDeviceID, currentDeviceName);
	printf("%s\n",currentDeviceName);
}


AudioDeviceID getRequestedDeviceID(char * requestedDeviceName, ASDeviceType typeRequested) {
	UInt32 propertySize;
	AudioDeviceID dev_array[64];
	int numberOfDevices = 0;
	char deviceName[256];
	
	AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &propertySize, NULL);
	// printf("propertySize=%d\n",propertySize);
	
	AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propertySize, dev_array);
	numberOfDevices = (propertySize / sizeof(AudioDeviceID));
	// printf("numberOfDevices=%d\n",numberOfDevices);
	
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
	UInt32 propertySize;
	AudioDeviceID dev_array[64];
	int numberOfDevices = 0;
	AudioDeviceID first_dev = kAudioDeviceUnknown;
	int found = -1;
	
	AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &propertySize, NULL);
	// printf("propertySize=%d\n",propertySize);
	
	AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propertySize, dev_array);
	numberOfDevices = (propertySize / sizeof(AudioDeviceID));
	// printf("numberOfDevices=%d\n",numberOfDevices);
	
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

	switch(typeRequested) {
		case kAudioTypeInput: 
			AudioHardwareSetProperty(kAudioHardwarePropertyDefaultInputDevice, propertySize, &newDeviceID);
			break;
		case kAudioTypeOutput:
			AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, propertySize, &newDeviceID);
			break;
		case kAudioTypeSystemOutput:
			AudioHardwareSetProperty(kAudioHardwarePropertyDefaultSystemOutputDevice, propertySize, &newDeviceID);
			break;
        default: break;
	}
	
}

void showAllDevices(ASDeviceType typeRequested, ASOutputType outputRequested) {
	UInt32 propertySize;
	AudioDeviceID dev_array[64];
	int numberOfDevices = 0;
	ASDeviceType device_type;
	char deviceName[256];
	
	AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &propertySize, NULL);
	
	AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propertySize, dev_array);
	numberOfDevices = (propertySize / sizeof(AudioDeviceID));
	
	for(int i = 0; i < numberOfDevices; ++i) {

		switch(typeRequested) {
			case kAudioTypeInput:
				if (!isAnInputDevice(dev_array[i])) continue;
                device_type = kAudioTypeInput;
				break;
                
			case kAudioTypeOutput:
				if (!isAnOutputDevice(dev_array[i])) continue;
                device_type = kAudioTypeOutput;
				break;

			case kAudioTypeSystemOutput:
                device_type = getDeviceType(dev_array[i]);
				if (device_type != kAudioTypeOutput) continue;
				break;
            default: break;
		}
		
		getDeviceName(dev_array[i], deviceName);
        switch(outputRequested) {
            case kFormatHuman:
                printf("%s (%s)\n",deviceName,deviceTypeName(device_type));
                break;
            case kFormatCLI:
                printf("%s,%s\n",deviceName,deviceTypeName(device_type));
            case kFormatJSON:
                printf("{\"name\": \"%s\", \"type\": \"%s\"}\n",deviceName,deviceTypeName(device_type));
                break;

            default: break;
        }
	}
}
