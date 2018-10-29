#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fstream>
#include <bits/stdc++.h>
using namespace std;


//*********************** GLOBAL VARIABLES ********************

struct sockaddr_in serv_addr,client_addr;
struct sockaddr_in cli_ser_addr; /*client listening to rpc*/
struct sockaddr_in cli_rpc_addr; /*accecpting rpc request..filling in request client*/
struct sockaddr_in cli_addr_rpc; /*client sending rpc request*/
struct sockaddr_in ser_addr_rpc;  /*connectong to server for rpc*/
int sock = 0;
int rpc_sock=0;
int cli_rpc=0; 
char buffer[1024];
int ser_rpc_sock=0;
const char *argv2;
int argv6;
ofstream clogs;
//************************** invoking client *****************

void sock_creation(const char *ser_ip ,int ser_port,const char* cli_ip, int cli_port)
{

    sock=socket(AF_INET, SOCK_STREAM,0);


    memset(&client_addr, '0', sizeof(client_addr));
    client_addr.sin_family = AF_INET;                               //binding client to port and creating socket with ip
    client_addr.sin_port = htons( cli_port );
    inet_pton(AF_INET, cli_ip, &client_addr.sin_addr);
    bind(sock, (struct sockaddr *)&client_addr,sizeof(client_addr));

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(ser_port);                                   //filling in the server details
	inet_pton(AF_INET, ser_ip, &serv_addr.sin_addr);   

	connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));           //call to server

}

//***************** INVOKING THE SERVER IN CLIENT ***************************
//Listening for the rpc and dwnloding of the files**************
void *sock_ser_create(void* a)
{
	pthread_detach(pthread_self());
	cli_rpc=socket(AF_INET, SOCK_STREAM,0);
	memset(&cli_ser_addr,'0',sizeof(cli_ser_addr)); // Clearing the client address


    cli_ser_addr.sin_family = AF_INET;
    cli_ser_addr.sin_port = htons( argv6 );
    inet_pton(AF_INET,argv2, &cli_ser_addr.sin_addr);
    bind(cli_rpc, (struct sockaddr *)&cli_ser_addr,sizeof(cli_ser_addr));
    listen(cli_rpc, 3);
    int ia=1;
    while(ia)
    {
        ser_rpc_sock=accept(cli_rpc, (struct sockaddr *)&cli_rpc_addr,(socklen_t*)&cli_rpc_addr);
        bzero(buffer,1024);
        read(ser_rpc_sock,buffer,1024);
      	if(buffer[0]=='2')
        {
	         string out_file=" | cat > out.txt";
	         string d=buffer;
	         d=d.substr(1);
	         out_file=d+ out_file;
	         system(out_file.c_str());
	         ifstream in;
	         string s;
	         in.open("out.txt");
	         if(in.is_open())
	        {
		        while(!in.eof())
		        {
		         getline(in,s);
		         send(ser_rpc_sock,s.c_str(),1024,0);
		        }
		        s="finish";
		        send(ser_rpc_sock,s.c_str(),1024,0);
		         in.close();
		         ia=0;
	       }
	       else
	          {
	            s="COMMAND NOT EXECUTED finish";
	            send(ser_rpc_sock,s.c_str(),1024,0);
	          }
       }
     else
      {
        string s;
        ifstream in;
        in.open(buffer);
        if(in.is_open())
         {
           while(!in.eof())
            {
              getline(in,s);
              send(ser_rpc_sock,s.c_str(),1024,0);
            }
            s="finish";
        send(ser_rpc_sock,s.c_str(),1024,0);
           in.close();
         }
         else
         {
           s="NO FILE FOUND finish";
           send(ser_rpc_sock,s.c_str(),1024,0);
         }
         ia=0;
      } 
   }
  close(ser_rpc_sock);
  close(cli_rpc);
  pthread_exit(0);
  return NULL;
}

