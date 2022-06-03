# CSE130 Repo

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


## Output

On a PUT request the server will respond with either a 200 OK, or a 201 CREATED on success, which indicated that the data has been written

On a GET request, the server will respond with a 200 OK, with the content of the file.

On an APPEND request, the server will respond with a 200 OK, with no content other than OK

These above outputs assume successful requests
## Design Decisions

The notable design decisions for this document include creating structs for request and response so that the http requests and responses can be more easily read from and written to. Additionally, the audit logging functionality is bundled in with request execution files. The program uses a thread pool design with a bounded buffer and a producer consumer solution to manage threads

Additional details
The read is performed in a way such that characters are read into a buffer until they either match the regex for the desired field, or the end of that field.
To produce and consume requests from the dispatcher to the main thread, the program applies the solution to the producer consumer problem, which mean that there are to CV and one lock, and the producer waits on the consumer to send a CV that there has been an element taken out of the queue, and vice versaThis ensures no two threads are changing the queue at once.
The program uses a circular bounded queue design to manage the thread queue, it is bounded so that a huge flood of new connections doesn't cause the dispatcher to infinately enqueue. 



This program additionally splits parsing the request into serveral parts, but passes buffers in between these functions.  
This allows the program to handle partial as well as all in one requests.
 


Program steps sequentially for one request:

1.Accept and enqueue the request from the dispatcher

2.Worker thread dequeues the request from the queue

3.Worker thread waits for data to be send in that connection.

4.Worker thread reads until a valid request line or \r\n is found, it passes the extra read characters to the next step.

5.Worker thread reads until \r\n\r\n or an invalid header is found, and passes the extra read characters to the next step.

6.Worker thread looks for a Content-Length header, and store it.

7.For PUT or APPEND, worker thread reads until it reads Content-Length bytes.

8.For GET, worker thread reads from the desired file and writes after writing the response header.

9.Worker thread sends response for PUT or Append.



File atomicity additions

The server now uses temporary files for PUT and APPEND requests, thus meaning that any partial put or partial append will not be written or logged until complete.

Using this in conjunction with flocks on the files, and a lock on file creation/checking if a file exists, ensure that no PUT or APPEND requests can interleave.

This creates a global critical section for checking the files for existing, and a critical section for writing each file.

Additionally, there also is a lock utilized for whenever there needs to be content written to the log. This ensures that reading and writing to the log will always be atomic.

## Notable Problems

At this point, non blocking input and output are not implemented. If a client is slow to send data, the server will simply wait until the client is finished. 

Additionally, the program is reported to timeout on test vigirours newton, however, I have been unable to reproduce this issue.

## Error Messages.

404 Not Found, when a file for a GET or PUT request is not found

500 Internal Server Error, something went wrong in processing. NOTE: the client may not see this message but it will be logged, if a client closes the connection before a request is completed or a response is sent.

