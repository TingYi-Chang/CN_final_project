#include "client_page.hpp"
#include "client_user_io.hpp"
#include "common_app_protocol.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>


void _split_out_ID(std::string &ID, std::string &message, std::string &raw){
	std::size_t found = raw.find_first_of(" ");
	if(found == std::string::npos){
		ID = raw;
		message = "";
	}
	else{
		ID = raw.substr(0, found);
		if(found+1 == raw.size())
			message = "";
		else
			message = raw.substr(found+1);
	}
}

bool _write_into_file(std::string filename, std::string &data){
	std::fstream file(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if(file){
		file.write(data.c_str(), data.size());
		file.close();
	}
	return true;
}

bool _check_file_existence(std::string filename){
	struct stat buf;
	if(stat(filename.c_str(), &buf) != -1)
		return true;
	return false;
}

bool _read_from_file(std::string filename, std::string &data){
	std::fstream file(filename, std::ios::in | std::ios::binary);
	if(file){
		file.seekg(0, file.end);
		int len = file.tellg();
		file.seekg(0, file.beg);
		char *buf = new char[len];
		file.read(buf, len);
		data = std::string(buf);
		file.close();
		delete[] buf;
	}
	return true;
}

Page::Page(int argc, char *argv[], UserQueue &queue)
	: _config(argc, argv)
	, _connection(_config.host_name(), _config.port())
	, _finish(false)
	, _state(PAGE_LOGIN)
	, _queue(queue)
{}

void Page::run_page(){
	switch(_state){
		case PAGE_EXIT:
			_run_page_exit();
			break;
		case PAGE_LOGIN:
			_run_page_login();
			break;
		case PAGE_SIGNUP:
			_run_page_signup();
			break;
		case PAGE_LOBBY:
			_run_page_lobby();
			break;
		case PAGE_CHAT:
			_run_page_chat();
			break;
		case PAGE_FILE:
			_run_page_file();
			break;
	}
	return;
}

bool Page::_auto_reconnect(){
	//std::cout << "Connection lost, trying to reconnect. Use \\cancel to stop.  " << std::flush;
	printf("Connection lost, trying to reconnect. Use \\cancel to stop.\n");
	//int animation = 0;
	UserLine line;
	while(!_connection.try_to_reconnect()){
		if(try_to_stdin(line, _queue) && line.is_command && line.topic == "\\cancel"){
			//std::cout 
			//	<< std::endl
			//	<< "Reconnection canceled." << std::endl;
			printf("Reconnection canceled.\n");
			return false;
		}
		//animation ++;
		//if(animation == 20)
		//	std::cout << "\rConnection lost, trying to reconnect. Use \\cancel to stop.. " << std::flush;
		//else if(animation == 40)
		//	std::cout << "\rConnection lost, trying to reconnect. Use \\cancel to stop..." << std::flush;
		//else if(animation >= 60){
		//	std::cout << "\rConnection lost, trying to reconnect. Use \\cancel to stop.  " << std::flush;
		//	animation = 0;
		//}
	}
	//std::cout << std::endl;
	//std::cout << "Reconnection succeeded." << std::endl;
	printf("Reconnection succeeded.\n");
	return true;
}

bool Page::_auto_send(int op, std::string data){
	while(!_connection.to_send(op, data)){
		if(!_auto_reconnect()){
			_state = PAGE_EXIT;
			return false;
		}
	}
	return true;
}

bool Page::_auto_recv(int &op, std::string &data){
	while(!_connection.to_recv(op, data)){
		if(!_auto_reconnect()){
			_state = PAGE_EXIT;
			return false;
		}
	}
	return true;
}

void Page::_run_page_exit(){
	_finish = true;
	printf("Bye bye.\n");
	return;
}

void Page::_run_page_login(){
	printf("#####################################\n");
	printf("#--------------welcome--------------#\n");
	printf("# please login first,               #\n");
	printf("# or use \\help to get command list. #\n");
	printf("#####################################\n");
	printf("Please enter your ID:\n");
	UserLine line;
	bool is_done = false;
	while(!is_done){
		to_stdin(line, _queue);
		if(line.is_command && line.topic == "\\help"){
			printf("\n");
			printf("****** valid commands ******\n");
			printf("\\help : to get command list.\n");
			printf("\\signup : to register a new username.\n");
			printf("\\quit : to quit the client program.\n");
			printf("****************************\n");
			printf("Please enter your ID:\n");
		}
		else if(line.is_command && line.topic == "\\signup"){
			_state = PAGE_SIGNUP;
			is_done = true;
		}
		else if(line.is_command && line.topic == "\\quit"){
			printf("quit.\n");
			_state = PAGE_EXIT;
			is_done = true;
		}
		else if(!line.is_command){
			int resOp;
			std::string resData;
			if(!_auto_send(APP_LOGIN, line.topic)) return;
			if(!_auto_recv(resOp, resData)) return;
			if(resOp == APP_LOGIN){
				printf("%s\n", resData.c_str());
				UserLine line2;
				while(to_stdin(line2, _queue) && line2.is_command)
					printf("Invalid command.\n");
				if(!_auto_send(APP_LOGIN, line2.topic)) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp == APP_MAIN){
					printf("[Server]%s\n", resData.c_str());
					_config.set_ID(line.topic);
					_state = PAGE_LOBBY;
					is_done = true;
				}
				else if(resOp == APP_ERROR){
					printf("[Server]%s\n", resData.c_str());
					printf("Please enter your ID:\n");
				}
				else{
					printf("[Warning]Server sent a strange op : %d\n", resOp);
				}
			}
			else if(resOp == APP_ERROR){
				printf("[Server]%s\n", resData.c_str());
				printf("Please enter your ID:\n");
			}
			else{
				printf("[Warning]Server sent a strange op : %d\n", resOp);
			}
		}
		else{
			printf("Invalid command, please try again.\n");
		}
	}
	

	return;
}

