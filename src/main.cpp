/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 10:13:42 by moetienn          #+#    #+#             */
/*   Updated: 2024/09/10 13:02:08 by moetienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Server.hpp"
#include "ServerParam.hpp"
#include "Webserver.hpp"
#include "Logger.hpp"

int main(int ac, char **av)
{
	Logger logger;

	logger.logMsg(INFO, "Program has started...");
	logger.logMsg(ERROR, "Error occured %s %s", "MALLOC FAILED!");
	logger.logMsg(WARNING, "PORT is not specified correcly!");
	// Added this;
	if (ac != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return (1);
	}

	ConfigParser ServerConf(av[1]);
	
	std::vector<ServerParam> servers = ServerConf.parse();
	
	if (servers.size() == 0)
	{
		std::cerr << "No server found in config file" << std::endl;
		return (1);
	}
	else
	{	
		std::cout << "===========SERVER INFO===========\n";
		for (size_t i = 0; i < servers.size(); i++)
		{	

			std::cout << "Server " << i << " listen on port " << servers[i].getPort() << std::endl;
			std::cout << "Server " << i << " server_name is " << servers[i].getServerName() << std::endl;
			std::cout << "Server " << i << " index is " << servers[i].getIndex() << std::endl;
			std::cout << "Server " << i << " root is " << servers[i].getRoot() << std::endl;
			std::cout << "Server " << i << " auto_index is " << servers[i].getAutoIndex() << std::endl;
			for (size_t j = 0; j < servers[i].getAllowedMethods().size(); j++)
			{
				std::cout << "Server " << i << " allowed_methods are: ";
				std::cout << servers[i].getAllowedMethods().at(j) << std::endl;
			}
		}
	};

	//Run first server
	std::cout << "===========RUNTIME===========\n";
	struct sockaddr_in address;

	Server  server(&address, servers[0].getPort());

	server.setup_server();
	server.run();

};

