#include "client_page.hpp"
#include "client_user_io.hpp"
#include "common_app_protocol.hpp"
#include <iostream>

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
	////////////
	return true;
}

bool _check_file_existence(std::string filename){
	////////////
	return true;
}

bool _read_from_file(std::string filename, std::string &data){
	////////////
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
	}
	return;
}

bool Page::_auto_reconnect(){
	std::cout << "Connection lost, trying to reconnect. Use \\cancel to stop.  " << std::flush;
	int animation = 0;
	UserLine line;
	while(!_connection.try_to_reconnect()){
		if(try_to_stdin(line, _queue) && line.is_command && line.topic == "\\cancel"){
			std::cout 
				<< std::endl
				<< "Reconnection canceled." << std::endl;
			return false;
		}
		animation ++;
		if(animation == 20)
			std::cout << "\rConnection lost, trying to reconnect. Use \\cancel to stop.. " << std::flush;
		else if(animation == 40)
			std::cout << "\rConnection lost, trying to reconnect. Use \\cancel to stop..." << std::flush;
		else if(animation >= 60){
			std::cout << "\rConnection lost, trying to reconnect. Use \\cancel to stop.  " << std::flush;
			animation = 0;
		}
	}
	std::cout << std::endl;
	std::cout << "Reconnection succeeded." << std::endl;
	return true;
}

bool Page::_auto_send(int op, std::string data){
	while(!_connection.to_send(op, data))
		if(!_auto_reconnect()){
			_state = PAGE_EXIT;
			return false;
		}
	return true;
}

bool Page::_auto_recv(int &op, std::string &data){
	while(!_connection.to_recv(op, data))
		if(!_auto_reconnect()){
			_state = PAGE_EXIT;
			return false;
		}
////////////
	std::cout << "[YOYO]recieved" << op << " " << data << std::endl;
	return true;
}

void Page::_run_page_exit(){
	_finish = true;
	std::cout << "Bye bye." << std::endl;
	return;
}

void Page::_run_page_login(){
	std::cout
		<< "#####################################" << std::endl
		<< "#--------------welcome--------------#" << std::endl
		<< "# please login first,               #" << std::endl
		<< "# or use \\help to get command list. #" << std::endl
		<< "#####################################" << std::endl
		<< "Please enter your ID:" << std::endl;
	UserLine line;
	bool is_done = false;
	while(!is_done){
		to_stdin(line, _queue);
		if(line.is_command && line.topic == "\\help"){
			std::cout
				<< "" << std::endl
				<< "****** valid commands ******" << std::endl
				<< "\\help : to get command list." << std::endl
				<< "\\signup : to register a new username." << std::endl
				<< "\\quit : to quit the client program." << std::endl
				<< "****************************" << std::endl
				<< "Please enter your ID:" << std::endl;
		}
		else if(line.is_command && line.topic == "\\signup"){
			_state = PAGE_SIGNUP;
			is_done = true;
		}
		else if(line.is_command && line.topic == "\\quit"){
			std::cout << "quit." << std::endl;
			_state = PAGE_EXIT;
			is_done = true;
		}
		else if(!line.is_command){
			int resOp;
			std::string resData;
			if(!_auto_send(APP_LOGIN, line.topic)) return;
			if(!_auto_recv(resOp, resData)) return;
			if(resOp == APP_LOGIN){
				std::cout << resData << std::endl;
				UserLine line2;
				while(to_stdin(line2, _queue) && line2.is_command)
					std::cout << "Invalid command." << std::endl;
				if(!_auto_send(APP_LOGIN, line2.topic)) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp == APP_MAIN){
					std::cout << "[Server]" << resData << std::endl;
					_config.set_ID(line.topic);
					_state = PAGE_LOBBY;
					is_done = true;
				}
				else if(resOp == APP_ERROR){
					std::cout << "[Server]" << resData << std::endl;
					std::cout << "Please enter your ID:" << std::endl;
				}
				else{
					std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
				}
			}
			else if(resOp == APP_ERROR){
				std::cout << "[Server]" << resData << std::endl;
				std::cout << "Please enter your ID:" << std::endl;
			}
			else{
				std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
			}
		}
		else{
			std::cout << "Invalid command, please try again." << std::endl;
		}
	}
	

	return;
}

void Page::_run_page_signup(){
	int resOp;
	std::string resData, want = "I want to sign up.";
	UserLine line;
	if(!_auto_send(APP_SIGNUP, want)) return;
	if(!_auto_recv(resOp, resData)) return;
	if(resOp == APP_SIGNUP){
		std::cout << "[Server]" << resData << std::endl;
		while(to_stdin(line, _queue) && line.is_command)
			std::cout << "Invalid command." << std::endl;
		if(!_auto_send(APP_SIGNUP, line.topic)) return;
		if(!_auto_recv(resOp, resData)) return;
		if(resOp == APP_SIGNUP){
			std::cout << "[Server]" << resData << std::endl;
			while(to_stdin(line, _queue) && line.is_command)
				std::cout << "Invalid command." << std::endl;
			if(!_auto_send(APP_SIGNUP, line.topic)) return;
			if(!_auto_recv(resOp, resData)) return;
			if(resOp == APP_SIGNUP){
				std::cout << "[Server]" << resData << std::endl;
			}
			else if(resOp == APP_ERROR){
				std::cout << "[Server]" << resData << std::endl;
			}
			else{
				std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
			}
		}
		else if(resOp == APP_ERROR){
			std::cout << "[Server]" << resData << std::endl;
		}
		else{
			std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
		}
	}
	else{
		std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
	}
	_state = APP_LOGIN;
	return;
}

