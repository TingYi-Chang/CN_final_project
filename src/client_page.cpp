#include "client_page.hpp"
#include "client_user_io.hpp"
#include "common_app_protocol.hpp"
#include <iostream>

Page::Page(int argc, char *argv[]){
	_config = Config(argc, argv);
	_connection = Connection(_config.host_name(), _config.port());
	_finish = false;
	_state = PAGE_LOGIN;

}

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
		<< "Please enter ID" << std::endl;
	UserLine line;
	bool is_done = false;
	while(!is_done){
		to_stdin(line);
		if(line.is_command && line.topic == "\\help"){
			std::cout
				<< "" << std::endl
				<< "****** valid commands ******" << std::endl
				<< "\\signup : to register a new username." << std::endl
				<< "\\quit : to quit the client program." << std::endl
				<< "****************************" << std::endl
				<< "Please enter ID" << std::endl;
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
			while(!_connection.to_send(APP_LOGIN, line.topic))
				_connection.auto_reconnect();
			//to do
		}
		else{
			std::cout << "unknown command, please try again." << std::endl;
		}
	}
	

	return;
}

void Page::_run_page_signup(){
	_finish = true;
	std::cout << "Bye bye." << std::endl;
	return;
}
