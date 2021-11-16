## Pipesort

pipesort reads text from standard input and writes the unique words to standard output sorted in alphabetic order, along with its word count. 

#### Synopsis (Usage):

```bash
foo@rxluke:~$ pipesort [s:l:]
```

#### Description:

* -s: words must have more than n letters, specified by user
* -l: the maximum number of letters in a word, specified by user. Letters beyond the maximum number are discarded. 

Internally, the program is organized into 3 types of processes. One single process reads the input and parse the lines into words, another process is used to execute '/usr/bin/sort', and the last process is used to suppress and count duplicated words, and write the output. 

#### Example:

assuming there is a file in the same directory called test.

```bash
foo@rxluke:~$ pipesort -s 4 -l 10 <test
```

This will parse the test file and print the result onto the console:

```bash
7         about
4         around
3         basically
15        boring
...
6         zigzag
```

