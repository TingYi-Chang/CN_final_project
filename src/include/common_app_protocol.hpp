#ifndef _COMMON_APP_PROTOCOL_HPP_
#define _COMMON_APP_PROTOCOL_HPP_

#define APP_ERROR 0
#define APP_LOGIN 1
#define APP_SIGNUP 2
#define APP_CHAT 3
#define APP_FILE 4

typedef struct{
	int op;
	int data_len;
}AppHeader;

typedef struct{
	std::string username;
	std::string password;
}LoginReq;

typedef struct{
	bool success;
	std::string message;
}LoginRes;

typedef struct{
	std::string username;
	std::string password;
}SignupReq;

typedef struct{
	bool success;
	std::string message;
}SignupRes;

typedef struct{
	std::string target_user;
	std::string message;
}ChatReq;

typedef struct{
	bool success;
	std::string target_user;
	std::string message;
}ChatRes;

#endif