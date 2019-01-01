#include "client_page.hpp"
#include "client_user_io.hpp"
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
	}
	return;
}

void Page::_run_page_exit(){
	_finish = true;
	std::cout << "Bye bye." << std::endl;
	return;
}

void Page::_run_page_login(){
	std::cout	<< "################################" << std::endl
				<< "#            welcome           #" << std::endl
				<< "# \\signup : register.          #" << std::endl
				<< "# \\login : log in.             #" << std::endl
				<< "################################" << std::endl;
	UserLine line;
	bool is_done = false;
	while(!is_done){
		while(!try_to_stdin(line));
		if(line.is_command && line.topic == "\\signup"){
			//send signup!!!!!
			std::cout << "send signup." << std::endl;
			is_done = true;
		}
		else if(line.is_command && line.topic == "\\login"){
			//send login!!!
			std::cout << "send login." << std::endl;
			is_done = true;
		}
		else if(line.is_command && line.topic == "\\quit"){
			std::cout << "quit." << std::endl;
			_state = PAGE_EXIT;
			is_done = true;
		}
		else{
			std::cout << "unknown command, please try again." << std::endl;
		}
	}
	

	return;
}