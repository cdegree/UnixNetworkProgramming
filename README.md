UnixNetworkProgramming
======================

Compile
======================
for normal *.c compile it with

gcc *.c -o *

Because of multithread, compile the Server.c in 4 and FileServerOne with

gcc Server.c -o Server -lpthread

Usage
======================
Server Usage:  ./Server port

Client Usage:  ./Client IP port
