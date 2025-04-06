DIST_DIR = ./dist
SRC_DIR = ./src
RES_DIR = ./res
 
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst %.c, $(DIST_DIR)/%.o, $(notdir $(SRC))) $(DIST_DIR)/manifest.o $(DIST_DIR)/icon.o

TARGET = skycol-nbs.exe
BIN_TARGET = $(DIST_DIR)/$(TARGET)

CC = gcc
PARAM = -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -static -flto -s

$(BIN_TARGET): $(OBJ)
	$(CC) $(PARAM) $(OBJ) -o $@ -lwinmm

$(DIST_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(PARAM) -c $< -o $@

$(DIST_DIR)/manifest.o: $(RES_DIR)/manifest.xml $(RES_DIR)/manifest.rc
	windres -i $(RES_DIR)/manifest.rc -o $(DIST_DIR)/manifest.o

$(DIST_DIR)/icon.o: $(RES_DIR)/icon.ico $(RES_DIR)/icon.rc
	windres -i $(RES_DIR)/icon.rc -o $(DIST_DIR)/icon.o

clean:
	del .\dist\*.o
	del .\dist\*.exe