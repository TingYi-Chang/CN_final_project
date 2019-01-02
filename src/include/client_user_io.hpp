#ifndef _CLIENT_USER_IO_HPP_
#define _CLIENT_USER_IO_HPP_

#include <vector>
#include <string>
#include <mutex>
#include <queue>

typedef struct{
	bool is_command;
	std::string topic;
	std::vector<std::string> arg;
}UserLine;

typedef struct {
	std::mutex mutex;
	std::queue<UserLine> q;
}UserQueue;

bool try_to_stdin(UserLine &line, UserQueue &queue);
bool to_stdin(UserLine &line, UserQueue &queue);
void slave_getting_user_line(UserQueue &queue);

#endif