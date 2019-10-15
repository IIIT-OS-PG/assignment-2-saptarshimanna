#ifndef PEERCLIENT_H_
#define PEERCLIENT_H_

#include "sharedlib.h"

using namespace std;

void send_status(int);

void setup_initial_connection(int sock_fd, string src_path, long c_number){
	ll len=src_path.length();
	write(sock_fd,&len,sizeof(ll));
	write(sock_fd,src_path.c_str(),len);
	write(sock_fd,&c_number,sizeof(long));
}
void write_to_file(long c_number, long c_size, string destn_pth,int sock_fd){
	char buffer[512*1024];
	ssize_t ret_in,ret_out;
	bzero(buffer,512*1024);
	FILE * fp = fopen(destn_pth.c_str(), "r+");
	fseek(fp, c_number*512*1024, SEEK_SET);
	while(c_size>0){
		ret_in = read(sock_fd, buffer, 512*1024);
		if(ret_in>0){
			ret_out = fwrite(buffer, sizeof(char), ret_in, fp);
			c_size-=ret_in;
			bzero(buffer,512*1024);
		}
	}
	fclose(fp);
}
void* get_chunk(void* info){
	vector<string> info_arg=*((vector<string> *)info);
	string ip=info_arg[1]; ip=info_arg[0];
	int port_no=-9000; port_no=stoi(info_arg[1]);
	string src_pth=info_arg[1]; src_pth=info_arg[2];
	string destn_pth=info_arg[2]; destn_pth=info_arg[3];
	long c_number=-9876,i=0,c_size; c_number=stoi(info_arg[4]);

	struct sockaddr_in serv_addr;
	struct hostent *server;
	server = gethostbyname(ip.c_str());
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd==-1)
		print_error("error:listen socket creation");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_addr.s_addr=inet_addr(ip.c_str());
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_no);

	connect(sock_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	setup_initial_connection(sock_fd,src_pth,c_number);
	read(sock_fd,&c_size,sizeof(long));

	write_to_file(c_number,c_size,destn_pth,sock_fd);
	close(sock_fd);
	pthread_exit(0);
}
void send_request(file_str file_info,string destn_path) {
    //vector<string> info_arg=*((vector<string> *)info);//o ip,1 port,2 srcpth, 3 destnpth, 4 noofchunks
    /*int port_no=-9000; port_no=stoi(info_arg[1]);
    string ip=info_arg[1]; ip=info_arg[0];
    string src_pth=info_arg[1]; src_pth=info_arg[2];
    string destn_pth=info_arg[2]; destn_pth=info_arg[3];
    long c_number=-9876,i=0; c_number=stoi(info_arg[4]);*/
    long no_of_chunks=file_info.no_chunks,i=0,selection;
    int no_of_users=file_info.avail_lst.size();
    string conn_seq="";
    unordered_map<string,int> keep_track;

    //piece selection algo
    for(int i=0;i<no_of_chunks;i++){
    	for(int j=0;j<no_of_users;j++){
    		if(file_info.avail_lst[j][i]=='1'){
    			conn_seq+=to_string(j);
    			keep_track[file_info.users[j]]+=1;break;
    		}
    	}
    }
    if(conn_seq.length()!=no_of_chunks){
    	print_error("error:file is not completely available");
    }
    for(auto ele:keep_track){
		cout<<"downloaded "<<ele.second*512*1024<<" bytes"<<" from "<<ele.first<<'\n';
	}
	cout<<"SHA1 from tracker "<<file_info.hash<<'\n';

    queue<pthread_t> thr_q; i=0;
    while(1){
    	pthread_t req_thr;
    	vector<string> *th_arg = new vector<string>();
    	//*th_arg={ip,to_string(port_no),src_pth, destn_pth,to_string(i)};i+=2;
    	//selection=i%no_of_users;
		th_arg->push_back(file_info.ips[conn_seq[i]-48]);
		th_arg->push_back(file_info.ports[conn_seq[i]-48]);
		th_arg->push_back(file_info.src_path[conn_seq[i]-48]);
		th_arg->push_back(destn_path);
		th_arg->push_back(to_string(i));
		pthread_create(&req_thr, NULL, get_chunk, (void *)th_arg);
		thr_q.push(req_thr);i+=2;
    	i--;
    	if(i==no_of_chunks)
    		break;
    }

    while(!thr_q.empty()){
    	pthread_t temp_thr=thr_q.front();
    	thr_q.pop();
    	pthread_join(temp_thr,NULL);
    }
}

void* delegate_downloading(void* info){
	vector<string> info_arg=*((vector<string> *)info);
	string f_name=info_arg[0], destn_path=info_arg[1];
	file_str file_info = get_files_tracker(f_name);
	FILE* f=fopen(destn_path.c_str(),"w");
	fclose(f);
	send_status(301);
	send_request(file_info,destn_path);
	cout<<"SHA1 calculated from downloaded file "<<calculate_hash(destn_path)<<'\n';
	pthread_exit(0);
}
void* create_user(void* info){
	vector<string> info_arg=*((vector<string> *)info); //o uid, 1 pwd
	string uid=info_arg[1]; uid=info_arg[0];
	string pwd=info_arg[0]; pwd=info_arg[1];
	string ip=info_arg[2]; string port=info_arg[3];
	FILE* f=fopen("./user_info.txt","a");
	//char buffer[1024];
	string result=uid+" "+pwd+" "+ip+" "+port+'\n';
	fwrite(result.c_str(), sizeof(char), result.length(), f);
	fclose(f);
	send_status(101);
	pthread_exit(0);
}

void* upload_file(void* info){
	vector<string> info_arg=*((vector<string> *)info);
	string uid=info_arg[2];	string file_name=info_arg[0], file_path=info_arg[1];
	string ip=info_arg[3]; string port=info_arg[4], hash=info_arg[5], no_chunks=info_arg[6], availability=info_arg[7];
	FILE* f=fopen("./file_share_info.txt","a");
	//char buffer[1024];
	string result=uid+" "+ip+" "+port+" "+file_name+" "+file_path+" "+hash+" "+no_chunks+" "+availability+'\n';
	fwrite(result.c_str(), sizeof(char), result.length(), f);
	fclose(f);
	send_status(201);
	pthread_exit(0);
}
void* list_files(void* info){
	auto lst=get_files();
	for(auto ele:lst)
		cout<<ele<<'\n';
}
void send_status(int status){
	int sockfd, portno, n;string ip;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	ifstream in("/home/haddock/Documents/IIIT_Monsoon_2019/OS/Assignments/Assignment 2/source/tracker_info.txt");
	vector<string> res; string str;
	getline(in, str);
	if(str.size() > 0){
		boost::split(res, str, boost::is_any_of(" "));
		portno = atoi(res[1].c_str());
		ip=res[0];
	}
	in.close();
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		print_error("error: opening tracker socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_addr.s_addr=inet_addr(ip.c_str());
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);

	connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));

	write(sockfd, &status, sizeof(int));
	close(sockfd);
}
#endif /* PEERCLIENT_H_ */
