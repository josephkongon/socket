#include <stdlib.h>
#include <fstream>
#include <sstream> 
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
#include <filesystem>
#include <dirent.h>
#include <sys/stat.h>



/* Definations */
using namespace std;

#define DEFAULT_BUFLEN 1024
#define PORT 1888


void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

string space(string);
string* getvalues(string); 
string getMsg(string ); 

void getReply(string,int,string);
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
    bool firstBoot = 1;

    do
    {
        //send clien the first message
        if(firstBoot){
            char snd[]="Welcome to Bob's file server\n";
            bytes_read=send(client, snd, sizeof(snd), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        break;
                }
            firstBoot=0;
        }

        bytes_read = recv(client, line, sizeof(line), 0);
        cout<<line;
        if (bytes_read > 0) {
                

                string factor =getMsg(line);

                
                getReply(factor,client,line); // returns msg to send to use based on  user input
               
                
                memset(line,0,sizeof(line));

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



int main(int argc, char *argv[])
{   int sd,opt,optval;
    struct sockaddr_in addr;
    socklen_t length;
    unsigned short port=0;
  
    //string str;
    //getline(cin,str);
    //string* values=getvalues(str);
    

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
string space(string str){
    
    string final="";
    int i=0;
    while (str[i]!='\0')
    {
        if(!isspace(str[i])){
            final+=str[i];

        }
        i++;
    }
   return final;

}

void getReply(string str,int client,string Str1){
  
    //cout<<Str1<<"str";
    vector<string> vt;
    stringstream ss(str);
    int bytes_read;

  
    while (ss.good())
    {
        
        string substring="";
        getline(ss, substring, ',');
        if(substring=="")
        {
            vt.push_back(str);
        }
        vt.push_back(substring);
    }
    int n = vt.size();
    string arr[n];
    for(int i = 0; i<vt.size(); i++)
    {
        
        arr[i]=vt[i];
    }
  
    string ls =space(arr[0]);
    FILE *fp;
    if(arr[0]=="user" || arr[0]=="USER"){
        //user acces
        char msg[]="200 User test grated access\n";
        char line[1024];

        if ((fp = fopen("users.txt", "r")) == NULL)
	        {
	        	cout<<" could not read file !"<<endl;
	        	
			}
        
	    while (fgets(line, 1024, fp) != NULL) {
         //cout<<line<<endl;
        
        vector<string> getUser;
        stringstream ss(line);

        while (ss.good())
        {
            
            string substring="";
            getline(ss, substring, ':');
            getUser.push_back(substring);
        }
        
        string name=space(getUser[0]);
        string password=space(getUser[1]);

        string uName=space(arr[1]);
        string upass=space(arr[2]);

        
        if(uName==name && upass==password)
        {
            bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                    printf("Send failed\n");
                }
            break;
        }
            
        memset(line,0,sizeof(line));
	}
	fclose(fp);
    
    }

    
    else if(ls=="list"){
        //list files
        char msg[]="list of all files\n";
        //
        //reading all files in directory
        char path[]="./files";
        
        DIR *dir;
        dir=opendir(path);
        struct dirent *enter;
       
        

        if(!dir){
            cout<<"directry not found";
            return;
        }

        while ((enter=readdir(dir))!=NULL)
        {
            if(enter->d_name[0] != '.'){
                std::string p =string(path)+'/'+ string(enter->d_name);
                //string sp=space(p);
               
                string fname=string(enter->d_name);

                char cstr[p.size() + 1];
                strcpy(cstr, p.c_str());

                //get file size
                FILE* fp = fopen(cstr, "r");
                if (fp == NULL) {
                    cout<< "file doesnt exist \n";
                    return;
                    
                }
                fseek(fp, 0L, SEEK_END);
                long int ans= ftell(fp);
                fclose(fp);
                cout<<fname<<" "<<ans<<"byets"<<endl;
            }
        }
        
       

        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
    }
      else if(arr[0]=="get"){
        //list files
        string path="./files";
        


        std::string f=string(path)+'/'+string(arr[1]);
        string s=space(f);
        char cstr[s.size() + 1];
        strcpy(cstr, s.c_str());
        cout<<cstr<<s.size()<<endl;

        ifstream myfile(cstr);
        string l;
        //myfile.open(cstr);
        if(!myfile){
            cout<<"no file";
        }
        while (getline(myfile,l))
        {
            cout<<l;
        }
    
        //FILE* fp = fopen(cstr, "r");
        //if (fp == NULL) {
       //     cout<< "file doesnt exist \n";
        //        return;
        //}
            
        myfile.close();
               
           
        char msg[]="read file content\n";
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
    }
     else if(arr[0]=="put"){
        //list files
        char msg[]="file delete\n";
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
    }
    else if(arr[0]=="del"){
        //delete file from directory

          string path="./files";
        


        std::string f=string(path)+'/'+string(arr[1]);
        string s=space(f);
        char cstr[s.size() + 1];
        strcpy(cstr, s.c_str());
        cout<<cstr<<s.size()<<endl;

        int state=remove(cstr);

        if(state==0){
            cout<<"file delete"<<endl;
        }
        else{
            cout<<"ERROR"<<endl;
        }


        char msg[]="file delete\n";
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
    }
    else if(arr[0]=="quit"){

        //close client connection to server
        char msg[]="good bye!\n";
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
        close(client);
    }

    else{
          char msg[]="to e handled\n";
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
    }
    
    


}

string getMsg(string str){
   
    string proStr="";
    string trStr;
    int i=0;
    //str.erase(remove(str.begin(), str.end(), ' '), str.end());
    //cout<<str.length();
    for(i; i<str.length()-1;i++)
    {
       if(str[i]==' ')
        {
            proStr+=",";
            
        }
        else if(str[i]=='\0')
        {
            break;
            
        }
        else
        {
            proStr+=str[i];
        }
    };
  

    return proStr;
}

string* getvalues(string str){
    
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
    }
    for(int i = 0; i<(sizeof(arr)/sizeof(arr[0])); i++)
    {

        cout<<arr[i]<<" "<<endl;
    }
    return arr;
}