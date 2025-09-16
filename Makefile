APP_TITLE := Mikann File Downloader
APP_DESCRIPTION := Download files via URL on 3DS
APP_AUTHOR := enswapwar

TARGET := mikann-downloader
BUILD := build
SOURCE := source
INCLUDE := include
ASSETS := assets

ICON := $(ASSETS)/icon.png
OUTPUT := $(BUILD)/$(TARGET)

LIBS := -lctru -lm

CFILES := $(wildcard $(SOURCE)/*.c)
OFILES := $(CFILES:$(SOURCE)/%.c=$(BUILD)/%.o)

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
CFLAGS := -g -Wall -O2 -mfloat-abi=hard -mtp=soft -fno-rtti -fno-exceptions -I$(INCLUDE)

.PHONY: all clean

all: $(OUTPUT).cia

$(BUILD)/%.o: $(SOURCE)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT).elf: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -o $@ $(LIBS)

$(OUTPUT).3dsx: $(OUTPUT).elf
	3dsxtool $< $@

$(OUTPUT).cia: $(OUTPUT).elf
	makerom -f cia -o $@ -elf $< -rsf meta.rsf -icon $(ICON) \
		-desc "$(APP_DESCRIPTION)" -publisher "$(APP_AUTHOR)" -title "$(APP_TITLE)"

clean:
	rm -rf $(BUILD)/*.o $(BUILD)/*.elf $(BUILD)/*.cia $(BUILD)/*.3dsx
