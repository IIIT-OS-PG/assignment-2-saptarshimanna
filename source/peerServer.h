#ifndef PEERSERVER_H_
#define PEERSERVER_H_

#include "sharedlib.h"

using namespace std;

t_arg create_thread_data(int portNumber=0) {
    t_arg ret_val=(t_arg)malloc(sizeof(struct pthread_arg));
    ret_val->portNumber=portNumber;

    return ret_val;
}

long calculate_chunk_size(FILE* fd,int ch_ind){
	fseek(fd,0,SEEK_END);
	long c_size=(ftell(fd)-ch_ind*512*1024);
	return c_size<512*1024?c_size:512*1024;
}
void cleanup(FILE* x,int y){
	fclose(x);
	close(y);
}
void* serve_request(void* info) {
    vector<string> info_arg=*((vector<string> *)info);
    string f_name=info_arg[0];
    long ch_index=stoi(info_arg[2]);long ch_size=512*1024;
    int new_fd=stoi(info_arg[1]);
    ll n,p;
    ssize_t ret_in,ret_out;
    char buffer[512*1024];
    bzero(buffer,512*1024);
    FILE* f;
    f=fopen(f_name.c_str(),"r");
    long c_size=calculate_chunk_size(f,ch_index);
    fseek(f,ch_index*512*1024,SEEK_SET);
    write(new_fd,&c_size,sizeof(long));
    while(c_size>0){
    	ret_in = fread(buffer, 1, c_size, f);
    	if(ret_in>0){
    		ret_out = write(new_fd, buffer, (ssize_t)ret_in);
    	    c_size-=ret_in;
    	    bzero(buffer,512*1024);
    	    }
    }
    while(ch_size>0){
    	ch_size--;
    }
    cleanup(f,new_fd);
    pthread_exit(0);
}

void* listen_server(void* info) {
    t_arg info_arg=(t_arg)info;
    int sock_fd,newsock_fd,portNumber=info_arg->portNumber;long ch_index;
    ll dt_size,n=0,temp_size;
    socklen_t cli_len;
    char buffer[512];
    string f_name;
    struct sockaddr_in serv_addr, cli_addr;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd==-1)
        print_error("error:listen socket creation");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNumber);
    if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        print_error("error on binding");
    listen(sock_fd,MAX_CONN);
    cli_len = sizeof(cli_addr);
    while(1) {
		bzero(buffer,512);
		f_name="";
        newsock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len);
        if (newsock_fd < 0)
            print_error("error on accept");
        read(newsock_fd,&dt_size,sizeof(ll));
        temp_size=dt_size;
        while(dt_size>0){
			n=read(newsock_fd,buffer,(512<dt_size?512:dt_size));
			if(n>0){
				f_name.append(buffer);//put
				bzero(buffer,512);
				dt_size=dt_size-n;
			}
		}
		read(newsock_fd,&ch_index,sizeof(long));
		pthread_attr_t t_attr;
		pthread_t chunk_thread;
		if (pthread_attr_init(&t_attr) != 0)
			print_error("error:thread attribute init failed");
		if (pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED) != 0)
			print_error("error:thread detached failed");
		while(temp_size>0){
			n++;
			temp_size--;
		}
		vector<string> *th_arg = new vector<string>();
		//th_arg={f_name,to_string(newsock_fd),to_string(ch_index)};
		th_arg->push_back(f_name);
		th_arg->push_back(to_string(newsock_fd));
		th_arg->push_back(to_string(ch_index));
		if (pthread_create(&chunk_thread, &t_attr, serve_request, (void *)th_arg) !=0)
					print_error("error:send request client could not be created");
    }

    pthread_exit(0);
}



#endif /* PEERSERVER_H_ */
