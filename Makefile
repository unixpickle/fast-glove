CFLAGS=-Isrc -Wall

all: build/test_word_list build/test_co_occur

build/test_word_list: src/word_list.c test/test_word_list.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

build/test_co_occur: src/co_occur.c test/test_co_occur.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

clean:
	rm -rf build
