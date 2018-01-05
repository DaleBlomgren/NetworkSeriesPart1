#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#define MAXBUFSIZE 100
#define FILEMAXSIZE 2000

char *end_flag = "$$&==&";

int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sock;                               //this will be our socket
	char buffer[MAXBUFSIZE];
	
	char cCmd[MAXBUFSIZE];
	char msg[MAXBUFSIZE] = "";
	char cmd[MAXBUFSIZE];
	unsigned int remote_len;
	struct sockaddr_in remote;              //"Internet socket address structure"

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	//Causes the system to create a generic socket of type UDP (datagram)
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		printf("unable to create socket\n");
		exit(0);
	}

	/******************
	  sendto() sends immediately.  
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
	int i = 0;

	while(i == 0){
		bzero(cmd,sizeof(cmd));
		printf("Please enter your command: ");
		fgets(cmd, MAXBUFSIZE, stdin);
		char msg[MAXBUFSIZE] = "";
		strcpy(cCmd, cmd);
		char *sCommand = strtok (cmd, " "); //strtok will mutate cmd, make a copy first | Initialize with first part of cmd
	
		int nSpaces = 0;
		char ** user_input = NULL;

		while (sCommand) {
    		user_input = realloc (user_input, sizeof (char*) * ++nSpaces);
			if (user_input == NULL){
    			exit (0); 
			}
			user_input[nSpaces-1] = sCommand;
			sCommand = strtok (NULL, " ");
    	}
    	if ((sizeof(user_input) / sizeof(user_input[0])) > 3){
    		printf("Not regognize command %s.  Exiting...", buffer);
    		exit(0);
    	}
    	strtok(user_input[0], "\n");
    	strtok(user_input[1], "\n");  
		
		
		remote_len = sizeof(remote);
		nbytes = sendto(sock, &cCmd, sizeof(cmd), 0, (struct sockaddr *)&remote, remote_len);
		if (nbytes < 0){
			printf("Error on sendto()");
		}

		// Blocks till bytes are received
		bzero(buffer,sizeof(buffer));
		printf("Waiting for response from server...\n");
		if ((nbytes = recvfrom(sock, &buffer, sizeof(buffer), 0, (struct sockaddr *)&remote, &remote_len)) < 0){
			printf("Error on recfrom()");
		} 
	
		if (strcmp(buffer, "put") == 0){
			int n, fd;
    		char buf[MAXBUFSIZE];

    		fd = open(user_input[1], O_RDONLY);
    		if (fd == -1){
    			printf("File not found, exiting...\n");
    			sendto(sock, "error", strlen("error"), 0, (struct  sockaddr *)&remote, remote_len);
    			close(sock);
    			exit(0);
    		}
    		sendto(sock, "ok", strlen("ok"), 0, (struct  sockaddr *)&remote, remote_len);
    		recvfrom(sock, buf, MAXBUFSIZE, 0,(struct sockaddr *)&remote, &remote_len);
    		if (strcmp(buf, "ok") == 0){
    			while ((n = read(fd, buf, MAXBUFSIZE)) > 0) {
    				sendto(sock, buf, n, 0, (struct sockaddr *) &remote, remote_len);
    			}
    			sendto(sock, end_flag, strlen(end_flag), 0, (struct  sockaddr *)&remote, remote_len);
    			//printf("Sent end flag\n");
    			close(fd);
    		}
    	}
    	
    	else if(strcmp(buffer, "get") == 0){
    		int fd;
			char buf[MAXBUFSIZE];
		
			fd = open(user_input[1], O_RDWR | O_CREAT, 0666);
			if(fd == -1){
				printf("unable to open %s, exiting...\n", user_input[1]);
				close(sock);
				exit(0);
			}
			printf("Recieving file from server...\n");
			while (nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0,(struct sockaddr *)&remote, &remote_len)){
				buf[nbytes] = 0;
				if(!(strcmp(buf, end_flag))){
					break;
				}
				if(strcmp(buf, "filenotfound") == 0){
					printf("File %s not found on server, server has closed.  exiting...\n", user_input[1]);
					close(fd);
					close(sock);
					remove(user_input[1]);
					exit(0);
				}	
				write(fd, buf, nbytes);
			}
			close(fd);
		
		
    	}

    	else if(strcmp(buffer, "delete") == 0){
    		char buf[MAXBUFSIZE];
    		strncpy(msg, "ok", MAXBUFSIZE);
    		
    		nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_len);
    		
    		nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_len);
    		if (strcmp(buf, "Unable to delete file") == 0)
    			printf("File deletion error on file %s\n", user_input[1]);
    	}

    	else if(strcmp(buffer, "ls") == 0){
    		char buf[MAXBUFSIZE];
    		strncpy(msg, "ok", MAXBUFSIZE);
    		
    		nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_len);
    		
    		while((nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_len))){
    			buf[nbytes] = 0;
    			if(!(strcmp(buf, "done"))){
    				break;
    			}
    			printf("%s\n", buf);
    		}
    	}

    	else if(strcmp(buffer, "exit") == 0){
    		printf("Server has shut down.  Exiting...\n");
    		close(sock);
    		exit(0);
    	}

    	else{
    		printf("Sorry, %s is invalid, exiting...\n", cmd);
    		close(sock);
    		exit(0);
    	}
    	
	}
	printf("Transaction complete. Closing...\n");
	close(sock);

	return 0;
}

