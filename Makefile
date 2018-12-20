CFLAGS=-Isrc -Wall

all: build/test_word_list build/test_co_occur build/test_trie build/word_analysis

build/test_word_list: src/word_list.c test/test_word_list.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

build/test_co_occur: src/co_occur.c test/test_co_occur.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

build/test_trie: src/word_list.c src/trie.c test/test_trie.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS)

build/word_analysis: src/*.c tools/word_analysis.c
	mkdir -p build
	gcc -o $@ $^ -O3 $(CFLAGS)

clean:
	rm -rf build