void Page::_run_page_signup(){
	int resOp;
	std::string resData, want = "I want to sign up.";
	if(!_auto_send(APP_SIGNUP, want)) return;
	if(!_auto_recv(resOp, resData)) return;
	UserLine line;
		
	if(resOp == APP_SIGNUP){
		bool is_done = false;
		while(!is_done){
			printf("[Server]%s\n", resData.c_str());
			while(to_stdin(line, _queue) && line.is_command)
				printf("Invalid command.\n");
			if(!_auto_send(APP_SIGNUP, line.topic)) return;
			if(!_auto_recv(resOp, resData)) return;
			if(resOp == APP_SIGNUP){
				printf("[Server]%s\n", resData.c_str());
				while(to_stdin(line, _queue) && line.is_command)
					printf("Invalid command.\n");
				if(!_auto_send(APP_SIGNUP, line.topic)) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp == APP_SIGNUP){
					printf("[Server]%s\n", resData.c_str());
					is_done = true;
				}
				else{
					printf("[Warning]Server sent a strange op : %d\n", resOp);
				}
			}
			else if(resOp == APP_ERROR){
				
			}
			else{
				printf("[Warning]Server sent a strange op : %d\n", resOp);
			}
		}
	}
	else{
		printf("[Warning]Server sent a strange op : %d\n", resOp);
	}
	_state = APP_LOGIN;
	return;
}

void Page::_run_page_lobby(){
	printf("#####################################\n");
	printf("#---------------lobby---------------#\n");
	printf("# use \\help to check command list.  #\n");
	printf("#####################################\n");
	while(!_notification.empty()){
		printf("%s\n", _notification.front().c_str());
		_notification.pop();
	}
	UserLine line;
	int op;
	std::string data;
	bool is_done = false;
	while(!is_done){
		if(_connection.try_to_recv(op, data)){
			if(op == APP_CHAT){
				std::string ID, message;
				_split_out_ID(ID, message, data);
				printf("[%s] sent you a message.\n", ID.c_str());			}
		}
		if(try_to_stdin(line, _queue)){
			if(line.is_command && line.topic == "\\help"){
				printf("\n");
				printf("****** valid commands ******\n");
				printf("\\help : to get command list.\n");
				printf("\\chat [id] : chat with someone.\n");
				printf("\\file : check mail box.\n");
				printf("\\quit : to quit the client program.\n");
				printf("****************************\n");
			}
			if(line.is_command && line.topic == "\\chat"){
				if(line.arg.size() != 1){
					printf("Useage : \\chat [id]\n");
				}
				else{
					_config.set_chatID(line.arg[0]);
					_state = PAGE_CHAT;
					is_done = true;
				}
			}
			else if(line.is_command && line.topic == "\\file"){
				_state = PAGE_FILE;
				is_done = true;
			}
			else if(line.is_command && line.topic == "\\quit"){
				printf("quit.\n");
				_state = PAGE_EXIT;
				is_done = true;
			}
			else if(line.is_command){
				printf("Invalid command, please try again.\n");
			}
		}
	}
}

