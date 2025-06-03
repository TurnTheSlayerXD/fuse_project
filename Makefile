CFLAGS=-Wall -Wextra -O2
SRC=./src/
BUILD=./make_build/
INCLUDE=-I./include -I./fuse-3.17.2/include -I./fuse-3.17.2/build -I./curl-8.13.0/include/curl
LIB=-L /usr/lib/x86_64-linux-gnu -l:libcurl.so.4.8.0 -L ./fuse-3.17.2/build/lib -l:libfuse3.so.3.17.2

all: fuse_test.exe

fuse_test.exe: $(BUILD)main.o $(BUILD)buffer.o $(BUILD)tg_config.o $(BUILD)tg_file.o $(BUILD)tg_storage.o
	gcc $(BUILD)main.o $(BUILD)buffer.o $(BUILD)tg_config.o $(BUILD)tg_file.o $(BUILD)tg_storage.o -o fuse_test.exe $(LIB) $(CFLAGS)

$(BUILD)main.o: main.c
	gcc -c main.c -o $(BUILD)main.o $(CFLAGS) $(INCLUDE)  

$(BUILD)buffer.o: $(SRC)buffer.c
	gcc -c $(SRC)buffer.c -o $(BUILD)buffer.o $(CFLAGS) $(INCLUDE)

$(BUILD)tg_config.o: $(SRC)tg_config.c
	gcc -c $(SRC)tg_config.c -o $(BUILD)tg_config.o $(CFLAGS) $(INCLUDE)

$(BUILD)tg_file.o: $(SRC)tg_file.c
	gcc -c $(SRC)tg_file.c -o $(BUILD)tg_file.o $(CFLAGS) $(INCLUDE)

$(BUILD)tg_storage.o: $(SRC)tg_storage.c
	gcc -c $(SRC)tg_storage.c -o $(BUILD)tg_storage.o $(CFLAGS) $(INCLUDE)