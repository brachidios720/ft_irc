/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tlegendr <tlegendr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 21:59:20 by hehuang           #+#    #+#             */
/*   Updated: 2025/03/09 15:48:39 by tlegendr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/Server.hpp"
#include "../Includes/Channel.hpp"
#include "../Includes/User.hpp"
#include <cstdlib>
#include <iostream>

#define MAX_PORT 65535 


Server::Server(){

}


Server::Server(std::string const &port, std::string const &password)
	:_password(password) , numConnection(0)
{
	if (isValidPort(port))
	{
		this->_port = std::atoi(port.c_str());
		poll_fds = new std::vector<pollfd>;
		std::cout << "Server created and running on port : "<< port << std::endl;
	}
	else
	{
		throw std::runtime_error("Invalid port number!\nPlease ensure it is numeric and between 0 and 65535.");
	}
}

bool Server::isValidPort(const std::string& portStr)
{
    if (portStr.empty())
		return false;
    for (size_t i = 0; i < portStr.length(); i++) {
        if (!std::isdigit(portStr[i])) {
            return false;
        }
    }
    long num = std::atol(portStr.c_str());
    if (num < 0 || num > 65535)
		return false;
    return true;
}

static void signalHandler(int signum) {
    std::cout << "\nReceived SIGINT (" << signum << "). Shutting down server gracefully..." << std::endl;
    Stop = true;
}

void Server::run() {
    std::cout << "Starting server initialization..." << std::endl;
	std::signal(SIGINT, signalHandler);

    // Creation de la socket
    SServer.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (SServer.fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server socket created." << std::endl;

    // Set socket options (e.g., SO_REUSEADDR)(pas sur d'en avoir besoin still need to check) 
    int opt = 1;
    if (setsockopt(SServer.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket options set." << std::endl;

    // Bind the server socket
    memset(&SServer.info, 0, sizeof(SServer.info));
    SServer.info.sin_family = AF_INET;
    SServer.info.sin_addr.s_addr = INADDR_ANY;
    SServer.info.sin_port = htons(_port);

    if (bind(SServer.fd, (struct sockaddr*)&SServer.info, sizeof(SServer.info)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket bound to port " << _port << "." << std::endl;

    // Listen on the socket
    if (listen(SServer.fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << _port << "." << std::endl;

    // Add the server socket to the poll vector
    pollfd server_poll;
    server_poll.fd = SServer.fd;
    server_poll.events = POLLIN;
    poll_fds->push_back(server_poll);
	serverLoop();
    std::cout << "Server is shutting down..." << std::endl;
}

void Server::serverLoop()
{
	// Main event loop
    std::cout << "Entering main event loop. Press CTRL+C to exit." << std::endl;
    while (!Stop) {
        int ret = poll(poll_fds->data(), poll_fds->size(), -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        // Check for new connection on the server socket
        if (poll_fds->at(0).revents & POLLIN) {
            int client_fd = accept(SServer.fd, nullptr, nullptr);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }
			std::cout << "DEBUG: New client connected: fd " << client_fd << std::endl;
			std::cout << "DEBUG: poll_fds size before push: " << poll_fds->size() << std::endl;
            std::cout << "New client connected: fd " << client_fd << std::endl;
            pollfd client_poll = { client_fd, POLLIN, 0 };
            client_poll.fd = client_fd;
            client_poll.events = POLLIN;
            poll_fds->push_back(client_poll);
			std::cout << "DEBUG: poll_fds size after push: " << poll_fds->size() << std::endl;
			//UserTab[client_poll.fd] = test;
			UserTab[client_poll.fd] = new User(client_poll.fd);// Maybe better
            numConnection++;
        }

        // Process data from connected clients
        for (size_t i = 1; i < poll_fds->size(); i++) {
            if (poll_fds->at(i).revents & POLLIN) {
                char buffer[1024];
                int bytes = read(poll_fds->at(i).fd, buffer, sizeof(buffer) - 1);
                if (bytes <= 0) {
                    std::cout << "Client fd " << poll_fds->at(i).fd << " disconnected." << std::endl;
                    close(poll_fds->at(i).fd);
                    poll_fds->erase(poll_fds->begin() + i);
                    i--; // Adjust index after removal
                    continue;
                }
                buffer[bytes] = '\0';
                std::cout << "Received from client fd " << poll_fds->at(i).fd << ": " << buffer << std::endl;
                User *callingUser = UserTab[poll_fds->at(i).fd];
                parseCommand(buffer, callingUser);
                
                //write(poll_fds->at(i).fd, RESPONSE TO USER, RESPONSE SIZE); RESPONSE BACK TO USER IF NEEDED
            }
        }
    }
}


void Server::parseCommand(const std::string command, User *user)
{
    std::string commandName;
    std::string message;
    std::stringstream ss(command);
    ss >> commandName;
    std::getline(ss, message);
    if (commandName == "CAP")
        CommandCAP(user);
    else if (commandName == "PASS")
        CommandPASS(user, message);
    else if (commandName == "NICK")
        CommandNICK(user, message);
    else if (commandName == "JOIN")
        CommandJOIN(user, message);
    else if (commandName == "USER")
        CommandUSER(user, message);
    else if (commandName == "NAMES")
        CommandNAMES(user, ChannelTab[message]);
    else if (commandName == "PRIVMSG")
        CommandPRIVMSG(user, message);
    else if (commandName == "PART")
        CommandPART(user, message);
    else if (commandName == "MODE")
        CommandMODE(user, message);
    else
    {
        std::string err = "ERROR: Command not found\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
    }
    
}



Server::~Server()
{
	for (std::vector<pollfd>::iterator it = poll_fds->begin(); it != poll_fds->end(); ++it)
		close(it->fd);
	for (std::map<int, User*>::iterator it = UserTab.begin(); it != UserTab.end(); ++it)
		delete it->second;
	UserTab.clear();
    delete poll_fds;
}