void Page::_run_page_chat(){
	if(!_auto_send(APP_CHAT, _config.chatID())) return;
	int resOp;
	std::string resData;
	if(!_auto_recv(resOp, resData)) return;
	if(resOp == APP_ERROR){
		printf("[Server]%s\n", resData.c_str());
		_config.set_chatID("");
		_state = PAGE_LOBBY;
		return;
	}
	else if(resOp == APP_CHAT){
		std::string ID, message;
		_split_out_ID(ID, message, resData);
		while(ID != _config.chatID() && ID != "log"){
			_notification.push("[" + std::string(ID) + "] sent you a message.");
			if(!_auto_recv(resOp, resData)) return;
			_split_out_ID(ID, message, resData);
		}
		if(ID == "log"){
			printf("##### chatting with %s #####\n", _config.chatID().c_str());
			printf("%s\n", message.c_str());
		}
	}
	else{
		printf("[Warning]Server sent a strange op : %d\n", resOp);
		_state = PAGE_LOBBY;
		return;
	}

	printf("%s: ", _config.ID().c_str());
	fflush(stdout);
	bool is_done = false;
	UserLine line;
	std::string carrage = "\r                                                                                                                                  \r";
	while(!is_done){
		if(_connection.try_to_recv(resOp, resData)){
			if(resOp == APP_CHAT){
				std::string ID, message;
				_split_out_ID(ID, message, resData);
				if(ID == _config.chatID()){
					printf("%s%s: %s\n", carrage.c_str(), ID.c_str(), message.c_str());
				}
				else{
					_notification.push("[" + std::string(ID) + "] sent you a message.");
				}
			}
			else{
				printf("[Warning]Server sent a strange op : %d\n", resOp);
			}
		}
		if(try_to_stdin(line, _queue)){
			if(line.is_command && line.topic == "\\help"){
				printf("%s", carrage.c_str());
				printf("****** valid commands ******\n");
				printf("\\help : to get command list.\n");
				printf("\\back : to go back to the lobby.\n");
				printf("****************************\n");
				printf("%s: ", _config.ID().c_str());
				fflush(stdout);
			}
			else if(line.is_command && line.topic == "\\back"){
				printf("Go back to the lobby.\n");
				if(!_auto_send(APP_MAIN, "\\back")) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp != APP_MAIN)
					printf("[Warning]Server sent a strange op : %d\n", resOp);
				_state = PAGE_LOBBY;
				is_done = true;
			}
			else if(!line.is_command){
				if(!_auto_send(APP_CHAT, line.topic)) return;
				printf("%s: ", _config.ID().c_str());
				fflush(stdout);
			}
			else{
				printf("Invalid command, please try again.\n");
				printf("%s: ", _config.ID().c_str());
				fflush(stdout);
			}
		}
	}

	_config.set_chatID("");
	return;
}

