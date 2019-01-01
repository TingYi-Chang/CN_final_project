#include "client_config.hpp"
#include "client_connection.hpp"
#include "client_page.hpp"

int main(int argc, char *argv[]){
	Page page(argc, argv);

	while(!page.finish()){
		page.run_page();
	}
	return 0;
}