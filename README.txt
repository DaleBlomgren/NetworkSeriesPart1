
This is a simple UDP solution for sending files over sockets over the internet.  It was created by Dale Judge Blomgren in 09/2017.

First, use make to build the program.  To begin the server process, you execute ./server <port you are using>.  Server will not respond but is actively listening for the client.
Execute ./client <ip address of server> <port number server is using> to begin a connection with the running server.
Commands for client are...
	put <filename>: 	This will push a file to the server
	get <filename>: 	Pull a file from the server
	delete <filename>:  delete a file from the server
	ls: 				list all files in server directory
	exit: 				exits server and client 

