#ifndef _CLIENT_PAGE_HPP_
#define _CLIENT_PAGE_HPP_

#include "client_config.hpp"
#include "client_connection.hpp"

#define PAGE_EXIT 0
#define PAGE_LOGIN 1
#define PAGE_SIGNUP 2

int run_page(int curr_page, Connection &connection);

class Page{
public:
	Page(int argc, char *argv[]);
	bool finish(){return _finish;}
	void run_page();

private:
	bool _finish;
	int _state;
	Config _config;
	Connection _connection;
	void _run_page_exit();
	void _run_page_login();
	void _run_page_signup();
};

#endif