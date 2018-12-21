CFLAGS=-Isrc -Wall

all: build/test_word_list build/test_co_occur build/test_hash_map build/word_dump build/csv_to_docs

build/test_word_list: src/word_list.c test/test_word_list.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

build/test_co_occur: src/co_occur.c test/test_co_occur.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS) 

build/test_hash_map: src/word_list.c src/hash_map.c test/test_hash_map.c
	mkdir -p build
	gcc -o $@ $^ $(CFLAGS)

build/word_dump: src/*.c tools/word_dump.c
	mkdir -p build
	gcc -o $@ $^ -O3 $(CFLAGS)

build/csv_to_docs: tools/csv_to_docs.go
	mkdir -p build
	go build -o build/csv_to_docs tools/csv_to_docs.go

clean:
	rm -rf build
