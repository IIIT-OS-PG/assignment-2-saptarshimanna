#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <iostream>
#include <bits/stdc++.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <fstream>
#include <functional>
#include <boost/algorithm/string.hpp>

#define MAX_CONN 1000
typedef struct service_req_dt srv_rspnd;
using namespace std;

struct service_req_dt {
    int portNumber,no_of_chunks;
    string ip,file_path,hash,avail_list;
};

void print_error(const char *msg)
{
    perror(msg);
    exit(0);
}
void serve_request(int portno){
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[512];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	srv_rspnd response;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		print_error("error opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			 sizeof(serv_addr)) < 0)
		print_error("error on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,
					   (struct sockaddr *) &cli_addr,
					   &clilen);
	if (newsockfd < 0)
		print_error("error on accept");
	close(newsockfd);
	close(sockfd);
}
void* create_user(void* info){
	vector<string> info_arg=*((vector<string> *)info); //o uid, 1 pwd
	string uid=info_arg[1]; uid=info_arg[0];
	string pwd=info_arg[0]; pwd=info_arg[1];
	string ip=info_arg[2]; string port=info_arg[3];
	FILE* f=fopen("./.user_inf.txt","a");
	//char buffer[1024];
	string result=uid+" "+pwd+" "+ip+" "+port+'\n';
	fwrite(result.c_str(), sizeof(char), result.length(), f);
	fclose(f);
	cout<<"user creation successful"<<'\n';
	pthread_exit(0);
}

void* upload_file(void* info){
	vector<string> info_arg=*((vector<string> *)info);
	string uid=info_arg[2];	string file_name=info_arg[0], file_path=info_arg[1];
	string ip=info_arg[3]; string port=info_arg[4], hash=info_arg[5], availability=info_arg[6];
	FILE* f=fopen("./.file_share_inf.txt","a");
	//char buffer[1024];
	string result=uid+" "+ip+" "+port+" "+file_name+" "+file_path+" "+hash+" "+availability+'\n';
	fwrite(result.c_str(), sizeof(char), result.length(), f);
	fclose(f);
	cout<<"file uploaded successfully"<<'\n';
	pthread_exit(0);
}

void* download_file(void* info){
	vector<string> info_arg=*((vector<string> *)info);
	string uid=info_arg[2];	string file_name=info_arg[0], file_path=info_arg[1];
	string ip=info_arg[3]; string port=info_arg[4], hash=info_arg[5], availability=info_arg[6];
	FILE* f=fopen("./.file_share_inf.txt","a");
	//char buffer[1024];
	string result=uid+" "+ip+" "+port+" "+file_name+" "+file_path+" "+hash+" "+availability+'\n';
	fwrite(result.c_str(), sizeof(char), result.length(), f);
	fclose(f);
	cout<<"file info sent to client successfully"<<'\n';
	pthread_exit(0);
}

#endif
