.PHONY: all clean

CC = gcc
CFLAGS = -I./cJSON -I./coverage_path_planning -I./coverage_path_planning/boustrophedon_cellular_decomposition
LIBS = -lws2_32
SRC = main.c webserver.c \
	coverage_path_planning/coverage_path_planning.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/bcd_event_list_building.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/bcd_cell_computation.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/bcd_coverage_planning.c \
	../../dependencies/cJSON/cJSON.c \
	../../dependencies/mongoose/mongoose.c
BUILD_DIR = build
OUT = $(BUILD_DIR)/main.exe

all: $(BUILD_DIR) $(OUT)

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)