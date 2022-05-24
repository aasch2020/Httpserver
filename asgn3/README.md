# Assignment 3 - HTTPserver audit log

This program is an HTTPserver that implementes GET PUT and APPEND methods, with an audit log and multithreading support

## Building

Run the following to build the 'httpserver' program:

...
$ make httpserver
...

## Running

There are two (functional) args which is the port number for the httpserver, and the file for the audit log

...
./split 'delim' files files...
...

## Design Decisions

The notable design decisions for this document include creating structs for request and response so that the http requests and responses can be more easily read from and written to. Additionally, the audit logging functionality is bundled in with request execution files. The program uses a thread pool design with a bounded buffer and a producer consumer solution to manage threads

## Error Messages.

The program will return http error responses on error, for example 404 not found if a given file does not exist. It will also only log successful or internal server errors.

