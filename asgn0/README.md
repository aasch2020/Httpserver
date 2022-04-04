# Assignment 0 - Split

This program takes input files (including stdin), and replaces the given delimiter character with a newline, then prints it to stdout.

## Building

Run the following to build the 'split' program:

...
$ make split
...

## Running

The args are the delimiter in the first position is one character '-' specifes stdin as a file input.
The files can be as many as possible, and will be listed after the delimiter

...
./split 'delim' files files...
...

## Design Decisions

The notable design decisions for this document include choosing to buffer the input in blocks as to reduce the amount of read and write.
Also opening and closing files one by one as not to have multiple files open at once.
