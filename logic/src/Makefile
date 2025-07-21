CC = gcc
CFLAGS = -I./cJSON
SRC = main.c cJSON/cJSON.c
OUT = main.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	del /Q $(OUT)