void Page::_run_page_file(){

	if(!_auto_send(APP_FILE, "anyways")) return;
	int resOp;
	std::string resData;
	bool got_file_list = false;
	while(!got_file_list){
		if(!_auto_recv(resOp, resData)) return;
		if(resOp == APP_CHAT){
			std::string ID, message;
			_split_out_ID(ID, message, resData);
			_notification.push("[" + std::string(ID) + "] sent you a message.");
		}
		else if(resOp == APP_ERROR){
			printf("[Server]%s\n", resData.c_str());
			_state = PAGE_LOBBY;
			return;
		}
		else if(resOp == APP_FILE){
			printf("Avalible file(s):\n%s\n", resData.c_str());
			got_file_list = true;
		}
		else{
			printf("[Warning]Server sent a strange op : %d\n", resOp);
			_state = PAGE_LOBBY;
			return;
		}

	}
	bool is_done= false;
	UserLine line;
	while(!is_done){
		if(_connection.try_to_recv(resOp, resData)){
			if(resOp == APP_CHAT){
				std::string ID, message;
				_split_out_ID(ID, message, resData);
				_notification.push("[" + std::string(ID) + "] sent you a message.");
			}
			else{
				printf("[Warning]Server sent a strange op : %d\n", resOp);
				_state = PAGE_LOBBY;
				return;
			}
		}
		if(try_to_stdin(line, _queue)){
			if(line.is_command && line.topic == "\\help"){
				printf("****** valid commands ******\n");
				printf("\\help : to get command list.\n");
				printf("\\download [filename] : download the required file.\n");
				printf("\\send [username] [filename] : send file to another user.\n");
				printf("\\back : to go back to the lobby.\n");
				printf("****************************\n");
			}
			else if(line.is_command && line.topic == "\\back"){
				printf("Go back to the lobby.\n");
				if(!_auto_send(APP_MAIN, "\\back")) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp != APP_MAIN)
					printf("[Warning]Server sent a strange op : %d\n", resOp);
				_state = PAGE_LOBBY;
				is_done = true;
			}
			else if(line.is_command && line.topic == "\\download"){
				if(line.arg.size() != 1){
					printf("Useage : \\download [filename]\n");
				}
				else{
					if(!_auto_send(APP_FILE, "download")) return;
					bool got_confirm = false;
					while(!got_confirm){
						if(!_auto_recv(resOp, resData)) return;
						if(resOp == APP_CHAT){
							std::string ID, message;
							_split_out_ID(ID, message, resData);
							_notification.push("[" + std::string(ID) + "] sent you a message.");
						}
						else if(resOp == APP_FILE){
							got_confirm = true;
						}
						else{
							printf("[Warning]Server sent a strange op : %d\n", resOp);
							_state = PAGE_LOBBY;
							return;
						}
					}
					if(!_auto_send(APP_FILE, line.arg[0])) return;
					got_confirm = false;
					while(!got_confirm){
						if(!_auto_recv(resOp, resData)) return;
						if(resOp == APP_CHAT){
							std::string ID, message;
							_split_out_ID(ID, message, resData);
							_notification.push("[" + std::string(ID) + "] sent you a message.");
						}
						else if(resOp == APP_ERROR){
							printf("[Server]%s\n", resData.c_str());
							got_confirm = true;
						}
						else if(resOp == APP_FILE){
							if(_write_into_file(line.arg[0], resData))
								printf("Download %s susseeded.\n", line.arg[0].c_str());
							else
								printf("Download %s failed.\n", line.arg[0].c_str());
							got_confirm = true;
						}
						else{
							printf("[Warning]Server sent a strange op : %d\n", resOp);
							_state = PAGE_LOBBY;
							return;
						}
					}
				}
			}
			else if(line.is_command && line.topic == "\\send"){
				if(line.arg.size() != 2){
					printf("Useage : \\send [username] [filename]\n");
				}
				else if(!_check_file_existence(line.arg[1])){
					printf("[Warning]No such file.\n");
				}
				else{
					if(!_auto_send(APP_FILE, "upload")) return;
					bool got_confirm = false;
					while(!got_confirm){
						if(!_auto_recv(resOp, resData)) return;
						if(resOp == APP_CHAT){
							std::string ID, message;
							_split_out_ID(ID, message, resData);
							_notification.push("[" + std::string(ID) + "] sent you a message.");
						}
						else if(resOp == APP_FILE){
							got_confirm = true;
						}
						else{
							printf("[Warning]Server sent a strange op : %d\n", resOp);
							_state = PAGE_LOBBY;
							return;
						}
					}
					if(!_auto_send(APP_FILE, line.arg[0] + " " + line.arg[1])) return;
					got_confirm = false;
					bool ready_to_send = false;
					while(!got_confirm){
						if(!_auto_recv(resOp, resData)) return;
						if(resOp == APP_CHAT){
							std::string ID, message;
							_split_out_ID(ID, message, resData);
							_notification.push("[" + std::string(ID) + "] sent you a message.");
						}
						else if(resOp == APP_FILE){
							got_confirm = true;
							ready_to_send = true;
						}
						else if(resOp == APP_ERROR){
							printf("[Server]%s\n", resData.c_str());
							got_confirm = true;
						}
						else{
							printf("[Warning]Server sent a strange op : %d\n", resOp);
							_state = PAGE_LOBBY;
							return;
						}
					}
					if(ready_to_send){
						printf("Sending %s to %s.\n", line.arg[1].c_str(), line.arg[0].c_str());
						std::string raw_file_content;
						_read_from_file(line.arg[1], raw_file_content);
						if(!_auto_send(APP_FILE, raw_file_content)) return;
						if(!_auto_recv(resOp, resData)) return;
						if(resOp != APP_FILE){
							printf("[Warning]Server sent a strange op : %d\n", resOp);
							_state = PAGE_LOBBY;
							return;
						}
					}
				}
			}
			else if(line.is_command){
				printf("Invalid command, please try again.\n");
			}
		}
	}


}
