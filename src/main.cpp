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
    
    // Added this;
    if (ac != 2)
    {
        logger.logMsg(ERROR, "Usage: ./webserv <config_file>");
        return (1);
    }

    ConfigParser ServerConf(av[1]);
    std::vector<ServerParam> servers = ServerConf.parse();

    if (servers.size() == 0)
    {
        logger.logMsg(ERROR, "No server found in config file.");
        return (1);
    }
    else
    {
        for (size_t i = 0; i < servers.size(); i++)
        {
            logger.logMsg(INFO, "Server %zu listens on port %d", i, servers[i].getPort());
            logger.logMsg(INFO, "Server %zu server_name is %s", i, servers[i].getServerName().c_str());
            logger.logMsg(INFO, "Server %zu index is %s", i, servers[i].getIndex().c_str());
            logger.logMsg(INFO, "Server %zu root is %s", i, servers[i].getRoot().c_str());
            logger.logMsg(INFO, "Server %zu auto_index is %d", i, servers[i].getAutoIndex());

            // Log allowed methods for each server
            for (size_t j = 0; j < servers[i].getAllowedMethods().size(); j++)
            {
                logger.logMsg(INFO, "Server %zu allowed_methods: %s", i, servers[i].getAllowedMethods().at(j).c_str());
            }
        }
    }

    // Run first server
    struct sockaddr_in address;

    Server server(&address, servers[0].getPort(), HOST);
    server.setup_server();
    server.run();

    return 0;
}
