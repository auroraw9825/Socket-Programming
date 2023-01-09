### How to run:

`make all` to compile code

```
./serverM
./serverA
./serverB
./serverC
./clientA xxx
./clientB xxx
```


Above order matters, servers need to be run before clients.


### What do my files do:

clientA.cpp/clientB.cpp:
Both two clients do the same thing. When run ./clientX with differnet operations, they will send a TCP request the main server. And when they get the results back from the main server, they will display in their own terminal window.

serverA.cpp/serverB.cpp/serverC.cpp
All these three files do the same thing, except that they have different port number, and they read and write to different file.
When they receive a request from the main server, they first check what the request is.
1. If it is TXLIST, they will send back all their logs in teh block.txt to the main server.
2. If the request is to get the biggest serial number, then the server will go all logs in the file, and send back the biggest number this server has.
3. If the first two chars of the request string is "W ", the server will write the string[3:] to its block file.
4. If it is a username, then they will check if the username is in their corresponding txt file. 
   If not, they will send back "+" to the main server, to let the main server know that they did not find it.
   If found, then the server will calcualte the balance on this txt file, and send back a positive or negative number string.

serverM.cpp
serverM listens to both clientA and clientB, but it does the same thing for the requests from both clientA and clientB.
When it gets a request, it will first parse it, to see what kind of message serverM need to send to the backend servers.
If after parse the TCP request string, and it gets 2 params.
1. If it is a TXLIST request, then it will send "TXLIST" to all backend servers though UDP, and after receive results, it will parse it too logs, and store them into a map which is ascending based on the key, which the key is the serial number. And then it will write it to the alichain.txt.
2. If it is a username, then it means the client wants to check his/her balance. So it will pass the username to all backend servers. If all backend servers reply with "+", then it means this uername is not in our system. Otherwise, it will parse the result, and get the integer balance, adn sum up across all backend servers. Then reply this value though TCP back to the client.
If after parse the TCP request string, and it gets 4 params, then it measn the client want to make a tranfer.
So first, the main server will send the sender name to all backend servers, to check if this person exists.
Then, it sill also check if the receiver name is in the systme, using the same method.
If one of them or both did not found, then reply the client, cna not trasnfer and say the username(s) did not found.
If both usernames exist in our system, then check if the sender has more coins than the values he/she wants to trasnfer.
If not enought, then also reply the client that can not transfer because of the insufficient amount.
If enough, then make the transfer, also need to choose a randome backend server, and send a UCP request to write this new log to this randomly selected backend server.
After all is done, the main server will reply the client the residual amount of coins she/he has.

tcpclient.cpp/tcpserver.cpp/udplisten.cpp/udpsend.cpp
These four files are all referenced from Beej's Book, in section 6, samples codes about TCP and UDP.
In my code, I only called udpsend.cpp, not the other three files. I implemented the other three files in to the server and client code.
But I can not delete these three files, because on my vitual machine, it gave me compiler errors after I deleted them.


The format for all the TCP and UDP are through strings.
what ever the user type in with ./clientX xxx, it will use a space seperated string, and send this string to the main server.
The main server will reply with a string, that need to be displayed on the client terminal window. Mostly the on-screen messages in the project desciption.

Between main server and all backend servers.
There are four types of messages.
1. GET_SERIAL_NUM: the main server wants to get the current biggest serial num, and the backend server reply with its biggest serial num as a string.
2. TXLIST: the main server wants all logs. The backend server concat all the logs using ";" and send back to the main server.
3. first 2 chars are "W ": the main server wants to write to the block.txt file. If successful, reply the main server "SUCCESSFUL".
4. otherwise: it will be treated as a username, and the backend server will reply with balance if this user exists. Otherwise, "+".


As mentioned on piazza post @429, the block.txt files should have a \n char but won't have blank lines. If there are empty lines, may cause some reading problems.

I think if the test cases are super big, for example, the username is too long, or the amount of coins is beyound what C++ can handle, then it will fail. 


I have only done part of the stats operation.

In my client code files, I reference from Beej's book on TCP.
In my backend servers code files, I reference from Beej's book on UCP.
In my serverM.cpp, I reference both TCP server and UDP client from Beej's book.
In my udpsender.cpp, I reference from Beej's book on UCP client.
I only referenced code involving TCP and UDP, all the data process, calculation, etc are write by my self. 
