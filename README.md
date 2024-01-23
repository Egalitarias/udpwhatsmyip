# UDP Whats my ip

* tested on Ubuntu 23.10 and Mac Sonoma 14.2.1 only
* Default is port 8000
* Remember to ufw allow 8000


## Server

* Bind the server to a port ./server PORT
* Bind the server to a network inerface and port ./server ADDRESS PORT

```
make server
./server
```

## Client

* Specify a port ./client ADDRESS PORT
* Server can be name or ip addresss.

```
make client
./client SERVER
```

## Makefile

* make
* make client
* make server
* make clean

