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
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <typeinfo>
#include <time.h>
#include <map>
#include <iostream>
#include <sstream> 
#include <vector>  
#include <fstream>
// UDP
#include "udplisten.cpp"
#include "udpsend.cpp"
//TCP
#include "tcpclient.cpp"
#include "tcpserver.cpp"


using namespace std;

#define BACKLOG 10   // how many pending connections queue will hold

#define HOST_ADDR "127.0.0.1"
#define SERVERM_PORT_UDP "24944"
#define SERVERM_PORT_ClientA "25944"
#define SERVERM_PORT_ClientB "26944"
#define SERVERA_PORT "21944"
#define SERVERB_PORT "22944"
#define SERVERC_PORT "23944"

typedef struct { 
    int num; // use atoi to Convert string to integer. itoa to convert integer to string.
    char *sender;
    char *receiver;
    int amount;
} RECORD;


// void sigchld_handler(int s)
// {
//     // waitpid() might overwrite errno, so we save and restore it:
//     int saved_errno = errno;

//     while(waitpid(-1, NULL, WNOHANG) > 0);

//     errno = saved_errno;
// }


// // get sockaddr, IPv4 or IPv6:
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



// int get_coins_amount(char *username, char *record_str){
//     string username_str(username);
//     cout << "record_str: " << record_str <<"end" << endl;
//     char *ptr_record; 
//     int amount = 1000; // starts with 1000 coins
//     ptr_record = strtok(record_str, ";");
//     cout << "ptr_record: " << ptr_record << endl;
//     string msg_args[4];
//     char *space_del = " ";
//     while (ptr_record != NULL) // split the string to records by "; "
//     {  

//         string record_string = ptr_record; // (ptr_record);
//         cout << "record_string: " << record_string <<endl;
//         // char *one_record = record_str.c_str(); 
//         // // strcpy(one_record, ptr_record);
//         // char *ptr; 
//         // ptr = strtok(one_record, " "); 
//         // int num_args = 0;
//         // while (ptr != NULL) // split a record to 4 fields with " "
//         // {  
//         //     msg_args[num_args] = ptr; 
//         //     ptr = strtok (NULL, " ");  
//         //     num_args++;
//         // }
//         // cout << "0: " << msg_args[0]<< "1: " << msg_args[1]<< "2: " << msg_args[2]<< "3: " << msg_args[3]<<endl;
//         // if (strcmp(username, msg_args[1]) == 0){ // if username is the sender in this record
//         //     amount = amount - atoi(msg_args[3]);
//         // }
//         // else if (strcmp(username, msg_args[1]) == 0){ //username is the receiver
//         //     amount = amount - atoi(msg_args[3]);
//         // }


//         string temp = "";
//         for(int i = 0; i < (int)record_string.size(); i++){
//             if(record_string[i] != space_del){
//                 temp += record_string[i];
//             }
//             else{
//                 msg_args[i] = temp;
//                 temp = "";
//             }
//         }   
//         cout << "0: " << msg_args[0]<< "1: " << msg_args[1]<< "2: " << msg_args[2]<< "3: " << msg_args[3]<<endl;
//         if (username_str == msg_args[1]){ // if username is the sender in this record
//             amount = amount - stoi(msg_args[3]); // convert string class ot intger, stoi, not atoi
//         }
//         else if (username_str == msg_args[2]){ //username is the receiver
//             amount = amount - stoi(msg_args[3]);
//         }
//         ptr_record = strtok (NULL, "; "); 
//     }  
//     return amount;
// }

int parse_amount(const char *amount_str){
    int amount = 0;
    char *plus_sign = "+";
    char *minus_sign = "-";
    char first_char = amount_str[0];
    // if amount_str == +, means did not find if.
    if (strcmp(amount_str, plus_sign) == 0){
        amount = 0;
    }
    else if (amount_str[0] == minus_sign[0]){ // (strcmp(amount_str[0], minus_sign) == 0 ){ // if starts with "-", the amount is negative.
        amount = amount - atoi(amount_str+1);
    }
    else{ // positive amount
        amount = amount + atoi(amount_str);
    }
    return amount;
}

