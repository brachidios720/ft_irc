/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hehuang <hehuang@student.42lehavre.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 23:19:29 by hehuang           #+#    #+#             */
/*   Updated: 2025/02/26 23:07:54 by hehuang          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/User.hpp"
#include <iostream>
#include <string>

User::User(int socket): _socket(socket)
{
	std::cout << "User constructor called" << std::endl;
	this->_isDown = 0;
    this->_buffCommand = "";
    this->_username = "";
    this->_channel = "";
    this->_nickname = "";
    this->_hostname = "";
    this->_getNick = false;
}

//GETTER

std::string	User::getUsername()
{
	return this->_username;
}

std::string	User::getNickname()
{
	return this->_nickname;
}

std::string	User::getChannel()
{
	return this->_channel;
}

std::string	User::getHostname()
{
	return this->_hostname;
}

int User::getSocket()
{
	return this->_socket;
}

int	User::getisDown()
{
	return this->_isDown;
}

bool	User::getGetNick()
{
	return this->_getNick;
}

std::string	User::getID()
{
	std::string ID = ":" + this->_nickname + "!" + this->_username + "@" + this->_ip;
    return (ID);
}

std::string	User::getbuffCommand()
{
	return this->_buffCommand;
}

std::string	User::getIp()
{
	return this->_ip;
}

//SETTER

void	User::setisDown(int down)
{
	this->_isDown = down;
}

void	User::setUsername(std::string username)
{
	this->_username = username;
}

void	User::setNickname(std::string nickname)
{
	this->_nickname = nickname;
}

void	User::setChannel(std::string channel)
{
	this->_channel = channel;
}

void	User::setHostname(std::string hostname)
{
	this->_hostname = hostname;
}

void	User::setGetNick()
{
	if (this->_getNick)
		this->_getNick = false;
	else
		this->_getNick = true;
}

void	User::setbuffCommand(std::string command)
{
	this->_buffCommand = command;
}

void	User::setIp(std::string ip)
{
	this->_ip = ip;
}

//AUTRES

void	User::joinbuffCommand(std::string command)
{
	this->_buffCommand += command;
}

void	User::incrementisDown()
{
	this->_isDown++;
}


User::~User()
{
	std::cout << "User Deconstructor called" << this->getSocket() << std::endl;
}