void Page::_run_page_lobby(){
	std::cout
		<< "#####################################" << std::endl
		<< "#---------------lobby---------------#" << std::endl
		<< "# use \\help to check command list.  #" << std::endl
		<< "#####################################" << std::endl;
	while(!_notification.empty()){
		std::cout << _notification.front() << std::endl;
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
				std::cout << "[" << ID << "] sent you a message." << std::endl;
			}
		}
		if(try_to_stdin(line, _queue)){
			if(line.is_command && line.topic == "\\help"){
				std::cout
					<< "" << std::endl
					<< "****** valid commands ******" << std::endl
					<< "\\help : to get command list." << std::endl
					<< "\\chat [id] : chat with someone." << std::endl
					<< "\\file : check mail box." << std::endl
					<< "\\quit : to quit the client program." << std::endl
					<< "****************************" << std::endl;
			}
			if(line.is_command && line.topic == "\\chat"){
				if(line.arg.size() != 1){
					std::cout << "Useage : \\chat [id]" << std::endl;
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
				std::cout << "quit." << std::endl;
				_state = PAGE_EXIT;
				is_done = true;
			}
			else if(line.is_command){
				std::cout << "Invalid command, please try again." << std::endl;
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
		std::cout << "[Server]" << resData << std::endl;
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
			std::cout << "##### chatting with " << _config.chatID() << " #####" << std::endl;
			std::cout << message;
		}
	}
	else{
		std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
		_state = PAGE_LOBBY;
		return;
	}

	std::cout << _config.ID() << ": " << std::flush;
	bool is_done = false;
	UserLine line;
	std::string carrage = "\r                                                                                                                                  \r";
	while(!is_done){
		if(_connection.try_to_recv(resOp, resData)){
			if(resOp == APP_CHAT){
				std::string ID, message;
				_split_out_ID(ID, message, resData);
				if(ID == _config.chatID()){
					std::cout
						<< carrage << ID << ": " << message << std::endl;
				}
				else{
					_notification.push("[" + std::string(ID) + "] sent you a message.");
				}
			}
			else{
				std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
			}
		}
		if(try_to_stdin(line, _queue)){
			if(line.is_command && line.topic == "\\help"){
				std::cout
					<< carrage
					<< "****** valid commands ******" << std::endl
					<< "\\help : to get command list." << std::endl
					<< "\\back : to go back to the lobby." << std::endl
					<< "****************************" << std::endl
					<< _config.ID() << ": " << std::flush;
			}
			else if(line.is_command && line.topic == "\\back"){
				std::cout << carrage << "Go back to the lobby." << std::endl;
				if(!_auto_send(APP_MAIN, "\\back")) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp != APP_MAIN)
					std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
				_state = PAGE_LOBBY;
				is_done = true;
			}
			else if(!line.is_command){
				if(!_auto_send(APP_CHAT, line.topic)) return;
				std::cout << _config.ID() << ": " << std::flush;
			}
			else{
				std::cout
					<< "Invalid command, please try again." << std::endl
					<< _config.ID() << ": " << std::flush;
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
			std::cout << "[Server]" << resData << std::endl;
			_state = PAGE_LOBBY;
			return;
		}
		else if(resOp == APP_FILE){
			std::cout << "Avalible file(s):" << std::endl << resData << std::endl;
			got_file_list = true;
		}
		else{
			std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
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
				std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
				_state = PAGE_LOBBY;
				return;
			}
		}
		if(try_to_stdin(line, _queue)){
			if(line.is_command && line.topic == "\\help"){
				std::cout
					<< "****** valid commands ******" << std::endl
					<< "\\help : to get command list." << std::endl
					<< "\\download [filename] : download the required file." << std::endl
					<< "\\send [username] [filename] : send file to another user." << std::endl
					<< "\\back : to go back to the lobby." << std::endl
					<< "****************************" << std::endl;
			}
			else if(line.is_command && line.topic == "\\back"){
				std::cout << "Go back to the lobby." << std::endl;
				if(!_auto_send(APP_MAIN, "\\back")) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp != APP_MAIN)
					std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
				_state = PAGE_LOBBY;
				is_done = true;
			}
			else if(line.is_command && line.topic == "\\download"){
				if(line.arg.size() != 1){
					std::cout << "Useage : \\download [filename]" << std::endl;
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
							std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
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
							std::cout << "[Server]" << resData << std::endl;
							got_confirm = true;
						}
						else if(resOp == APP_FILE){
							if(_write_into_file(line.arg[0], resData))
								std::cout << "Download " << line.arg[0] << " susseeded." << std::endl;
							else
								std::cout << "Download " << line.arg[0] << " failed." << std::endl;
							got_confirm = true;
						}
						else{
							std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
							_state = PAGE_LOBBY;
							return;
						}
					}
				}
			}
			else if(line.is_command && line.topic == "\\send"){
				if(line.arg.size() != 2){
					std::cout << "Useage : \\send [username] [filename]" << std::endl;
				}
				else if(!_check_file_existence(line.arg[1])){
					std::cout << "[Warning]No such file." << std::endl;
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
							std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
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
							std::cout << "[Server]" << resData << std::endl;
							got_confirm = true;
						}
						else{
							std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
							_state = PAGE_LOBBY;
							return;
						}
					}
					if(ready_to_send){
						std::cout << "Sending " << line.arg[1] << " to " << line.arg[0] << "." << std::endl;
						std::string raw_file_content;
						_read_from_file(line.arg[1], raw_file_content);
						if(!_auto_send(APP_FILE, raw_file_content)) return;
						if(!_auto_recv(resOp, resData)) return;
						if(resOp != APP_FILE){
							std::cout << "[Warning]Server sent a strange op :" << resOp << std::endl;
							_state = PAGE_LOBBY;
							return;
						}
					}
				}
			}
			else if(line.is_command){
				std::cout << "Invalid command, please try again." << std::endl;
			}
		}
	}


}
