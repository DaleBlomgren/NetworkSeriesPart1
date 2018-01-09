#include <sys/types.h>
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
#include <string.h>
#include <dirent.h>

#define MAXBUFSIZE 100
#define FILEMAXSIZE 2000
char *end_flag = "$$&==&";

int main (int argc, char * argv[])
{
	int sock;                           //This will be our socket
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length; 
	//char cmd[MAXBUFSIZE];
	char msg[MAXBUFSIZE] = "";
	//char file_name[MAXBUFSIZE];

	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE]; 
	int i = 0;            //a buffer to store our received message

	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine

	//Causes the system to create a generic socket of type UDP (datagram)
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		printf("unable to create socket");
	}


	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	remote_length = sizeof(remote);
	while(i == 0){
		//waits for an incoming message
		bzero(buffer,sizeof(buffer));
		nbytes = recvfrom(sock, &buffer, sizeof(buffer), 0, (struct sockaddr *)&remote, &remote_length);
	
		if (nbytes < 0)
		{
			printf("Error on recvfrom()");
		}


		printf("blah: %s\n", buffer);
		char * sCommand = strtok (buffer, " ");
	
		int n_spaces = 0;
		char ** user_input = NULL;

		while (sCommand){
    		user_input = realloc (user_input, sizeof (char*) * ++n_spaces);

    		if (user_input == NULL)
    			exit (-1); 

    		user_input[n_spaces-1] = sCommand;

    		sCommand = strtok (NULL, " ");
   		}
    	
    	if ((sizeof(user_input) / sizeof(user_input[0])) > 3){
    		printf("Not recognize command %s.  Exiting...", buffer);
    		exit(0);
    	}

    	strtok(user_input[0], "\n");
    	strtok(user_input[1], "\n");
    	printf("user_input[0]: %s", user_input[0]);
    	printf("user_input[1]: %s", user_input[1]);
    	


		if (strcmp(user_input[0], "get") == 0){
			int n, fd;
    		char buf[MAXBUFSIZE];
    		strncpy(msg, "get", MAXBUFSIZE);
    		sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&remote, remote_length);//++

    		fd = open(user_input[1], O_RDONLY);
    		if (fd == -1){
    			printf("Error finding file %s. Exiting...\n", user_input[1]);
    			strncpy(msg, "filenotfound", MAXBUFSIZE);
    			sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&remote, remote_length);
    			exit(0);
    		}

    		while ((n = read(fd, buf, MAXBUFSIZE)) > 0) {
    			sendto(sock, buf, n, 0, (struct sockaddr *) &remote, remote_length);
    		}
    		sendto(sock, end_flag, strlen(end_flag), 0, (struct  sockaddr *)&remote, remote_length);
    		close(fd);
		}
	
		else if (strcmp(user_input[0], "put") == 0){
			int fd, n;
		
			char buf[MAXBUFSIZE];
			strncpy(msg, "put", MAXBUFSIZE);
			nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
			//nbytes = recvfrom(sock, &buf, sizeof(buf), 0, (struct sockaddr *)&remote, &remote_length);
			fd = open(user_input[1], O_RDWR | O_CREAT, 0666);
			if(fd == -1){
				printf("unable to open %s, exiting...\n", user_input[1]);
				exit(0);
			}
			while ((n = recvfrom(sock, buf, MAXBUFSIZE, 0,(struct sockaddr *)&remote, &remote_length))){
				buf[n] = 0;
				if(!(strcmp(buf, end_flag))){
					break;
				}
				write(fd, buf, n);
			}
			close(fd);
		}
	
		else if (strcmp(user_input[0], "delete") == 0){
			char buf[MAXBUFSIZE];
			strncpy(msg, "delete", MAXBUFSIZE);
			nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
			nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
			if (strcmp(buf, "ok") == 0){
				if (remove(user_input[1]) == 0){
					strncpy(msg, "Deleted", MAXBUFSIZE);
					sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
				}
				else{
					strncpy(msg, "Unable to delete file", MAXBUFSIZE);
					sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
				}
			}
			else{
				printf("server communication error, exiting...\n");
				exit(0);
			}
		}
	
		else if (strcmp(user_input[0], "ls") == 0){
			printf("PINGPINGPING\n\n");
			char buf[MAXBUFSIZE];
			//char msg[MAXBUFSIZE] = "";
			DIR *dir;
			struct dirent *dp;
		
			strncpy(msg, "ls", MAXBUFSIZE);
			nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
			nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
			if (strcmp(buf, "ok") == 0){
				if ((dir = opendir(".")) == NULL){
					printf("Cannot open .");
					exit(0);
				}
			
				while((dp = readdir(dir)) != NULL){
					strncpy(msg, dp->d_name, MAXBUFSIZE);
					if((strcmp(msg, ".") == 0) || (strcmp(msg, "..") == 0)){
						int i = 0;
					}
					else{
					sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
					//printf("Sent: %s \n", msg);
					}
				}
				strncpy(msg, "done", MAXBUFSIZE);
				sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
				//bzero(&msg, sizeof(msg));

			}
			
		}
	
		else if (strcmp(user_input[0], "exit") == 0){
			printf("Exiting...");
			strncpy(msg, "exit", MAXBUFSIZE);
			nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
			exit(0);
		}
		
		else{
			printf("Invalid input.  Exiting...\n");
			sendto(sock, "error", sizeof("error"), 0, (struct sockaddr *)&remote, remote_length);
			close(sock);
			exit(0);
		}
		//nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
		if (nbytes < 0)
		{
			printf("Error on sendto()");
		}
	}
	printf("Transaction complete. Closing...\n");
	close(sock);
}

