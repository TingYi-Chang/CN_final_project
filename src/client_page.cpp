#include "client_page.hpp"
#include "client_user_io.hpp"
#include "common_app_protocol.hpp"
#include <iostream>

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

bool Page::_auto_send(int op, std::string &data){
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
				while(to_stdin(line, _queue) && line.is_command)
					std::cout << "Invalid command." << std::endl;
				if(!_auto_send(APP_LOGIN, line.topic)) return;
				if(!_auto_recv(resOp, resData)) return;
				if(resOp == APP_LOGIN){
					std::cout << "[Server]" << resData << std::endl;
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

void _run_page_lobby(){
	std::cout << "Welcome to lobby, but lobby is under construction." << std::endl;

}
