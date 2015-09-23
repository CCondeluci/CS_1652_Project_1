//University of Pittsburgh
//9-22-15
//Brian Lester, bld20@pitt.edu
//Carmen Condeluci, crc73@pitt.edu
//CS1652 Project 1 - HTTP Client

#include "minet_socket.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define BUFSIZE 1024

int main(int argc, char * argv[]) {
  int clientSocket;
  struct hostent *host;
  struct sockaddr_in addr;
  int resp_code = 0;
  fd_set set;
  char buf[BUFSIZE]; 
  std::string header;
  std::string response;
  std::size_t position;
  std::size_t position2;
  int connection;
  int sent;
  int read;
  bool found = false;

  char * server_name = NULL;
  int server_port    = -1;
  char * server_path = NULL;
  char * req         = NULL;
  bool ok            = false;

  /*parse args */
  if (argc != 5) {
	  fprintf(stderr, "usage: http_client k|u server port path\n");
	  exit(-1);
  }

  server_name = argv[2];
  server_port = atoi(argv[3]);
  server_path = argv[4];

  // need to free at error
  req = (char *)malloc(strlen("GET  HTTP/1.0\r\n\r\n") 
			 + strlen(server_path) + 1);  

  /* initialize */
  if (toupper(*(argv[1])) == 'K') { 

	 minet_init(MINET_KERNEL);
   
  } else if (toupper(*(argv[1])) == 'U') { 
	
	 minet_init(MINET_USER);
	 
  } else {
	  fprintf(stderr, "First argument must be k or u\n");
	  free(req);
	  exit(-1);
  }

  /* make socket */
  clientSocket = minet_socket(SOCK_STREAM);
  if (clientSocket < 0) {
  	fprintf(stderr, "Error creating the Socket.\n");
  	free(req);
  	exit(-1);
  }

  /* get host IP address  */
  /* Hint: use gethostbyname() */
  host = gethostbyname(server_name);
  if (host == NULL) {
  	fprintf(stderr, "Error getting host IP.\n");
  	free(req);
  	exit(-1);
  }

  /* set address */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);

  /* connect to the server socket */
  connection = minet_connect(clientSocket, &addr);
  if (connect < 0) {
    fprintf(stdout, "Error connecting to Socket.\n");
    free(req);
    exit(-1);
  }

  /* send request message */
  sprintf(req, "GET %s HTTP/1.0\r\n\r\n", server_path);
  
  sent = minet_write(clientSocket, req, strlen(req));
  if (sent <= 0) {
    fprintf(stdout, "Error sending request.\n");
    free(req);
    exit(-1);
  }

  /* wait till socket can be read. */
  /* Hint: use select(), and ignore timeout for now. */
  FD_ZERO(&set);
  FD_SET(clientSocket, &set);
  if (minet_select(clientSocket+1, &set, NULL, NULL, NULL) < 0) {

    fprintf(stdout, "Error selecting socket.\n");
    free(req);

    minet_close(clientSocket);
    minet_deinit();

    exit(-1);

  }

  /* first read loop -- read headers */
  read = minet_read(clientSocket, buf, BUFSIZE - 1);

  if (read <= 0) {
  	fprintf(stderr, "Error reading from socket.\n");
  	free(req);

    minet_close(clientSocket);
    minet_deinit();

  	exit(-1);
  }
  buf[read] = '\0';

  while (read > 0) {
    response += std::string(buf);
    position = response.find("\r\n\r\n", 0);

    if (position != std::string::npos) {
    	header += response.substr(0,position);
    	response = response.substr(position+4);
    	break;
    } else {
    	header += response;
    }

    read = minet_read(clientSocket, buf, BUFSIZE - 1);
    buf[read] = '\0';
  }

  /* examine return code */   
  //Skip "HTTP/1.0"
  //remove the '\0'
  position = header.find(" ");
  position2 = header.find(" ", position);
  resp_code = std::stoi(header.substr(position, position2));
  //std::cout << resp_code;

  // Normal reply has return code 200
  if (resp_code == 200) {
  	ok = true;
  }
  /* print first part of response: header, error code, etc. */
  std::cout << header << std::endl;
  /* second read loop -- print out the rest of the response: real web content */
  while (read > 0) {
  	minet_select(clientSocket+1, &set, NULL, NULL, NULL);
  	read = minet_read(clientSocket, buf, BUFSIZE - 1);
  	buf[read] = '\0';
    response += std::string(buf);
  }

  // print response
  std::cout << response << std::endl;

  /*close socket and deinitialize */
  minet_close(clientSocket);
  minet_deinit();
  free(req);
  
  if (ok) {
	  return 0;
  } else {
	  return -1;
  }
}
