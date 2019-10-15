#include <iostream>
#include "utilities.h"

using namespace std;


int main(int argc, char **argv) {
	int sockfd, newsockfd, portno;
	string uid="u001",pwd="abcd",ip="1.1.1.1",port="8888";
	string f_name="", hash="", src_path="",avalmap="";
	socklen_t clilen;
	char buffer[512];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		print_error("error: opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	ifstream in("/home/haddock/Documents/IIIT_Monsoon_2019/OS/Assignments/Assignment 2/source/tracker_info.txt");
	vector<string> res; string str;
	getline(in, str);
	if(str.size() > 0){
		boost::split(res, str, boost::is_any_of(" "));
		portno = atoi(res[1].c_str());
	}
	in.close();

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		print_error("error: on binding");
	listen(sockfd,MAX_CONN);
	clilen = sizeof(cli_addr);int status;
	while(1){
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			print_error("error: on accept");
		read(newsockfd, &status, sizeof(int));
		pthread_attr_t t_attr;
		pthread_t listen_thread;
		if (pthread_attr_init(&t_attr) != 0)
			print_error("error:thread attribute init failed");
		if (pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED) != 0)
			print_error("error:thread detached failed");
		if(status==101){
			vector<string>* th_arg=new vector<string>();
			th_arg->push_back(uid);
			th_arg->push_back(pwd);
			th_arg->push_back(ip);
			th_arg->push_back(port);
 			if (pthread_create(&listen_thread, &t_attr, create_user, (void *)th_arg) !=0)
					print_error("error:send request client could not be created");
		}
		else if(status==201){
			vector<string>* th_arg=new vector<string>();
			th_arg->push_back(uid);
			th_arg->push_back(f_name);
			th_arg->push_back(src_path);
			th_arg->push_back(hash);
			th_arg->push_back(avalmap);
			th_arg->push_back(ip);
			th_arg->push_back(port);
			if (pthread_create(&listen_thread, &t_attr, upload_file, (void *)th_arg) !=0)
					print_error("error:send request client could not be created");
		}
		else if(status==301){
			vector<string>* th_arg=new vector<string>();
			th_arg->push_back(uid);
			th_arg->push_back(f_name);
			th_arg->push_back(src_path);
			th_arg->push_back(hash);
			th_arg->push_back(avalmap);
			th_arg->push_back(ip);
			th_arg->push_back(port);
			if (pthread_create(&listen_thread, &t_attr, download_file, (void *)th_arg) !=0)
					print_error("error:send request client could not be created");
		}
	}
	return 0;
}
