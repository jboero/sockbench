# SockPerf
Simple benchmark app for UDS and TCP sockets.

# Building
Simple makefile.

# Usage
Server is multithreaded and can use either AF_UNIX (default) or AF_INET (option for IPv6 via AF_UNSPEC).  Client is a single thread and specifies UDS (default) or AF_INET port.

# Examples
Start a UNIX Domain Socket server at ./sockperf.sock:
`sockperf -s`

Start a TCP server at your port:
`sockperf -s9000`

Start a UDS client binding to ./sockperf.sock, using default blocksize 32k and default count 32k:
`sockperf -c`

Start a UDS client with custom block size and quantity:
`sockperf -c -b4096 -q16384`

Start a TCP client on your port (block size and quantity optional):
`sockperf -c9000`
