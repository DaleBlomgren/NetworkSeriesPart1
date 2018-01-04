##Dale's UDP File Transfer!!

This program consists of a client and server which will transfer and store files
remotely.  This solution is simplistic because it uses a UDP protocol.

Note: THIS IS PURELY FOR EDUCATIONAL PURPOSES ONLY. UDP protocol offers no 
guarentee a file will be recieved and is quite unreliable for commertial use 
(for the most part).  

You can build the client and server with the 'make' command.  Launch server with './server <port number>' and you launch client with './client <server ip> <port number>'.

The commands at your disposal are...
put <filename>
get <filename>
delete <filename>
ls
exit

Good times! 
