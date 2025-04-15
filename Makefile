CC = gcc
CFLAGS = -Wall #-Os -ffunction-sections -fdata-sections -Wl,--gc-sections -static -flto -s
LDFLAGS = -lwinmm -lcomdlg32

TARGET = skycol-nbs.exe
BIN_TARGET = $(DIST_DIR)/$(TARGET)

DIST_DIR = ./dist
RES_DIR = ./res

SRC_DIRS = src $(wildcard src/*/)
SRC_FILES = $(wildcard src/*.c src/*/*.c)
OBJ = $(addprefix dist/, $(notdir $(SRC_FILES:.c=.o))) $(DIST_DIR)/res.o

vpath %.c $(SRC_DIRS)

$(BIN_TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(DIST_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIST_DIR)/res.o: $(RES_DIR)/manifest.xml $(RES_DIR)/res.rc
	windres -i $(RES_DIR)/res.rc -o $(DIST_DIR)/res.o

clean:
	del .\dist\*.o
	del .\dist\*.exe