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
#include <fstream>

// UDP
#include "udplisten.cpp"
#include "udpsend.cpp"
//TCP
#include "tcpclient.cpp"
#include "tcpserver.cpp"


// for different server A, B, C, only need to change the define here
#define HOST_ADDR "127.0.0.1"
#define SERVERM_PORT "24944"
#define SERVER_PORT "22944" // SERVERB_PORT, backend server B port
#define MAXBUFLEN 100    // udp
#define INPUT_FILE "block2.txt" // input txt file

using namespace std;

// reference Beej's book, mostly on the UDP listener part.
// get sockaddr, IPv4 or IPv6:
void *get_in_addr_serverB(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, SERVER_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    // printf("listener: waiting to recvfrom...\n");
    cout<< "The serverB is up and running using UDP on port " << SERVER_PORT << "." << endl;

    addr_len = sizeof their_addr;


    while(1){
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
        cout << "The ServerB received a request from the Main Server." << endl;

        // if buf == GET_SERIAL_NUM
        char get_serial_num_str[] = "GET_SERIAL_NUM";
        char txlist_str[] = "TXLIST";
        char first_2_chars[5]; //  the first chars of the buf
        memcpy(first_2_chars, buf, 2); // only first two chars
        char write_str[] = "W "; // if need to write, buf should starts with "W "
        if (strcmp(buf, get_serial_num_str) == 0){ //buf == GET_SERIAL_NUM
            // traverse all transaction records, and return the biggest one as a string.
            char *res_str = "0"; 
            int res_str_len = 0;
            int biggest_serial_num = 0;
            int serial_num;
            string one_line;
            ifstream infile;
            infile.open (INPUT_FILE);
            while(getline(infile, one_line)) // (!infile.eof()) // To get all lines.
            {
                // getline(infile, one_line); // Saves the line in one_line.
                // split the string
                if (one_line != ""){
                    char *ptr; // declare a ptr pointer  
                    char one_line_char[one_line.size()+2]; 
                    strcpy(one_line_char, one_line.c_str());
                    ptr = strtok(one_line_char, " ");
                    // cout << "in serverA, if GET_SERIAL_NUM: ptr=" << ptr << " should be a number str" << endl;
                    serial_num = atoi(ptr);
                    // if (strcmp(res_str, ptr) < 0){ // ptr is bigger than the current biggest
                    //     cout << " in if (strcmp(res_str, ptr) < 0" << endl;
                    //     // strcpy(res_str, ptr);
                    //     res_str = ptr;
                    //     res_str_len = strlen(ptr);
                    //     cout << "after strcpy" << endl;
                    // }
                    // cout << "res_str: " << res_str <<endl;

                    if (serial_num > biggest_serial_num){
                        biggest_serial_num = serial_num;

                    }
                }


            }
            infile.close();

            // char *biggest_serial_num_str = itoa(biggest_serial_num);
            string biggest_serial_num_str = to_string(biggest_serial_num);


            // cout << "strlen(res_str): " << strlen(res_str) << endl;
            // cout << "res_str_len: " << res_str_len << endl;
            // char
            // res_str_len
            // now the res_str is the biggest serial number on this server.
            if ((numbytes = sendto(sockfd, biggest_serial_num_str.c_str(), biggest_serial_num_str.size(), 0,
                (struct sockaddr *)&their_addr, addr_len)) == -1) {
                perror("backend server response sendto");
                exit(1);
            }
               
        }

        // returns all records as one string.
        else if(strcmp(buf, txlist_str) == 0) //buf == TXLIST)
        {
            string res_str = "";
            // get all data from the txt file.
            string one_line;
            ifstream infile;
            infile.open (INPUT_FILE);
            while(getline(infile, one_line)) // !infile.eof()) // To get all lines.
            {
                // getline(infile, one_line); // Saves the line in one_line.
                string original_line = one_line;
                res_str = res_str + original_line + ";";
            }
            res_str.pop_back();
            infile.close();
            // system ("pause");

            // cout << "serverA: return all records as one string, res_str: " << res_str <<endl;

            // send the result back to the main server.
            // string res = "hello from server.cpp";
            if ((numbytes = sendto(sockfd, res_str.c_str(), res_str.length(), 0,
                (struct sockaddr *)&their_addr, addr_len)) == -1) {
                perror("backend server response sendto");
                exit(1);
            }
        }


        // if write, first 2 chars are "W ", then comes the log string.
        else if(strcmp(first_2_chars, write_str) == 0){ // write one new log to the txt file
            ofstream outfile;
            outfile.open(INPUT_FILE, ofstream::app); // append to file, do not overwrite
            char *first_2_chars_chopped = buf + 2;
            outfile << first_2_chars_chopped;
            outfile.close();

            // reply sucessfully write
            string res_str = "SUCESSFUL";
            if ((numbytes = sendto(sockfd, res_str.c_str(), res_str.length(), 0,
                (struct sockaddr *)&their_addr, addr_len)) == -1) {
                perror("backend server response: write successfully");
                exit(1);
            }

        }


        else{ // if buf is just a username, return all records contains this username on this server.
            string res_str = "";
            int return_amount = 0;
            // get all data from the txt file.
            string one_line;
            ifstream infile;
            infile.open (INPUT_FILE);
            bool found_user = false;
            while(getline(infile, one_line)) // !infile.eof()) // To get all lines.
            {
                // getline(infile, one_line); // Saves the line in one_line.
                string original_line = one_line;
                // split the string
                char *ptr; // declare a ptr pointer  
                char one_line_char[one_line.size()+2]; 
                strcpy(one_line_char, one_line.c_str());
                ptr = strtok(one_line_char, " "); // separate string by space delimiter.   
                // use while loop to check ptr is not null  
                int num_args = 0;
                char * msg_args[4];
                while (ptr != NULL)  
                {  
                    // cout << ptr  << endl; // print the string token 
                    msg_args[num_args] = ptr; 
                    ptr = strtok (NULL, " ");  
                    num_args++;
                    // if the username we want to find is in this transaction record, 
                    // add the whole record string to the res_str.
                    // if (num_args == 1 || num_args == 2){
                    //     if (strcmp(ptr, buf) == 0){ // if they are equal
                    //         res_str = res_str + original_line + "; ";
                    //         break;
                    //     }
                    // }
                }
                if (strcmp(msg_args[1], buf) == 0){ // as sender
                    return_amount = return_amount - atoi(msg_args[3]);
                    found_user = true;
                }
                else if (strcmp(msg_args[2], buf) == 0){ // as receiver
                    return_amount = return_amount + atoi(msg_args[3]);
                    found_user = true;
                }
            }
            infile.close();
            // system ("pause");

            // cout << "serverA: records with the username -> res_str: " << res_str <<endl;
            

            // send the result back to the main server.
            // string res = "hello from server.cpp";
            // if ((numbytes = sendto(sockfd, res_str.c_str(), res_str.length(), 0,
            //     (struct sockaddr *)&their_addr, addr_len)) == -1) {
            //     perror("backend server response sendto");
            //     exit(1);
            // }


            // send back the amount as a string
            // if returned_amount is a negative number, to_string will add a "-".
            string return_s;
            if (found_user){
                return_s = to_string(return_amount);
            }
            else{ // did not find user. return + to let serverM know that this backend server did not find the username.
                return_s = "+";
            }
            if ((numbytes = sendto(sockfd, return_s.c_str(), return_s.length(), 0,
                    (struct sockaddr *)&their_addr, addr_len)) == -1) {
                    perror("backend server response sendto");
                    exit(1);
                }
            
        }
        cout << "The ServerB finished sending response to the Main Server." << endl;

        

    }
    

    close(sockfd);

    return 0;
}



