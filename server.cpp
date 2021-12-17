#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
/* Definations */
using namespace std;
#define DEFAULT_BUFLEN 1024
#define PORT 1888

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

string* getvalues(); 
char* getReply(string);
/*--------------------------------------------------------------------*/
/*--- Child - echo server                                         ---*/

/*--------------------------------------------------------------------*/
int fd;
void setFd(int f){
    fd=f;

}
void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;
    char recvbuf[DEFAULT_BUFLEN],bmsg[DEFAULT_BUFLEN];
    int  recvbuflen = DEFAULT_BUFLEN;

    do
    {
        bytes_read = recv(client, line, sizeof(line), 0);
        cout<<line;
        if (bytes_read > 0) {
                char *replay=getReply(line);
                bytes_read=send(client, replay, sizeof(replay), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        break;
                }
                


                //sending and receiving
           /*     bytes_read = recv(fd, line, sizeof(line), 0);
                if (bytes_read < 0) {
                    printf("Send failed:\n");
                    close(fd);
                    break;
                }
                */




                //end
        } else if (bytes_read == 0 ) {
                printf("Connection closed by client\n");
                break;
        } else {
                printf("Connection has problem\n");
                break;
        }
    } while (bytes_read > 0);
    close(client);
    return arg;
}

/*--------------------------------------------------------------------*/
/*--- main - setup server and await connections (no need to clean  ---*/
/*--- up after terminated children.                                ---*/
/*--------------------------------------------------------------------*/
char* getReply(string){
    
}
int main(int argc, char *argv[])
{   int sd,opt,optval;
    struct sockaddr_in addr;
    socklen_t length;
    unsigned short port=0;
  
    //string* values=getvalues();
    

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
                port=atoi(optarg);
                break;
        }
    }


    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( port > 0 )
                addr.sin_port = htons(port);
    else
                addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("System ready on port %d\n",ntohs(addr.sin_port));

    while (1)
    {
        int client;
        length = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr*)&addr, &length);
       
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if ( pthread_create(&child, NULL, Child, &client) != 0 )
            perror("Thread creation");
        else
            pthread_detach(child);  /* disassociate from parent */
    }
    return 0;
}

string* getvalues(){
    string str;
    getline(cin,str);
    cout<<str<<endl;
    string proStr="";
    string trStr;
    int i=0;
    for(i; i<str.length();i++)
    {
       if(str[i]==' ' && str[i+1]=='-' && (str[i+2]=='d' ||str[i+2]=='p' ||str[i+2]=='u'))
        {
            proStr+=",";
            i=i+3;
        }
        else
        {
            proStr+=str[i];
        }
    }
    cout<<proStr<<endl;
    vector<string> vector;
    stringstream ss(proStr);

    while (ss.good())
    {
        string substring;
        getline(ss, substring, ',');
        vector.push_back(substring);
    }
    int n = vector.size();
    string arr[n];
    for(int i = 0; i<vector.size(); i++)
    {
        arr[i]=vector[i];
        cout<<arr[i]<<" "<<endl;
    }
    return arr;
}