/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hehuang <hehuang@student.42lehavre.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 18:33:49 by hehuang           #+#    #+#             */
/*   Updated: 2025/03/12 18:00:01 by hehuang          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/Channel.hpp"
#include <algorithm>
#include <string>
#include <vector>

Channel::Channel(std::string name){};
Channel::~Channel(){};
std::string	Channel::getName(){return this->_name;}
void		Channel::AddUser(User *user, std::string mdp, int super){};
void		Channel::DelUser(User *user){};
std::string	Channel::getStringUser(std::string name){return "test";};
void		Channel::SendMsg(User *user, std::string message){};
bool		Channel::IsHere(User *user)
{
	std::map<User *, int>::iterator it = std::find(this->UserBook.begin(), this->UserBook.end(), user);
	if (it != this->UserBook.end())
		return (true);
	return (false);
};
void		Channel::SetPassword(std::string password){this->_password = password;};
void		Channel::SetMode(char mode, bool x)
{
	if (mode == 'i')_modeI = x;
	else if (mode == 't')_modeT = x;
	else if (mode == 'k')_modeK = x;
	else if (mode == 'l')_modeL = x;
};
bool		Channel::getMode(char mode)
{
	if (mode == 'i')return _modeI;
	else if (mode == 't')return _modeT;
	else if (mode == 'k')return _modeK;
	else if (mode == 'l')return _modeL;
    return false;
};
std::string	Channel::getPassword(void){return this->_password;};
bool		Channel::isEmpty(){return this->UserBook.empty();};
std::string	Channel::getTopic(){return this->_topic;};
void		Channel::setTopic(std::string topic){this->_topic = topic;};
//int			Channel::isOp(std::string nickname){};
void		Channel::changeOp(std::string nickname, int op){};
void		Channel::SetUserLimit(int limit){this->_userLimit = limit;};
int			Channel::getNbUser(){return this->UserBook.size();};
bool		Channel::isPlace(){return this->getNbUser() < this->_userLimit;};
void		Channel::addUserInvite(User *user){this->UserInvite.push_back(user);};
bool		Channel::IsInvite(User *user)
{
	std::vector<User *>::iterator it = std::find(this->UserInvite.begin(), this->UserInvite.end(), user);
	if (it != this->UserInvite.end())
		return (true);
	return (false);
};
std::vector<std::string> Channel::getUserNicknames()
{
	std::vector<std::string> res;
	for(std::map<User*, int>::iterator it = this->UserBook.begin(); it != this->UserBook.end(); ++it)
	{
		res.push_back(it->first->getNickname());
	}
	return res;
};
void		Channel::setInviteOnly(bool status){this->_modeI = status;};//
bool		Channel::isInviteOnly(){return this->_modeI;};//
bool		Channel::isTopicRestricted(){return this->_modeT;};//
void		Channel::setTopicRestricted(bool status){this->_modeT = status;};//
int			Channel::getUserLimite(){return this->_userLimit;};

