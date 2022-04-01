/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name: Devan Patel
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 1024
#define LISTENQ 1024
#define PATH 1
#define NOPATH -1

/*
 * Function prototypes
 */

void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

typedef struct sockaddr SA;

/* 
 * main - Main routine for the proxy program 
 */
 
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }
    
    int listenfd, connfd,serverconnfd;
	int reqPort; 
	int pathExists;
	int len;
	int intResponse;
	int total;
    struct sockaddr_in servaddr, cliaddr;

    int port = atoi(argv[1]); //port, user arguement
	
    char recvline[MAXLINE], reqtyp[MAXLINE],httpBuffer[MAXLINE],fullPath[MAXLINE],hostWebAddress[MAXLINE],portpath[MAXLINE],strPort[MAXLINE],specificPath[MAXLINE];
    char httpRequest[MAXLINE],response[MAXLINE], fileBuffer[MAXLINE];
    char *token;


    //Web Proxy listens for requests
    
    listenfd = Socket(AF_INET, SOCK_STREAM, 0); 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    
    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	
    
    Listen(listenfd, LISTENQ);
    
    while(1){

        len = sizeof(cliaddr);

    	connfd = Accept(listenfd, (SA *) &cliaddr, (socklen_t *) &len);
    	
    	//Parsing the user request

    	read(connfd, recvline, MAXLINE);

    	sscanf(recvline, "%s %s %s", reqtyp, fullPath, httpBuffer);
    	
    	if(strstr(fullPath, "http://") != NULL) {

    		sscanf(fullPath, "http://%s", fullPath);

    	}

    	if(strstr(fullPath, ":") != NULL) {

    		strcpy(hostWebAddress, strtok(fullPath, ":"));

    		token = strtok(NULL, "");

    		strcpy(portpath, token);

    		if(strstr(portpath, "/") != NULL ) {

    			strcpy(strPort, strtok(portpath, "/"));

    			reqPort = atoi(strPort);

    			token = strtok(NULL, "");

    			strcpy(specificPath, token);

    			pathExists = PATH;
    			
    		}
			
			else {
    			strcpy(strPort, portpath);

    			pathExists = NOPATH;

    			
    		}
    	}
		
		else if(strstr(fullPath, ":") == NULL) {

    		reqPort = 80;

    		if(strstr(fullPath, "/") != NULL) {

    			strcpy(hostWebAddress, strtok(fullPath, "/"));

    			if((token = strtok(NULL, "")) != NULL) {

    				strcpy(specificPath, token);

    				pathExists = PATH;

    			}
				
				else {
    				pathExists = NOPATH;
    			}	
    		}
			
			else {

    			strcpy(hostWebAddress, fullPath);

    			pathExists = NOPATH;

    		}
    		
    	}
    	
    	if(pathExists == PATH) {

    		sprintf(httpRequest, "%s /%s HTTP/1.1\r\nHost: %s\r\n\r\n", reqtyp, specificPath, hostWebAddress);

    	}
		
		else if(pathExists == NOPATH) {

    		sprintf(httpRequest, "%s / HTTP/1.1\r\nHost: %s\r\n\r\n", reqtyp, hostWebAddress);

    	}
    	
    	//If true, then it the fields are filled out

    	if((pathExists == PATH || pathExists == NOPATH) && strcmp(reqtyp, "GET") == 0) {

    		printf("%s\n", httpRequest);

    		fflush(stdout);

    	        rio_t robust;

	    	serverconnfd = Open_clientfd(hostWebAddress, reqPort);

	    	Rio_readinitb(&robust, serverconnfd);

	    	Rio_writen(serverconnfd, httpRequest, MAXLINE);
	    	
	    	while((intResponse = Rio_readlineb(&robust, response, 1024)) > 0) {

	    		Write(connfd, response, intResponse);

	    		bzero(response, sizeof(response));

	    		total += intResponse;

	    	}
	    	}
	    	
	  
	    	
	    	FILE *fp = fopen("proxy.txt", "a");

	    	format_log_entry(fileBuffer, &cliaddr, fullPath, total);

	        fprintf(fp, "%s\n", fileBuffer);

	    	fclose(fp);

	    	Close(serverconnfd);
	    	 	
    	}
    	
  	exit(0);
    }
  



/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}


