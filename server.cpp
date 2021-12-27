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

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }

string space(string);
void getvalues(); 
string getMsg(string ); 

int PORT;
bool users=false;
string path;
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

   getvalues();

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
                users=true;
                if ( bytes_read < 0 ) {
                    printf("Send failed\n");
                }
            break;
        }
        else{
            char m[]="400 user not found. please try another user\n";
              bytes_read=send(client, m, sizeof(m), 0);
                if ( bytes_read < 0 ) {
                    printf("Send failed\n");
                }
            break;
        }

            
        memset(line,0,sizeof(line));
	}
	fclose(fp);
    
    }

if(users){
    
    if(ls=="list"){
        //list files
        char msg[]="list of all files\n";
        //
        //reading all files in directory
 //       char path[]="./files";

        char p[path.size() + 1];
        strcpy(p, path.c_str());
        
        DIR *dir;
        dir=opendir(p);
        struct dirent *enter;
       
        

        if(!dir){
            cout<<"directry not found";
            return;
        }
        string content="";
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
                long int ansz= ftell(fp);
                fclose(fp);

                stringstream nm;
                nm<<ansz;
                string c;
                nm>>c;

                content+=string(fname)+" "+string(c)+" Bytes\n";
                
            }
        }
                char msgz[content.size() + 1];
                strcpy(msgz, content.c_str());
                bytes_read=send(client, msgz, sizeof(msgz), 0);
                        if ( bytes_read < 0 ) {
                                printf("Send failed\n");
                                
                        }
        
       

    }
      else if(arr[0]=="get"){
        //list files
//        string path="./files";
        


        std::string f=string(path)+'/'+string(arr[1]);
        string s=space(f);
        char cstr[s.size() + 1];
        strcpy(cstr, s.c_str());

        ifstream myfile(cstr);
        string l;
        //myfile.open(cstr);
        if(!myfile){
            cout<<"no file";
        }
        string contect="";
        bool ex=false;
        while (getline(myfile,l))
        {
            contect+=l+"\n";
            ex=true;
        }
        contect+=".\n";
        myfile.close();

        if(ex==true){
        char msg[contect.size() + 1];
        strcpy(msg, contect.c_str());
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }

        }
        else{
            string stm="404 file "+string(arr[1])+" not found\n";
             char msg[stm.size() + 1];
             strcpy(msg, stm.c_str());
            bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }

        }
    

               
           
    }
     else if(arr[0]=="put"){
        //copy file

//        string path="./files";
        


        //std::string f=string(path)+'/'+string(arr[1]);

        std::string f=string(arr[1]);
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
        string content="";
        while (getline(myfile,l))
        {
            content+=l+"\n";
        }
    
        myfile.close();

        //getting the files name from the directory
        vector<string> vn;
        stringstream sst(arr[1]);
        int bytes_read;

    
        while (sst.good())
        {
            
            string substring="";
            getline(sst, substring, '/');
            if(substring=="")
            {
                vn.push_back(str);
            }
            vn.push_back(substring);
        }
        int x = vn.size();
        string fn= vn[x-1];

        cout<<fn<<endl;
        std::string fl=string(path)+'/'+string(fn);

        char cflc[fl.size() + 1];
        strcpy(cflc, fl.c_str());

        //creating file and writing to file
        ofstream newfile;
        newfile.open(fl);
        bool w=false;
        
        if(newfile.is_open()){
            newfile<<content;
            w=true;

            newfile.close();
        }
        else{

        }

        FILE* fz = fopen(cstr, "r");
            if (fz == NULL) {
                cout<< "file doesnt exist \n";
                    return;
                    
                }
            fseek(fz, 0L, SEEK_END);
            long int fsans= ftell(fz);
            fclose(fz);
                


        if(w){
            stringstream nm;
            nm<<fsans;
            string c;
             nm>>c;

            std::string mstr="200 "+string(c)+" Bytes file was retrieved by server and was saved\n";
            char msg[mstr.size() + 1];
            strcpy(msg, mstr.c_str());
            bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
        }
        else{

            std::string mstr="400 File cannot save on server side.\n";
            char msg[mstr.size() + 1];
            strcpy(msg, mstr.c_str());
            bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }

        }



    }
    else if(arr[0]=="del"){
        //delete file from directory

//         string path="./files";
        


        std::string f=string(path)+'/'+string(arr[1]);
        string s=space(f);
        char cstr[s.size() + 1];
        strcpy(cstr, s.c_str());
        cout<<cstr<<s.size()<<endl;

        int state=remove(cstr);

        if(state==0){

            
            std::string mstr="200 "+string(arr[1])+" deleted\n";
            char msg[mstr.size() + 1];
            strcpy(msg, mstr.c_str());

            bytes_read=send(client, msg, sizeof(msg), 0);
                    if ( bytes_read < 0 ) {
                            printf("Send failed\n");
                            
                    }
        }
        else{
            std::string mstr="404 "+string(arr[1])+" is not on the server\n";
            char msg[mstr.size() + 1];
            strcpy(msg, mstr.c_str());

            bytes_read=send(client, msg, sizeof(msg), 0);
                    if ( bytes_read < 0 ) {
                            printf("Send failed\n");
                            
                    }
        }


    }
    else if(arr[0]=="quit"){

        //close client connection to server
        char msg[]="good bye!\n";
        users=false;
        bytes_read=send(client, msg, sizeof(msg), 0);

                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
        close(client);
    }

    else{
          char msg[]="command does not exit\n";
        bytes_read=send(client, msg, sizeof(msg), 0);
                if ( bytes_read < 0 ) {
                        printf("Send failed\n");
                        
                }
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

void getvalues(){
    
    string str;
    getline(cin,str);
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

    bool pas=true;
    do{
        
        string password;
        stringstream pass(arr[3]);
        pass>>password;

        stringstream getInt(arr[2]);
        stringstream getFile(arr[1]);

        string fPath;
        getInt>>PORT; 
        getFile>>path;

        char f[path.size() + 1];
        strcpy(f, path.c_str());

        DIR* dir = opendir(f);
        if (dir)
        {

            cout<<"directry exist"<<endl;
            closedir(dir);
            pas=false;
        }
        else
        {
            cout<<"directry does not exit"<<endl;
            exit(0);
        }
    }while(pas);
    
}