int main(void)
{
    // for TCP
    // Reference: Beej's book. https://beej.us/guide/bgnet/html/ 
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char buf[1024];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(HOST_ADDR, SERVERM_PORT_ClientA, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    // cout << "here2222. " <<endl;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    cout<< "The main server is up and running." << endl;
    // printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop for TCP
        int numbytes;
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        // printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            // receive a message string from client
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            // cout << "msg received from client A: " << buf << endl;

            // end !!!!!!!!

            // end phase 1

            // send UDP to 3 servers.
            // reference: https://www.javatpoint.com/how-to-split-strings-in-cpp
            // split the received msg string from client
            char *ptr; // declare a ptr pointer  
            ptr = strtok(buf, " "); // use strtok() function to separate string by space delimiter.  
            // cout << " \n Split string using strtok() function: " << endl;  
            // use while loop to check ptr is not null  
            int num_args = 0;
            char * msg_args[4];
            while (ptr != NULL)  
            {  
                // cout << "ptr: " <<  ptr  << endl; // print the string token 
                msg_args[num_args] = ptr; 
                ptr = strtok (NULL, " ");  // was " , ", don't know why, but if error, can check here.
                num_args++;
            } 
            // cout << "num args: " <<num_args << endl; 
            // base on the num_args, we can use msg_args[0] to msg_args[3]
            // cout << "msg_args: " << msg_args[0] << endl;
            // cout << "msg_args: " << msg_args[2] << endl;
            char txlist_str[] = "TXLIST";
            string response_to_client = "";
            char amount_server[1000];
            string client_over_port;
            string client_str(msg_args[0]);
            // cout << client_str << endl;
            if (client_str == "./clientA"){
                client_over_port = "25944";
            }
            else{
                client_over_port = "26944";
            }
            if (num_args == 2){
                // after send the username msg_args[1] to the backend server
                // we get results from this backend server, and store the result in receive_msg_serverA
           		// cout << "received two args" << endl;

                if (strcmp(msg_args[1], txlist_str) != 0){ // msg_args[1] != TXLIST

                    cout << "The main server received input=" << msg_args[1] << " from the client using TCP over port " << client_over_port << "." << endl;
                    // need to find if the username is in the system
                    // if found, return number of alicoins
                    char *amount_serverA;
                    char *amount_serverB;
                    char *amount_serverC;
                    char *plus_sign = "+";
                    // cout << "checkpoint 1." << endl;
                    amount_serverA = udpsend2(SERVERA_PORT, msg_args[1], amount_server);
                    // cout << "received from server A: !!" << amount_serverA <<"??" << endl;
                    string balance_A(amount_serverA);
                    // cout << "balance_A string: " << balance_A << endl;
                    cout << "The main server sent a request to server A." << endl;
                    cout << "The main server received transactions from server A using UDP over port " << SERVERA_PORT << "." << endl;
                    amount_serverB = udpsend2(SERVERB_PORT, msg_args[1], amount_server);
                    string balance_B(amount_serverB);
                    cout << "The main server sent a request to server B." << endl;
                    cout << "The main server received transactions from server B using UDP over port " << SERVERB_PORT << "." << endl;
                    amount_serverC = udpsend2(SERVERC_PORT, msg_args[1], amount_server);
                    string balance_C(amount_serverC);
                    cout << "The main server sent a request to server C." << endl;
                    cout << "The main server received transactions from server C using UDP over port " << SERVERC_PORT << "." << endl;
                    // if (strcmp(amount_serverA,plus_sign)==0&&strcmp(amount_serverB,plus_sign)==0&&strcmp(amount_serverC,plus_sign)==0){
                    if (balance_A=="+" && balance_B=="+" and balance_C == "+"){
                        // this username is not found in 3 servers.
                        response_to_client = "Username was not found on database.\n";
                    }
                    else{
                        // int total_amount = 1000 + parse_amount(amount_serverA) + parse_amount(amount_serverB) + parse_amount(amount_serverC);
                        int total_amount = 1000 + parse_amount(balance_A.c_str()) + parse_amount(balance_B.c_str()) + parse_amount(balance_C.c_str());
                        string user_name_str = string(msg_args[1]);
                        response_to_client = "The current balance of " + user_name_str + " is: " + to_string(total_amount) + " alicoins.\n";
                    }

                    // send back results
                    if (send(new_fd, response_to_client.c_str(), response_to_client.size(), 0) == -1){
                        perror("send");
                    }
                    close(new_fd);
                    cout << "The main server sent the current balance to client " << msg_args[0][8] <<"." << endl;
                }

                else{ // TXLIST
                    cout << "A TXLIST request has been received" << endl;
                    map<int, string, less<int> > logs_map; // sort the map with ascending key int.
                    string temp_log;
                    string log_serial_str;
                    int log_serial_int;

                    // server A
                    char *all_logs_A = udpsend2(SERVERA_PORT, txlist_str, amount_server);
                    string logs_A(all_logs_A);
                    cout << "The main server sent a request to server A." << endl;
                    stringstream logs_A_stream(logs_A);
                    while (getline(logs_A_stream, temp_log, ';')) {  // temp_log is used for store the token string
                        log_serial_str = temp_log.substr(0, temp_log.find(" "));
                        log_serial_int = stoi(log_serial_str);
                        // temp_log.back() = '\n'; //change the last char from ":" ro "\n"
                        logs_map[log_serial_int] = temp_log + "\n";
                    }  
                    cout << "The main server received transactions from server A using UDP over port " << SERVERA_PORT << "." << endl;
                    // for(auto const & item : logs_map) {
                    //     cout << "{" << item.first  <<"," << item.second << "}\n";
                    // }
                    
                    // server B
                    char *all_logs_B = udpsend2(SERVERB_PORT, txlist_str, amount_server);
                    string logs_B(all_logs_B);
                    // cout << "logs_B: " << logs_B <<endl;
                    cout << "The main server sent a request to server B." << endl;
                    stringstream logs_B_stream(logs_B);
                    while (getline(logs_B_stream, temp_log, ';')) {  // temp_log is used for store the token string
                        // cout << "temp_log: " << temp_log<<endl;
                        // cout << "temp_log 1: " << temp_log<<endl;
                        log_serial_str = temp_log.substr(0, temp_log.find(" "));
                        log_serial_int = stoi(log_serial_str);
                        // temp_log.back() = '\n'; //change the last char from ":" ro "\n"
                        logs_map[log_serial_int] = temp_log + "\n";
                        // cout << "temp_log  2: " << temp_log<<endl;
                    }  
                    cout << "The main server received transactions from server B using UDP over port " << SERVERB_PORT << "." << endl;
                    // for(auto const & item : logs_map) {
                    //     cout << "{" << item.first  <<"," << item.second << "}\n";
                    // }
                    
                    
                    // server C
                    char *all_logs_C = udpsend2(SERVERC_PORT, txlist_str, amount_server);
                    string logs_C(all_logs_C);
                    cout << "The main server sent a request to server C." << endl;
                    stringstream logs_C_stream(logs_C);
                    while (getline(logs_C_stream, temp_log, ';')) {  // temp_log is used for store the token string
                        log_serial_str = temp_log.substr(0, temp_log.find(" "));
                        log_serial_int = stoi(log_serial_str);
                        // temp_log.back() = '\n'; //change the last char from ":" ro "\n"
                        logs_map[log_serial_int] = temp_log + "\n";
                    }  
                    cout << "The main server received transactions from server C using UDP over port " << SERVERC_PORT << "." << endl;
                    
                    // need to generate the alichain.txt file.
                    // first store all in a ascending-key map.
                    string all_logs_str = "";
                    for(auto const & item : logs_map) {
                        // cout << "{" << item.first  <<"," << item.second << "}\n";
                        all_logs_str = all_logs_str + item.second; // item.second alreay has \n at the end.
                    }
                    // overwrite the file, no matter if it exists.
                    ofstream write_to_file;
                    write_to_file.open ("alichain.txt");
                    write_to_file << all_logs_str;
                    write_to_file.close();


                    response_to_client = "The sorted file is up and ready.";
                    if (send(new_fd, response_to_client.c_str(), response_to_client.size(), 0) == -1){
                        perror("send");
                    }
                    cout << "The sorted file is up and ready." << endl;

                } // end of TXLIST
                       
            } // end of num_args == 2


            else if (num_args == 4){
                // get records of both usernames, and if both exsit and can transfer, 
                // get the current biggest serial number
                // cout << "in else if num_args == 4 " << msg_args[1] << endl;
                cout << "The main server received from " << msg_args[1] << " to transfer " << msg_args[3] << " coins to " << msg_args[2] << " using TCP over port " << client_over_port << "." << endl;
                // after send the username msg_args[1] to the backend server
                // we get results from this backend server, and store the result in receive_msg_serverA
                
                // first find the sender.
                bool found_sender = true;
                bool found_receiver = true;
                int sender_amount = 0;
                // no need to consider receiver_amount, as long as receiver is in the network.
                string sender_name_str = string(msg_args[1]);
                string receiver_name_str = string(msg_args[2]);
                char *plus_sign = "+";

                char *amount_serverA = udpsend2(SERVERA_PORT, msg_args[1], amount_server);
                string balance_A(amount_serverA);
                cout << "The main server sent a request to server A." << endl;
                cout << "The main server received the feedback from server A using UDP over port " << SERVERA_PORT << "." << endl;
                char *amount_serverB = udpsend2(SERVERB_PORT, msg_args[1], amount_server);
                string balance_B(amount_serverB);
                cout << "The main server sent a request to server B." << endl;
                cout << "The main server received the feedback from server B using UDP over port " << SERVERB_PORT << "." << endl;
                char *amount_serverC = udpsend2(SERVERC_PORT, msg_args[1], amount_server);
                string balance_C(amount_serverC);
                cout << "The main server sent a request to server C." << endl;
                cout << "The main server received the feedback from server C using UDP over port " << SERVERC_PORT << "." << endl;
                // if (strcmp(amount_serverA,plus_sign)==0&&strcmp(amount_serverB,plus_sign)==0&&strcmp(amount_serverA,plus_sign)==0){
                if (balance_A=="+" && balance_B=="+" and balance_C == "+"){
                    // this username is not found in 3 servers.
                    found_sender = false;
                }
                else{
                    // sender_amount = 1000 + parse_amount(amount_serverA) + parse_amount(amount_serverB) + parse_amount(amount_serverC);
                    sender_amount = 1000 + parse_amount(balance_A.c_str()) + parse_amount(balance_B.c_str()) + parse_amount(balance_C.c_str());
                }

                // find receiver
                char *receiver_serverA = udpsend2(SERVERA_PORT, msg_args[2], amount_server);
                string bal_A(receiver_serverA);
                cout << "The main server sent a request to server A." << endl;
                cout << "The main server received the feedback from server A using UDP over port " << SERVERA_PORT << "." << endl;
                char *receiver_serverB = udpsend2(SERVERB_PORT, msg_args[2], amount_server);
                string bal_B(receiver_serverB);
                cout << "The main server sent a request to server B." << endl;
                cout << "The main server received the feedback from server B using UDP over port " << SERVERB_PORT << "." << endl;
                char *receiver_serverC = udpsend2(SERVERC_PORT, msg_args[2], amount_server);
                string bal_C(receiver_serverC);
                cout << "The main server sent a request to server C." << endl;
                cout << "The main server received the feedback from server C using UDP over port " << SERVERC_PORT << "." << endl;
                // if (strcmp(receiver_serverA,plus_sign)==0&&strcmp(receiver_serverB,plus_sign)==0&&strcmp(receiver_serverA,plus_sign)==0){
                if (bal_A=="+" && bal_B=="+" and bal_C == "+"){
                    // this username is not found in 3 servers.
                    found_receiver = false;
                }

                if (!found_sender && ! found_receiver){
                    response_to_client = "Unable to preceed with the transaction as " + sender_name_str + " and " + receiver_name_str + " are not part of the network.\n";
                }
                else if (!found_sender){
                    response_to_client = "Unable to preceed with the transaction as " + sender_name_str + " is not part of the network.\n";
                }
                else if (!found_receiver){
                    response_to_client = "Unable to preceed with the transaction as " + receiver_name_str + " is not part of the network.\n";
                }
                else{ // both sender and receiver are in the network.
                    if (sender_amount < atoi(msg_args[3])){ // sender insufficient balance
                        response_to_client = sender_name_str + " was unable to transfer " + to_string(atoi(msg_args[3])) + " alicoins to " + receiver_name_str + " because of insufficient balance.\nThe current balance of " + sender_name_str + " is: " + to_string(sender_amount) + " alicoins.\n";
                    }
                    else{ // able to transfter.
                        // get the current serail number.
                        char get_serial_num_str[] = "GET_SERIAL_NUM";
                        char *serial_num_A = udpsend2(SERVERA_PORT, get_serial_num_str, amount_server);
                        int serial_int_A = atoi(serial_num_A);
                        char *serial_num_B = udpsend2(SERVERB_PORT, get_serial_num_str, amount_server);
                        int serial_int_B = atoi(serial_num_B);
                        char *serial_num_C = udpsend2(SERVERC_PORT, get_serial_num_str, amount_server);
                        int serial_int_C = atoi(serial_num_C);
                        int new_serial_num;
                        if (serial_int_A > serial_int_B && serial_int_A > serial_int_C){
                            new_serial_num = serial_int_A + 1;
                        }
                        else if (serial_int_B > serial_int_A && serial_int_B > serial_int_C){
                            new_serial_num = serial_int_B + 1;
                        }
                        else if (serial_int_C > serial_int_A && serial_int_C > serial_int_B){
                            new_serial_num = serial_int_C + 1;
                        }
                        
                        // add the transaction log to any of the block file on backend servers.
                        // generate the new log with new_serial_num.
                        // need to use "W " to tell backend server we want to write
                        // need to add "\n" after the log string, to indicate a new line.
                        string new_log = "W " + to_string(new_serial_num) + " " + sender_name_str + " " + receiver_name_str + " " + to_string(atoi(msg_args[3])) + "\n";
                        // get a random server
                        srand(time(NULL));
                        int r = rand() % 3; // if no set time, here rand() always return 16807, as the first seed.
                        // cout << "rand()="<<r<<endl;
                        // int r = (new_serial_num*new_serial_num) % 3; // random number between 0 and 2.
                        char *write_result; // will return "SUCESSFUL" if sucessully write.
                        // cout << "r=" << r << "  new serial num=" << new_serial_num << endl;
                        if (r == 0){ // write to server A.
                            write_result = udpsend2(SERVERA_PORT, new_log, amount_server);
                        }
                        else if (r == 1){ // write to server B.
                            write_result = udpsend2(SERVERB_PORT, new_log, amount_server);
                        }
                        else if (r == 2){ // write to server C.
                            write_result = udpsend2(SERVERC_PORT, new_log, amount_server);
                        }
                        else{
                            cout << "r=" << r << " in else" << endl;
                        }
                        // if successfully write.
                        response_to_client = sender_name_str + " successfully transferred " + to_string(atoi(msg_args[3])) + " alicoins to " + receiver_name_str + ".\nThe current balance of " + sender_name_str + " is: " + to_string(sender_amount-atoi(msg_args[3])) + " alicoins.\n";
                    }
                }

                // send back results
                if (send(new_fd, response_to_client.c_str(), response_to_client.size(), 0) == -1){
                    perror("send");
                }
                close(new_fd);
                cout << "The main server sent the result of the transaction to client " << msg_args[0][8] <<"." << endl;

            } // end of num_args == 4


            // if (send(new_fd, response_to_client.c_str(), response_to_client.size(), 0) == -1)
            //     perror("send");
            // close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}