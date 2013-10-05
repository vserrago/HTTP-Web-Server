HTTP-Web-Server
===============

HTTP Web Server is a simple webserver I made for my CPS730 Web Tech class. It 
is meant to serve html and text files from a folder you specify in a 
configuration file.

Features
--------

-  Serves http GET, HEAD, and post requests.
-  http post will create a file with the requested name containing the contents
   specified in the request.
-  Server runs a number of concurrent threads specified by a configuration file.
-  Server maintains a queue of incoming unhandled requests whose size is 
   specified by a configuration file.
-  Server handles several errors, including 400,403,404 and 501.


To Compile
----------

The sever was programmed for a linux environment, as specified by the 
assignment. To compile the server type

    make
    
On a linux distribution with gcc. If you want to remove the binaries, type

    make clean
    
Which will clean up the directory.


To Run
------

The make file will compile the code to an executable called server. To run, type

    ./server
    
In the directory. There are several runtime options you can specify:

-  -a, --address: The following argument specifies the address(host) that this
   server runs on.
-  -c, --config: The following argument specifies a configuration file to use.
-  -d, --debug: Enables debug logging.
-  -p, --port: The following argument specifies a port number to use.
