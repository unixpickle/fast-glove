CFLAGS=-Isrc -Wall

all: build/test_word_list

build/test_word_list: src/word_list.c test/test_word_list.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

clean:
	rm -rf build
