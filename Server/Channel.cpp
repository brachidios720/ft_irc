/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tlegendr <tlegendr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 17:27:49 by tlegendr          #+#    #+#             */
/*   Updated: 2025/04/08 15:30:10 by tlegendr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/Channel.hpp"

Channel::Channel() : _name(""),  _userLimit(0), _modeK(false), _modeT(false), _modeL(false), _modeI(false), _topicRestricted(false)
{
}

Channel::Channel(std::string name) : _name(name), _userLimit(0), _modeK(false), _modeT(false), _modeL(false), _modeI(false), _topicRestricted(false)
{
}

Channel::~Channel()
{
}

std::string Channel::getName(void)
{
    return _name;
}

void Channel::AddUser(User *user, std::string mdp, int super)
{
    (void)mdp;
    UserBook[user] = super;
    if (super == 1)
        user->setChannel(_name);
}

void Channel::DelUser(User *user)
{
    UserBook.erase(user);
}

std::string Channel::getStringUser(std::string name)
{
    for (std::map<User *, int>::iterator it = UserBook.begin(); it != UserBook.end(); ++it)
    {
        if (it->first->getNickname() == name)
            return it->first->getNickname();
    }
    return "";
}

void Channel::SendMsg(User *user, std::string message)
{
    for (std::map<User *, int>::iterator it = UserBook.begin(); it != UserBook.end(); ++it)
    {
        if (it->first != user)
            send(it->first->getSocket(), message.c_str(), message.length(), 0);
    }
}

bool Channel::IsHere(User *user)
{
    return UserBook.find(user) != UserBook.end();
}

void Channel::SetPassword(std::string password)
{
    _password = password;
}

void Channel::SetMode(char mode, bool x)
{
    if (mode == 'k')
        _modeK = x;
    else if (mode == 't')
        _modeT = x;
    else if (mode == 'l')
        _modeL = x;
    else if (mode == 'i')
        _modeI = x;
}

bool Channel::getMode(char mode)
{
    if (mode == 'k')
        return _modeK;
    else if (mode == 't')
        return _modeT;
    else if (mode == 'l')
        return _modeL;
    else if (mode == 'i')
        return _modeI;
    return false;
}

std::string Channel::getPassword(void)
{
    return _password;
}

bool Channel::isEmpty()
{
    return UserBook.empty();
}

std::string Channel::getTopic()
{
    return _topic;
}

void Channel::setTopic(std::string topic)
{
    _topic = topic;
}

int Channel::isOp(std::string nickname)
{
    for (std::map<User *, int>::iterator it = UserBook.begin(); it != UserBook.end(); ++it)
    {
        if (it->first->getNickname() == nickname)
            return it->second;
    }
    return 0;
}

void Channel::changeOp(std::string nickname, int op)
{
    for (std::map<User *, int>::iterator it = UserBook.begin(); it != UserBook.end(); ++it)
    {
        if (it->first->getNickname() == nickname)
            it->second = op;
    }
}

void Channel::SetUserLimit(int limit)
{
    _userLimit = limit;
}

int Channel::getNbUser()
{
    return UserBook.size();
}

bool Channel::isPlace()
{
    return _userLimit == 0 || UserBook.size() < (unsigned int)_userLimit;
}

void Channel::addUserInvite(User *user)
{
    UserInvite.push_back(user);
}

bool Channel::IsInvite(User *user)
{
    return std::find(UserInvite.begin(), UserInvite.end(), user) != UserInvite.end();
}

std::vector<std::string> Channel::getUserNicknames()
{
    std::vector<std::string> res;
    for (std::map<User *, int>::iterator it = UserBook.begin(); it != UserBook.end(); ++it)
        res.push_back(it->first->getNickname());
    return res;
}

void Channel::setInviteOnly(bool status)
{
    _modeI = status;
}

bool Channel::isInviteOnly()
{
    return _modeI;
}

bool Channel::isTopicRestricted()
{
    return _topicRestricted;
}

void Channel::setTopicRestricted(bool status)
{
    _topicRestricted = status;
}

int Channel::getUserLimite()
{
    return _userLimit;
}
