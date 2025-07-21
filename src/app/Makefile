CC = gcc
CFLAGS = -I./cJSON
LIBS = -lws2_32
SRC = main.c cJSON/cJSON.c mongoose/mongoose.c
OUT = main.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	del /Q $(OUT)