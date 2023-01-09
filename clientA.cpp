#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
// UDP
#include "udplisten.cpp"
#include "udpsend.cpp"
//TCP
#include "tcpclient.cpp"
#include "tcpserver.cpp"


#define HOST_ADDR "127.0.0.1"
#define SERVERM_PORT "25944"


#define MAXDATASIZE 4096 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
// void *get_in_addr(struct sockaddr *sa)
// {
//     if (sa->sa_family == AF_INET) {
//         return &(((struct sockaddr_in*)sa)->sin_addr);
//     }

//     return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }


string string_concat(int argc, char *argv[]){
    // int string_len = 0
    // for(int i = 0; i < argc; i++){
    //     string_len += strlen(argv[i]) 
    // }
    // string_len += argc + 3
    // cout << "string_len: " << string_len << endl;

    // char str[HUGE];
    // strcpy(str, argv[1]);

    // for (int i = 2; i < argc; ++i)
    // {
    // strcat(str, " ");
    // strcat(str, argv[i]);
    // }

    string msg_str = "";
    for(int i = 0; i < argc-1; i++){
        msg_str = msg_str + argv[i] + " "; // add a space
    }
    msg_str = msg_str + argv[argc-1];
    return msg_str;
}

// reference: Beej's Book, on TCP.
int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(HOST_ADDR, SERVERM_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    // printf("client: connecting to %s\n", s);
    cout << "The client A is up and running." << endl;

    freeaddrinfo(servinfo); // all done with this structure


    // add by me !!!!!!!!!
    string msg_str = string_concat(argc, argv);
    if ((numbytes = send(sockfd, msg_str.c_str(), msg_str.length(), 0)) == -1) {
        perror("send");
        exit(1);
    }

    if (argc == 4){
        cout << argv[1] << " has requested to transfer " << argv[3] << " coins to " << argv[2] << "." << endl;
    }
    else if (argc == 2){
        string param_1(argv[1]);
        if (param_1 == "TXLIST"){
            cout <<"ClientA sent a sorted list request to the main server." << endl;
        }
        else{
            cout << param_1 << " sent a balance enquiry request to the main server." <<  endl;
        }
    }
    else if (argc == 3){
    	cout << argv[1] << "sent a statistics enquiry request to the mains server." << endl;
    	cout << argv[1] << " statistics are the following:" << endl;
    	cout << "Rank--Username--NumofTransactions--Total" << endl;
    }

    // end!!!!!

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    // printf("client: received '%s'\n",buf);
    cout << buf << endl; // buf is the message sent back from the main server.

    close(sockfd);

    return 0;
}
