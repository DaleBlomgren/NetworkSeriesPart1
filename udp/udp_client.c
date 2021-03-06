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
	int nbytes, i = 0;                      // number of bytes send by sendto()
	int sock;                               // this will be our socket
	char buffer[MAXBUFSIZE];
	char cmd[MAXBUFSIZE];
	unsigned int remote_len;
	struct sockaddr_in remote;              // "Internet socket address structure"
	char cCmd[MAXBUFSIZE];
	char msg[MAXBUFSIZE] = "";
	char * sCommand;             
	int nSpaces = 0;

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	//Causes the system to create a generic socket of type UDP (datagram)
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		printf("unable to create socket");
	}

	while (i == 0){
		nSpaces = 0;
		bzero(&msg, sizeof(msg));
		bzero(&cmd, sizeof(cmd));
		bzero(&cCmd, sizeof(cCmd));
		
		printf("Please enter your command: ");
		fgets(cmd, MAXBUFSIZE, stdin);
		strcpy(cCmd, cmd);
		nSpaces = 0;
		char ** user_input = NULL;
		sCommand = strtok(cmd, " ");

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
	
		//Sends request
		remote_len = sizeof(remote);
		nbytes = sendto(sock, &cCmd, sizeof(cmd), 0, (struct sockaddr *)&remote, remote_len);

		if (nbytes < 0){
			printf("Error on sendto()");
		}
		
		// Blocks till bytes are received
		// Recieves confermation from server, returns command
		bzero(buffer,sizeof(buffer));

		if ((nbytes = recvfrom(sock, &buffer, sizeof(buffer), 0, (struct sockaddr *)&remote, &remote_len)) < 0){
			printf("Error on recfrom()");
		} 

		if (strcmp(buffer, "push") == 0){
			int n, fd;
    		char buf[MAXBUFSIZE];

    		fd = open(user_input[1], O_RDONLY);
    		if (fd == -1){
    			printf("error opening file exiting...");
    			exit(0);
    		}

    		while ((n = read(fd, buf, MAXBUFSIZE)) > 0) {
    			sendto(sock, buf, n, 0, (struct sockaddr *) &remote, remote_len);
    		}
    		sendto(sock, end_flag, strlen(end_flag), 0, (struct  sockaddr *)&remote, remote_len);
    		close(fd);
    	}

    	else if(strcmp(buffer, "pull") == 0){
    		int fd;
			char buf[MAXBUFSIZE];
		
			
			fd = open(user_input[1], O_RDWR | O_CREAT, 0666);
			if(fd == -1){
				printf("unable to open %s, exiting...\n", user_input[1]);
				exit(0);
			}
			while ((nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0,(struct sockaddr *)&remote, &remote_len))){
				buf[nbytes] = 0;
				if(!(strcmp(buf, end_flag))){
					break;
				}
				if(strcmp(buf, "filenotfound") == 0){
					printf("File %s not found on server, server has closed.  exiting...\n", user_input[1]);
					close(fd);
					close(sock);
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
		}

    	else if(strcmp(buffer, "ls") == 0){
    		char buf[MAXBUFSIZE] = "";
    		strncpy(msg, "ok", MAXBUFSIZE);
    		nbytes = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_len);
    		while((nbytes = recvfrom(sock, buf, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_len))){
    			buf[nbytes] = 0;
    			if(!(strcmp(buf, "done"))){
    				break;
    			}
    			printf("%s\n", buf);
    		}
    		printf("%s\n", buf);
    		bzero(&buf, sizeof(buf));
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

	return 0;
}