// old file!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// int main(void)
// {
//     int sockfd;
//     struct addrinfo hints, *servinfo, *p;
//     int rv;
//     int numbytes;
//     struct sockaddr_storage their_addr;
//     char buf[MAXBUFLEN];
//     socklen_t addr_len;
//     char s[INET6_ADDRSTRLEN];

//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
//     hints.ai_socktype = SOCK_DGRAM;
//     hints.ai_flags = AI_PASSIVE; // use my IP

//     if ((rv = getaddrinfo(NULL, SERVER_PORT, &hints, &servinfo)) != 0) {
//         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
//         return 1;
//     }

//     // loop through all the results and bind to the first we can
//     for(p = servinfo; p != NULL; p = p->ai_next) {
//         if ((sockfd = socket(p->ai_family, p->ai_socktype,
//                 p->ai_protocol)) == -1) {
//             perror("listener: socket");
//             continue;
//         }

//         if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
//             close(sockfd);
//             perror("listener: bind");
//             continue;
//         }

//         break;
//     }

//     if (p == NULL) {
//         fprintf(stderr, "listener: failed to bind socket\n");
//         return 2;
//     }

//     freeaddrinfo(servinfo);

//     printf("listener: waiting to recvfrom...\n");
//     cout<< "The serverB is up and running using UDP on port " << SERVER_PORT << "." << endl;

