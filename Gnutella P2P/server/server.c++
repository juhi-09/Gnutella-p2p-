#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fstream>
#include <ctime>
#include <bits/stdc++.h>
using namespace std;
 
//*****************GLOBAL Variables*******************
int server_fd, new_socket;
struct sockaddr_in ser_address;
struct sockaddr_in cli_address;
char buffer[1024];
ofstream repo_file;
string repo_n;
ofstream logs;

//********************* creation of repo file *************

map <string,vector<string> > repo;
map <string,vector<string> >::iterator m_repo;
map <string,string> current;
pair<string, vector < string > > p1;


//************************** invoking server ************


void sock_creation_server(const char* ip,int port)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    ser_address.sin_family = AF_INET;
    ser_address.sin_port = htons( port );
    inet_pton(AF_INET, ip, &ser_address.sin_addr);
    bind(server_fd, (struct sockaddr *)&ser_address,sizeof(ser_address));
    listen(server_fd, 3);
}

//**************** HANDLE BUFFER *******************
 void server(string buffer,int x)
{

  string s;
  s.clear();
  s=buffer;
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

int n=0;
if(input[0]=="search")
 n=0; 
if(input[0]=="share")
 n=1;
if(input[0]=="del")
 n=2;
if(input[0]=="exec")
n=3;
if(input[0]=="get")
n=4;

switch(n)
{

  case 0: //search
         {
           string file_name=input[1];
           m_repo=repo.find(file_name);
              ofstream search;
              search.open("search.txt");
              int found;
           if(m_repo!=repo.end())
             {
                vector<string>v;
                v=m_repo->second;
                found=v.size();
               if(search.is_open())
                {
                 search<<"FOUND:"<<"["<<found<<"]"<<endl;
                 int i=1;
                 search.close();
               for(vector<string>::iterator fi=m_repo->second.begin();fi!=m_repo->second.end();++fi)
                 {
                  search.open("search.txt",ios::app | ios::out);
                  search<<"["<<i<<"]"<<*fi<<"\n";
                  search.clear();
                  i++;
                  search.close();
                 }
                }
              }
           else 
            {
             if(search.is_open())
             {
             found=0;
             search<<"["<<found<<"]"<<"NO FILE FOUND\n";
             search.clear();
             search.close();
             }
            }
          string s;
          ifstream in;
          in.open("search.txt");
          
          while(!in.eof())
           {
            getline(in,s);
            send(x,s.c_str(),s.length(),0);
           }
           s="finish";
           send(x,s.c_str(),s.length(),0);
         in.close();
        }
     break;
     
case 1: //share
       {
         string file_name,g;
         int q=input[1].length()-1;
         g=input[1];
         while(g[q]!='/')
            q--;

         file_name=g.substr(q+1);
         string s;
         s=input[1]+':'+input[2];

         m_repo=repo.find(file_name);
         if(repo.empty())
          {
           vector < string > v1;
           v1.push_back(s);
           p1 = make_pair(file_name, v1); 
           repo.insert(p1);
          }
       else
        {
          if(m_repo!=repo.end())
           {
            m_repo->second.push_back(s);
           }
          else
            {
              vector < string > v1;
              v1.push_back(s);
              p1 = make_pair(file_name, v1); 
              repo.insert(p1);
            }
          }
        repo_file.open(repo_n.c_str(),ios::out | ios::app);
        repo_file<<file_name<<':'<<s<<endl;
        repo_file.close();
       }
    break;

case 2://delete
        {
          string file_name,g;
          int q=input[1].length()-1;
          g=input[1];
          while(g[q]!='/')
              q--;

          file_name=g.substr(q+1);
          cout<<file_name<<"    "<<input[2]<<endl;

          m_repo=repo.find(file_name);
            if(m_repo!=repo.end())
              {
                 vector < string > v1;
                 v1=m_repo->second;
                 int d=0;
                 for(vector<string>::iterator i=m_repo->second.begin();i!=m_repo->second.end();++i)
                   {
                    string p=*i;
                    string r;
                    int v=p.length();
                    v=v-1;
                     while(p[v]!=':')
                        v--;
                    r=p.substr(v+1);
                   if(!strcmp(r.c_str(),input[2].c_str()))
                     {
                        d=1;
                        v1.erase(i);
                        break;
                     }
                   }
            string r;
            if(d==1)
            r="SUCCESS:FILE_REMOVED";
            else
            r="NO DATA FOUND TO BE DELETED";
            send(x,r.c_str(),r.length(),0);
  
             for(map<string,vector<string> >::iterator it=repo.begin();it!=repo.end();++it)
              {
                for(vector<string>::iterator i=it->second.begin();i!=it->second.end();++i)
                 {
                   repo_file.open(repo_n.c_str(),ios::out);
                   repo_file<<it->first<<':'<<*i<<endl;
                   repo_file.close();
                 }
              }
           }
          }
       break;

case 3://exec
       {
        string h;
        for(map<string,string>::iterator it=current.begin();it!=current.end();++it)
         {
          if(it->first==input[1])
          { 
           h=it->second;
           break;
          }
        }
         send(x,h.c_str(),h.length(),0);
       }
     break;

case 4://get
        {
        string h;
         for(map<string,string>::iterator it=current.begin();it!=current.end();++it)
          {
           if(it->first==input[1])
           { 
             h=it->second;
            break;
           }
          }
         send(x,h.c_str(),h.length(),0);
        }
        break;
  }
}


//**********************THREADS*******************

 void *serv(void* arg)
{
  pthread_detach(pthread_self());
  int x = *((int*)(&arg));
  bzero(buffer,1024);
   while( read(x , buffer, 1024)>0)
     {
      logs.open("log_ser.txt",ios::out |ios::app);
      if(logs.is_open())
      {
      logs<<buffer<<endl;
      time_t now = time(0);
      char*d=ctime(&now);
      logs<<d<<endl;
      }
      logs.close();
       server(buffer,x);
       bzero(buffer,1024);
     }
   close(x);
   pthread_exit(0);
   return NULL;
 }



//******************MAIN   PROGRAM **************************


int main(int argc, char const *argv[])
{
    int addrlen = sizeof(cli_address);
    pthread_t th;
    sock_creation_server(argv[1],atoi(argv[2]));
    repo_n=argv[3];

    repo_file.open(repo_n.c_str());
    repo_file<<"CREATION OF REPO FILE"<<endl;
    repo_file.close();

    ofstream client_list(argv[4]);
    client_list<<" CREATION OF CURRENT CLIENT FILE"<<endl;
    client_list<<"ALIAS:CLIENT_IP:CLIENT_PORT:CLIENT_DOWNLOAD_PORT"<<endl;
    client_list.close();

    while(1)
     {
       new_socket = accept(server_fd, (struct sockaddr *)&cli_address,(socklen_t*)&addrlen);
       bzero(buffer,1024);
       read(new_socket,buffer,1024);

       //**************** MAKE CLIENT ENTRY IN SERVER ********************

       string buf=buffer;
       int i=0;
       while(buf[i]!=':')
         i++;

      pair<string,string> p;
      p=make_pair(buf.substr(0,i),buf.substr(i+1));
      current.insert(p);
      client_list.open(argv[4], ios::app);
        if(client_list.good())
           {
             client_list<<buf<< '\n';
             client_list.close();
           }
      pthread_create(&th,NULL,&serv,(void*)new_socket);

     }
   close(new_socket);
   return 0;
}
