#ifndef _CLIENT_PAGE_HPP_
#define _CLIENT_PAGE_HPP_

#include "client_config.hpp"
#include "client_connection.hpp"
#include "client_user_io.hpp"

#define PAGE_EXIT 0
#define PAGE_LOGIN 1
#define PAGE_SIGNUP 2
#define PAGE_LOBBY 3
#define PAGE_CHAT 4
#define PAGE_FILE 5

int run_page(int curr_page, Connection &connection);

class Page{
public:
	Page(int argc, char *argv[], UserQueue &queue);
	bool finish(){return _finish;}
	void run_page();

private:
	bool _finish;
	int _state;
	Config _config;
	Connection _connection;
	UserQueue &_queue;
	std::queue<std::string> _notification;
	bool _auto_reconnect();
	bool _auto_send(int op, std::string data);
	bool _auto_recv(int &op, std::string &data);
	void _run_page_exit();
	void _run_page_login();
	void _run_page_signup();
	void _run_page_lobby();
	void _run_page_chat();
};

#endif