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
	return;
}

void Page::_run_page_login(){
	std::cout	<< "################################" << std::endl;
				<< "#            welcome           #" << std::endl;
				<< "# \\signup : register.          #" << std::endl;
				<< "# \\login : log in.             #" << std::endl;
				<< "################################" << std::endl;
	UserLine line;
	while(!try_to_stdin(line)){
		if(!line.is_command || line.topic != "\\signup" || line.topic != "\\login"){
			std::cout << "unknown command, please try again." << std::endl;
		}
		else
			break;
	}
	if(line.topic != "\\signup"){
		//send signup!!!!!
	}
	else if(line.topic != "\\login"){
		//send login!!!

	}

	return;
}