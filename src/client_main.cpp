#include "client_config.hpp"
#include "client_connection.hpp"
#include "client_page.hpp"
#include "client_user_io.hpp"
#include <thread>
#include <functional>

int main(int argc, char *argv[]){
	UserQueue queue;
	std::thread slave(slave_getting_user_line, std::ref(queue));
	slave.detach();

	Page page(argc, argv, queue);
	while(!page.finish()){
		page.run_page();
	}
	return 0;
}
