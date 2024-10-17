/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moetienn <moetienn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 10:13:42 by moetienn          #+#    #+#             */
/*   Updated: 2024/10/17 08:27:32 by moetienn         ###   ########.fr       */
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
            logger.logMsg(INFO, "Server %zu host is %s", i, servers[i].getHost().c_str());
            logger.logMsg(INFO, "Server %zu server_name is %s", i, servers[i].getServerName().c_str());
            for (size_t j = 0; j < servers[i].getLocations().size(); j++)
            {
                // logger.logMsg(INFO, "Server %zu location %zu is %s", i, j, servers[i].getLocations().at(j).getUri().c_str());
                logger.logMsg(INFO, "Server %zu location %zu root is %s", i, j, servers[i].getLocations().at(j).getRoot().c_str());
                logger.logMsg(INFO, "Server %zu location %zu auto_index is %d", i, j, servers[i].getLocations().at(j).getAutoIndex());
                logger.logMsg(INFO, "Server %zu location %zu index is %s", i, j, servers[i].getLocations().at(j).getIndex().c_str());
                for (size_t k = 0; k < servers[i].getLocations().at(j).getAllowedMethods().size(); k++)
                {
                    logger.logMsg(INFO, "Server %zu location %zu allowed_methods: %s", i, j, servers[i].getLocations().at(j).getAllowedMethods().at(k).c_str());
                }
            }
        }
    }

    // Run first server
    struct sockaddr_in address;

    Server server(&address, servers);

    try
    {
    server.setup_server();
    }
    catch (const std::exception& e)
    {
        Logger::logMsg(ERROR, "Failed to start: %s", e.what());
        return 1;
    }
    server.run();

    return 0;
}