//*************** THE MAIN PROGRAM **************************
int main(int argc, char const *argv[])
{

	pthread_t t;
	argv2=argv[2];
	argv6=atoi(argv[6]);
	 pthread_create(&t,NULL,&sock_ser_create,NULL);

    sock_creation(argv[4],atoi(argv[5]),argv[2],atoi(argv[3]));
    string message;
    message = string(argv[1])+':'+argv[2]+':'+argv[3]+':'+argv[6];
    send(sock,message.c_str(),message.length(),0);


while(1)
{
 string s;
 s.clear();
 getline(cin,s);
 
 
 clogs.open("logcli.txt",ios::out | ios::app);
 if(clogs.is_open())
 {
 clogs<<s<<endl;
 }
 clogs.close();
 
 string input[20];
 int j=0;
 for(int i=0;i<s.length();i++)
  { 
   if(s[i]!=' ' && s[i] !='\t')
     input[j]=input[j]+s[i];
   if(s[i]=='"')
      {
        int k=1;
        while(s[i+k]!='"')
          { 
            k++;
             if(s[i+k]==' ')
               k++;
          }
         input[j]=s.substr(i+1,k-1);
          i=i+k;
      }
    else if(s[i]== ' ' || s[i]=='\t')
        j++;
   }


int n;
if(input[0]=="search")
 n=0;
if(input[0]=="get")
 n=1;
if(input[0]=="share")
 n=2;
if(input[0]=="del")
 n=3;
if(input[0]=="exec")
n=4;


switch(n)
{

case 0: //SEARCH THE MAP FOR FIle
       {
       char b[1024];
        bzero(b,1024);
         send(sock,s.c_str(),s.length(),0);
         int i=1;
         while(i)
         {
         read(sock,b,1024);
         string f=b;
         string p=b;
         f=f.substr(f.length()-6);
         if(f=="finish")
         {
         i=0;
         p=p.substr(0,p.length()-6);
         if(p=="")
         p="UNABLE TO FETCH";
         }
         cout<<p<<endl;
         bzero(b,1024);
         }
         
        }
        break;
       

case 1: //GET FILE FROM CLIENT ALIAS
      {
        bzero(buffer,1024);
        string m;
        m=input[0]+' '+input[1];
        send(sock,m.c_str(),m.length(),0);
        read(sock,buffer,1024);
        string l=buffer;
        string ip,port;
        int z=l.length()-1;
        while(l[z]!=':')
        z--;
        port=l.substr(z+1);
         z=0;
        while(l[z]!=':')
        z++;
        ip=l.substr(0,z);
 
    rpc_sock=socket(AF_INET, SOCK_STREAM,0);
    memset(&cli_addr_rpc, '0', sizeof(cli_addr_rpc));
    cli_addr_rpc.sin_family = AF_INET;                               //binding client to port and creating socket with ip
    cli_addr_rpc.sin_port = htons(6110);
    cli_addr_rpc.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(rpc_sock, (struct sockaddr *)&cli_rpc_addr,sizeof(cli_rpc_addr));
  
    memset(&ser_addr_rpc, '0', sizeof(ser_addr_rpc));
    ser_addr_rpc.sin_family = AF_INET;
    ser_addr_rpc.sin_port = htons(atoi(port.c_str()));                 //filling in the server details
    inet_pton(AF_INET,ip.c_str(),&ser_addr_rpc.sin_addr);   
    
    connect(rpc_sock, (struct sockaddr *)&ser_addr_rpc, sizeof(ser_addr_rpc));         //call to server
 
    if((send(rpc_sock,input[2].c_str(),input[2].length(),0))<0)
    cout<<"FAILURE:"<<endl;
    else
     {
      char b[1024];
      bzero(b,1024);
      cout<<"SUCCESS:"<<endl;
      int i=1;
      while(i)
       {
        read(rpc_sock,b,1024);
        
        string f=b;
        string p=b;
        if(f.length()>=6)
        {
        f=f.substr(f.length()-6);
        if(f=="finish")
         {
          i=0;
          p=p.substr(0,p.length()-6);
         }
         }
         cout<<p<<endl;
        bzero(b,1024);
       }
     }
        close(rpc_sock);
     }
      break;

case 2://MAKE THE ENTRY OF FILE IN MAP ****** share*
       {
         s=s+' '+argv[1];
         cout<<s<<endl;
         if((send(sock,s.c_str(),s.length(),0)<0))
          cout<<"FAILURE:INVALID_ARGUMENTS"<<endl;
         else
          cout<<"SUCCESS:FILE_SHARED"<<endl;
       }
        break;
         

case 3: //ERASE ENTRY FROM THE FILE  
       {
        s=s+' '+argv[1];
        bzero(buffer,0);
        send(sock,s.c_str(),s.length(),0);
        read(sock,buffer,1024);
        cout<<buffer<<endl;
       }
        break;
        
case 4://rpc call 
       {
 
        bzero(buffer,1024);
        string m;
        m=input[0]+' '+input[1];
        send(sock,m.c_str(),m.length(),0);
        read(sock,buffer,1024);

        string l=buffer;
        string ip,port;
        int z=l.length()-1;
           while(l[z]!=':')
              z--;
        port=l.substr(z+1);
             z=0;
          while(l[z]!=':')
             z++;
        ip=l.substr(0,z);
 

    rpc_sock=socket(AF_INET, SOCK_STREAM,0);
    memset(&cli_addr_rpc, '0', sizeof(cli_addr_rpc));
    cli_addr_rpc.sin_family = AF_INET;                               //binding client to port and creating socket with ip
    cli_addr_rpc.sin_port = htons(8110);
    cli_addr_rpc.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(rpc_sock, (struct sockaddr *)&cli_rpc_addr,sizeof(cli_rpc_addr));
  
    memset(&ser_addr_rpc, '0', sizeof(ser_addr_rpc));
    ser_addr_rpc.sin_family = AF_INET;
    ser_addr_rpc.sin_port = htons(atoi(port.c_str()));             //filling in the server details 
                                                            
    inet_pton(AF_INET,ip.c_str(),&ser_addr_rpc.sin_addr);   
    connect(rpc_sock, (struct sockaddr *)&ser_addr_rpc, sizeof(ser_addr_rpc));//call to server
    input[2]="2"+input[2];
 
    if((send(rpc_sock,input[2].c_str(),input[2].length(),0))<0)
       cout<<"FAILURE:"<<endl;
    else
     {
       char b[1024];
       bzero(b,1024);
       cout<<"SUCCESS:"<<endl;
       int i=1;
      while(i)
       {
        read(rpc_sock,b,1024);
        //cout<<b<<"   "<<endl;
        string f=b;
        string p=b;
        if(f.length()>=6)
        {
        f=f.substr(f.length()-6);
        if(f=="finish")
         {
          i=0;
          p=p.substr(0,p.length()-6);
         }
        }
         cout<<p<<endl;
        bzero(b,1024);
       }
     }
      close(rpc_sock);
    }
   break;
 }
}

close(sock);

    return 0;
}