//     addr_len = sizeof their_addr;


//     while(1){
//         if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
//             (struct sockaddr *)&their_addr, &addr_len)) == -1) {
//             perror("recvfrom");
//             exit(1);
//         }
//         buf[numbytes] = '\0';

//         cout << "in server.cpp: " << buf << endl;
//         // here get relevant data based on the operation.

//         // if buf == GET_SERIAL_NUM
//         char get_serial_num_str[] = "GET_SERIAL_NUM";
//         char txlist_str[] = "TXLIST";
//         char first_2_chars[5]; //  the first chars of the buf
//         memcpy(first_2_chars, buf, 2); // only first twoc chars
//         char write_str[] = "W "; // if need to write, buf should starts with "W "

//         if (strcmp(buf, get_serial_num_str) == 0){ //buf == GET_SERIAL_NUM
//             // traverse all transaction records, and return the biggest one as a string.
//             char *res_str = "0"; 
//             int res_str_len = 0;
//             int biggest_serial_num = 0;
//             int serial_num;
//             string one_line;
//             ifstream infile;
//             infile.open (INPUT_FILE);
//             while(!infile.eof()) // To get all lines.
//             {
//                 getline(infile, one_line); // Saves the line in one_line.
//                 // split the string
//                 char *ptr; // declare a ptr pointer  
//                 char one_line_char[one_line.size()+2]; 
//                 strcpy(one_line_char, one_line.c_str());
//                 ptr = strtok(one_line_char, " ");
//                 cout << "in serverA, if GET_SERIAL_NUM: ptr=" << ptr << " should be a number str" << endl;
//                 serial_num = atoi(ptr);
//                 // if (strcmp(res_str, ptr) < 0){ // ptr is bigger than the current biggest
//                 //     cout << " in if (strcmp(res_str, ptr) < 0" << endl;
//                 //     // strcpy(res_str, ptr);
//                 //     res_str = ptr;
//                 //     res_str_len = strlen(ptr);
//                 //     cout << "after strcpy" << endl;
//                 // }
//                 // cout << "res_str: " << res_str <<endl;

//                 if (serial_num > biggest_serial_num){
//                     biggest_serial_num = serial_num;

//                 }


//             }
//             infile.close();

//             // char *biggest_serial_num_str = itoa(biggest_serial_num);
//             string biggest_serial_num_str = to_string(biggest_serial_num);


//             cout << "strlen(res_str): " << strlen(res_str) << endl;
//             cout << "res_str_len: " << res_str_len << endl;
//             // char
//             // res_str_len
//             // now the res_str is the biggest serial number on this server.
//             if ((numbytes = sendto(sockfd, biggest_serial_num_str.c_str(), biggest_serial_num_str.size(), 0,
//                 (struct sockaddr *)&their_addr, addr_len)) == -1) {
//                 perror("backend server response sendto");
//                 exit(1);
//             }
               
//         }

//         // returns all records as one string.
//         else if(strcmp(buf, txlist_str) == 0) //buf == TXLIST)
//         {
//             string res_str = "";
//             // get all data from the txt file.
//             string one_line;
//             ifstream infile;
//             infile.open (INPUT_FILE);
//             while(!infile.eof()) // To get all lines.
//             {
//                 getline(infile, one_line); // Saves the line in one_line.
//                 string original_line = one_line;
//                 res_str = res_str + original_line + ";";
//             }
//             infile.close();
//             // system ("pause");

