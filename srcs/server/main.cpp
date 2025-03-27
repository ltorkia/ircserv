#include "../../incs/server/Server.hpp"

// === NAMESPACES ===
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/colors.hpp"

using namespace server_messages;
using namespace colors;

// =========================================================================================

int main(int argc, char **argv)
{

	if (argc != 3)
	{
		std::cout << RED << "❌ Usage: " << RESET << ERR_INVALID_ARGUMENTS << std::endl << std::endl;
		return 1;
	}

	try
	{
		std::string port(argv[1]);
		std::string password(argv[2]);
		
		Server server(port, password);
		server.launch();

	} 
	catch (const std::exception &e)
	{
		std::cerr << RED << "❌ Error: " << RESET << e.what() << std::endl << std::endl;
		return 1;
	}
	
	return 0;
}