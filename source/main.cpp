#include <iostream>
#include "peerClient.h"
#include "peerServer.h"

int main(int argc, char **argv)
{
    char buffer[512]; string uid,ip,portno,trck_info;
    ip=argv[2];portno=argv[1];trck_info=argv[3];
    t_arg server_thread_arg=create_thread_data(atoi(argv[1]));
    pthread_attr_t t_attr;
    pthread_t server_thread;
    if (pthread_attr_init(&t_attr) != 0)
		print_error("error:thread attribute init failed");
    if (pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED) != 0)
        print_error("error:thread detached failed");
    if (pthread_create(&server_thread, &t_attr, listen_server, (void *)server_thread_arg) !=0)
		print_error("error:listening server could not be created");
	while(1){
		bzero(buffer,512);
		fgets(buffer,512,stdin);

		pthread_attr_t td_attr;
		pthread_t client_thread;
		if (pthread_attr_init(&td_attr) != 0)
			print_error("error:thread attribute init failed");
		if (pthread_attr_setdetachstate(&td_attr, PTHREAD_CREATE_DETACHED) != 0)
			print_error("error:thread detached failed");

		vector<string>* cli_args=new vector<string>();
		char* token = strtok(buffer, " ");
		char* command=token;
		token = strtok(0, " ");
		while(token != NULL){
			cli_args->push_back(string(token));
			token = strtok(0, " ");
		}
		cli_args->back().pop_back();
		/*if(strncmp("download_file",command,13)==0){
			if (pthread_create(&client_thread, &td_attr, send_request, (void *)cli_args) !=0)
					print_error("error:send request client could not be created");
		}*/
		if(strncmp("create_user",command,11)==0){
			uid="";
			uid+=(*cli_args)[0];
			cli_args->push_back(ip);
			cli_args->push_back(portno);
			if (pthread_create(&client_thread, &td_attr, create_user, (void *)cli_args) !=0)
					print_error("error:user creation failed");
		}
		else if(strncmp("upload_file",command,11)==0){
			long n_o_c;
			cli_args->push_back(uid);
			cli_args->push_back(ip);
			cli_args->push_back(portno);
			cli_args->push_back(calculate_hash((*cli_args)[1]));
			cli_args->push_back(to_string(n_o_c=calculate_number_of_chunks((*cli_args)[1])));
			cli_args->push_back(string(n_o_c,'1'));
			if (pthread_create(&client_thread, &td_attr, upload_file, (void *)cli_args) !=0)
					print_error("error:upload file failed");
		}
		else if(strncmp("download_file",command,13)==0){
			if (pthread_create(&client_thread, &td_attr, delegate_downloading, (void *)cli_args) !=0)
					print_error("error:download file failed");
		}
		else if(strncmp("list_files",command,10)==0){
			if (pthread_create(&client_thread, &td_attr, list_files, (void *)cli_args) !=0)
					print_error("error:download file failed");
		}
		else if(strncmp("exit",command,4)==0){
			break;
		}
		else{
			cout<<"command is not recognized"<<'\n';
		}
	}
    return 0;
}



