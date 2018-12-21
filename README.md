# fast-glove

This is a clean & fast implementation of [GloVe](https://nlp.stanford.edu/projects/glove/). It can be used to generate word embeddings from arbitrary corpora.

# Usage

First, install Go, a C compiler, and `make`. Then compile the code by running this command in the root of the repository:

```
$ make
```

Next, create a single file containing the entire corpus. Let's call this file `/path/to/corpus`. In this file, each document should be separated by a NUL byte. The tool `./build/csv_to_docs` converts a CSV file to this NUL-separated format, assuming that each document is stored in the last column.

Next, generate a word list. You can do this using the `./build/word_dump` tool like so:

```
$ cat /path/to/corpus | ./build/word_dump 300000 >words.txt
```

In the above example, I select the `300000` most common words from the corpus.

Next, generate a co-occurrence matrix. This can take anywhere from 2-30 minutes depending on the corpus. It will use a lot of memory, as well:

```
$ cat /path/to/corpus | ./build/build_co words.txt 10 /path/to/co_occur
```

In the above example, I use a window size of `10` and save the resulting co-occurrence matrix to `/path/to/co_occur`.

Next, we can train the embedding itself. Here's an example of how to do so:

```
$ ./build/train_embed /path/to/co_occur 10 /path/to/embeddings
```

This example runs 10 epochs of training and saves the resulting embeddings to `/path/to/embeddings`. This will take anywhere from 10 minutes to a few hours, depending on how many co-occurrences you have and how many epochs you use. With 127M co-occurrences, I've done successful training runs in <20 minutes.

Finally, you can perform analysis using the embeddings. For example, here is how to find the neighbors of the word "iphone":

```
$ ./build/neighbors words.txt /path/to/embeddings iphone
neighbors for word id: 1056                   
970 apple (0.636739)                                        
1610 android (0.613653)             
1351 plus (0.607840)                 
5709 bluetooth (0.565462)                       
5203 charger (0.564842)  
4170 display (0.558018)  
2437 bottle (0.539423)          
1307 smart (0.534340)      
449 phone (0.526477)                 
1656 pack (0.511795)  
```
