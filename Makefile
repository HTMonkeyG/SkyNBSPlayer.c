DIST_DIR = ./dist
SRC_DIR = ./src
RES_DIR = ./res
 
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst %.c, $(DIST_DIR)/%.o, $(notdir $(SRC))) $(DIST_DIR)/res.o

TARGET = skycol-nbs.exe
BIN_TARGET = $(DIST_DIR)/$(TARGET)

CC = gcc
PARAM = -Wall #-Os -ffunction-sections -fdata-sections -Wl,--gc-sections -static -flto -s
LINK = -lwinmm -lcomdlg32

$(BIN_TARGET): $(OBJ)
	$(CC) $(PARAM) $(OBJ) -o $@ $(LINK)

$(DIST_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(CC) $(PARAM) -c $< -o $@

$(DIST_DIR)/res.o: $(RES_DIR)/manifest.xml $(RES_DIR)/res.rc
	windres -i $(RES_DIR)/res.rc -o $(DIST_DIR)/res.o

clean:
	del .\dist\*.o
	del .\dist\*.exe