//             cout << "serverA: return all records as one string, res_str: " << res_str <<endl;

//             // send the result back to the main server.
//             // string res = "hello from server.cpp";
//             if ((numbytes = sendto(sockfd, res_str.c_str(), res_str.length(), 0,
//                 (struct sockaddr *)&their_addr, addr_len)) == -1) {
//                 perror("backend server response sendto");
//                 exit(1);
//             }
//         }


//         // if write, first 2 chars are "W ", then comes the log string.
//         else if(strcmp(first_2_chars, write_str) == 0){ // write one new log to the txt file
//             ofstream outfile;
//             outfile.open(INPUT_FILE, ofstream::app); // append to file, do not overwrite
//             char *first_2_chars_chopped = buf + 2;
//             outfile << first_2_chars_chopped;
//             outfile.close();

//             // reply sucessfully write
//             string res_str = "SUCESSFUL";
//             if ((numbytes = sendto(sockfd, res_str.c_str(), res_str.length(), 0,
//                 (struct sockaddr *)&their_addr, addr_len)) == -1) {
//                 perror("backend server response: write successfully");
//                 exit(1);
//             }

//         }


//         else{ // if buf is just a username, return all records contains this username on this server.
//             string res_str = "";
//             int return_amount = 0;
//             // get all data from the txt file.
//             string one_line;
//             ifstream infile;
//             infile.open (INPUT_FILE);
//             bool found_user = false;
//             while(!infile.eof()) // To get all lines.
//             {
//                 getline(infile, one_line); // Saves the line in one_line.
//                 string original_line = one_line;
//                 // split the string
//                 char *ptr; // declare a ptr pointer  
//                 char one_line_char[one_line.size()+2]; 
//                 strcpy(one_line_char, one_line.c_str());
//                 ptr = strtok(one_line_char, " "); // separate string by space delimiter.   
//                 // use while loop to check ptr is not null  
//                 int num_args = 0;
//                 char * msg_args[4];
//                 while (ptr != NULL)  
//                 {  
//                     // cout << ptr  << endl; // print the string token 
//                     msg_args[num_args] = ptr; 
//                     ptr = strtok (NULL, " ");  
//                     num_args++;
//                     // if the username we want to find is in this transaction record, 
//                     // add the whole record string to the res_str.
//                     // if (num_args == 1 || num_args == 2){
//                     //     if (strcmp(ptr, buf) == 0){ // if they are equal
//                     //         res_str = res_str + original_line + "; ";
//                     //         break;
//                     //     }
//                     // }
//                 }
//                 if (strcmp(msg_args[1], buf) == 0){ // as sender
//                     return_amount = return_amount - atoi(msg_args[3]);
//                     found_user = true;
//                 }
//                 else if (strcmp(msg_args[2], buf) == 0){ // as receiver
//                     return_amount = return_amount + atoi(msg_args[3]);
//                     found_user = true;
//                 }
//             }
//             infile.close();
//             // system ("pause");

//             // cout << "serverA: records with the username -> res_str: " << res_str <<endl;
            

//             // send the result back to the main server.
//             // string res = "hello from server.cpp";
//             // if ((numbytes = sendto(sockfd, res_str.c_str(), res_str.length(), 0,
//             //     (struct sockaddr *)&their_addr, addr_len)) == -1) {
//             //     perror("backend server response sendto");
//             //     exit(1);
//             // }


//             // send back the amount as a string
//             // if returned_amount is a negative number, to_string will add a "-".
//             string return_s;
//             if (found_user){
//                 return_s = to_string(return_amount);
//             }
//             else{ // did not find user. return + to let serverM know that this backend server did not find the username.
//                 return_s = "+";
//             }
//             if ((numbytes = sendto(sockfd, return_s.c_str(), return_s.length(), 0,
//                     (struct sockaddr *)&their_addr, addr_len)) == -1) {
//                     perror("backend server response sendto");
//                     exit(1);
//                 }
//         }

        

//     }
    

//     close(sockfd);

//     return 0;
// }