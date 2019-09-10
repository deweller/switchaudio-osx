TARGET = SwitchAudioSource
OUTPUT = build/Release/$(TARGET)
CC=$(wildcard Source/*.c)

build: $(OUTPUT)

$(OUTPUT): $(CC)
	xcodebuild -target $(TARGET)
