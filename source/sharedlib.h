#ifndef SHAREDLIB_H_
#define SHAREDLIB_H_

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

typedef struct pthread_arg* t_arg;
typedef long long ll;
typedef struct file_info_struct file_str;

using namespace std;

struct pthread_arg {
    int portNumber;
};

struct file_info_struct{
	vector<string> users,ips,ports,src_path,avail_lst;
	string hash;long no_chunks;
};

void print_error(const char *msg)
{
    perror(msg);
    exit(0);
}

string calculate_hash(string filepath){
	ssize_t ret_in;
	char buffer[8192];
	int rd_fd= open(filepath.c_str(), O_RDONLY);
	SHA_CTX ctx;
	SHA1_Init(&ctx);

	while((ret_in = read(rd_fd, buffer, 8192)) > 0){
		SHA1_Update(&ctx, buffer, ret_in);
	}
	close(rd_fd);
	unsigned char hash[SHA_DIGEST_LENGTH];
	char result[2*SHA_DIGEST_LENGTH];
	SHA1_Final(hash, &ctx);
	//string str( hash, hash + sizeof hash / sizeof hash[0] );
	for(int i=0, shaIndex=0;i<SHA_DIGEST_LENGTH;i++, shaIndex+=2) {
		sprintf((char *)&result[shaIndex], "%02x", hash[i]);
	}
	return string(result, result + sizeof result / sizeof result[0] );
}

long calculate_number_of_chunks(string filepath){
	FILE* f=fopen(filepath.c_str(),"r");
	fseek(f,0,SEEK_END);
	long c_num=ceil((double)ftell(f)/(512*1024));
	fclose(f);
	return c_num;
}

set<string> get_files(){
	ifstream in("/home/haddock/Documents/IIIT_Monsoon_2019/OS/Assignments/Assignment 2/source/file_share_info.txt");
	vector<string> res;set<string> lst_files; string str;
	while (std::getline(in, str))
	{
		if(str.size() > 0){
			boost::split(res, str, boost::is_any_of(" "));
			lst_files.insert(res[3]);
		}
	}
	in.close();
	return lst_files;
}

file_str get_files_tracker(string f_name){
	ifstream in("/home/haddock/Documents/IIIT_Monsoon_2019/OS/Assignments/Assignment 2/source/file_share_info.txt");
	string str;vector<string> res;
	unordered_map<string,file_str> file_info;
	while (std::getline(in, str))
	{
		res.clear();
		if(str.size() > 0){
			boost::split(res, str, boost::is_any_of(" "));
			if(file_info.find(res[3])==file_info.end()){
				file_str details;
				details.users.push_back(res[0]);
				details.ips.push_back(res[1]);
				details.ports.push_back(res[2]);
				details.src_path.push_back(res[4]);
				details.avail_lst.push_back(res[7]);
				details.hash=res[5]; details.no_chunks=stoi(res[6]);
				file_info[res[3]]=details;
			}
			else{
				auto details=file_info.find(res[3]);
				details->second.users.push_back(res[0]);
				details->second.ips.push_back(res[1]);
				details->second.ports.push_back(res[2]);
				details->second.src_path.push_back(res[4]);
				details->second.avail_lst.push_back(res[7]);
			}
		}
	}
	in.close();
	auto details=file_info.find(f_name);
	if(details==file_info.end()){
		print_error("error:file not found on tracker");
	}
	else
		return details->second;
}
#endif
