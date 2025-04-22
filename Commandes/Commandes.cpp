/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commandes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tlegendr <tlegendr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 17:44:33 by hehuang           #+#    #+#             */
/*   Updated: 2025/04/22 15:27:26 by tlegendr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Includes/Server.hpp"
#include <string>

void	Server::CommandJOIN(User *user, std::string &message)
{
	int isFirst = 0;
	std::cout << "JOIN received | message : " << message << std::endl;
	std::stringstream ss(message);
	std::string canal, mdp;
	ss >> canal;
	ss >> mdp;

	
	if (canal.empty()){
		std::cout << "DEBUG: canal = " << canal << " ERROR: canal name empty" << std::endl;
		std::string empty = ":server 461 JOIN :Not enough parameters\r\n";
		send(user->getSocket(), empty.c_str(), empty.length(), 0);
		return;
	}
	if (canal.length() == 1)
	{
		std::cout << "DEBUG: canal = " << canal << " ERROR: canal name too short" << std::endl;
		std::string toShort = ":server 403 " + user->getNickname() + " " + canal + " :No such channel\r\n";
		send(user->getSocket(), toShort.c_str(), toShort.length(), 0);
		return;
	}
	if (canal[0] != '#' && canal[0] != '&') {
		std::cout << "DEBUG: canal = " << canal << " ERROR: Channel name norme error" << std::endl;
		std::string normErr = ":server 403 " + user->getNickname() + " " + canal + " :No such channel\r\n";
		send(user->getSocket(), normErr.c_str(), normErr.length(), 0);
		return;
	}
	if(canal.length() > 32){
		std::cout << "DEBUG: canal = " << canal << " ERROR: canal name too long" << std::endl;
		std::string toHigh = "ERROR : Channel name too long\r\n";
		send(user->getSocket(), toHigh.c_str(), toHigh.length(), 0);
		return;
	}

	std::cout << "DEBUG: canal = " << canal << std::endl;

	Channel *channel = FindChannel(canal);
	if(!channel){
		std::string create = "Channel " + canal + " created\r\n";
		std::cout << create << std::endl;
		channel = new Channel(canal);
		ChannelTab[canal] = channel;
		isFirst = 1;
	}
	if (channel->isInviteOnly() && !channel->IsInvite(user)) {
		std::string err = ":server 473 " + user->getNickname() + " " + channel->getName() + " :Cannot join channel (+i)\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(channel->getMode('k')){
		if(mdp.empty() || channel->getPassword() != mdp){
			std::string mpdError = ":server 475 " + user->getNickname() + " " + canal + " :Invalid password\r\n";
			send(user->getSocket(), mpdError.c_str(), mpdError.length(), 0);
			return;
		}
	}
	if (channel->isPlace() == false)
	{
		std::string err = ":server 471 " + user->getNickname() + " " + canal + " :Cannot join channel (+l)\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::string reponse = "Welcome to the channel " + canal + "\r\n";
	send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
	user->setChannel(canal);
	channel->AddUser(user, mdp, isFirst);

	std::string valid = user->getNickname() + " JOIN " + canal + "\r\n";
	send(user->getSocket(), valid.c_str(), valid.length(), 0); 
	std::string reponse2 = ":" + user->getFullMask() + " JOIN " + canal + "\r\n";
	channel->SendMsg(user, reponse2);
	std::map<User*, int> users = channel->getUsers();
	std::string name_list = ":server 353 " + user->getNickname() + " = " + canal + " :";
	for (std::map<User*, int>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->first != user)
		{
			if (channel->isOp(it->first))
				name_list += "@" + it->first->getFullMask() + " ";
			else
				name_list += it->first->getFullMask() + " ";
		}
	}
	name_list += "\r\n";
	send(user->getSocket(), name_list.c_str(), name_list.length(), 0);
	std::string end_list = ":server 366 " + user->getNickname() + " " + canal + " :End of NAMES list\r\n";
	std::cout << "DEBUG: canal = " << canal << " JOINED" << std::endl;
	if (channel->getTopic().empty())
	{
		std::string topic = ":server 331 " + user->getNickname() + " " + canal + " :No topic is set\r\n";
		send(user->getSocket(), topic.c_str(), topic.length(), 0);
	}
	else
	{
		std::string topic = ":server 332 " + user->getNickname() + " " + canal + channel->getTopic() + "\r\n";
		send(user->getSocket(), topic.c_str(), topic.length(), 0);
	}
};

void Server::CommandPART(User* user, std::string& message)
{
    std::stringstream ss(message);
    std::string channelname;
    ss >> channelname;

    std::string optionalMsg;
    std::getline(ss, optionalMsg); // get rest of the message
    if (!optionalMsg.empty() && optionalMsg[0] == ':')
        optionalMsg = optionalMsg.substr(1); // remove leading colon

    if (channelname.empty())
    {
        std::string err = ":" + user->getFullMask() + " 461 PART :Not enough parameters\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    Channel* channel = FindChannel(channelname);
    if (!channel)
    {
        std::string err = ":" + user->getFullMask() + " 403 " + channelname + " :No such channel\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    if (!channel->IsHere(user))
    {
        std::string err = ":" + user->getFullMask() + " 442 " + channelname + " :You're not on that channel\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    // Compose the PART message with optional message
    std::string partMsg = ":" + user->getFullMask() + " PART " + channelname;
    if (!optionalMsg.empty())
        partMsg += " :" + optionalMsg;
    partMsg += "\r\n";

    // Broadcast to others
    channel->broadcast(partMsg);

    // Remove user from the channel
    channel->DelUser(user);

    // If the channel is now empty, delete it
    if (channel->getUsers().empty())
    {
        ChannelTab.erase(channelname);
        delete channel;
    }
	//std::string confirmationMsg = ":" + user->getFullMask() + " PART " + channelname + " :You have left the channel\r\n";
    //send(user->getSocket(), confirmationMsg.c_str(), confirmationMsg.length(), 0);
}

/*void	Server::CommandMODE(User *user, std::string &message)
{
(void) user;
	std::string msg = "RECEIVED "+ message + "\r\n";
	send(user->getSocket(), msg.c_str(), msg.length(), 0);
};*/


void    Server::CommandPING(User *user, std::string &message)
{
	if (message.empty())
	{
		std::string err = ":server 461 PING :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	else if (message[0] == ':')
		message = message.substr(1);
	std::string server = message.substr(1);
	if (server.empty())
	{
		std::string err = ":server 461 PING :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::cout << "PING received | server : '" << server << "'" << std::endl;
	std::string pong = ":server PONG " + server + "\r\n";
	send(user->getSocket(), pong.c_str(), pong.length(), 0);
}

void    Server::CommandNICK(User *user, std::string &message){

	if (message.empty())
	{
		std::string err = ":server 431 " + user->getNickname() + " :No nickname given\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::string nickname = message.substr(1);
	std::cout << "NICK received | nickname : '" << nickname << "'" << std::endl;
	
	if(nickname.empty()){
		std::cout << "empty nick" << std::endl;
		std::string errorMessage = ":server 431 " + user->getNickname() + " :No nickname given\r\n";
		std::cout << "username = " << nickname << std::endl;
		send(user->getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
		return;
	}

	if(nickname.length() > 9 || nickname.length() < 3 || !isalpha(nickname[0])){
		std::string errorFormat = ":server 432 " + user->getNickname() + " " + nickname + " :Erroneous nickname\r\n";
		send(user->getSocket(), errorFormat.c_str(), errorFormat.length(), 0);
		return;
	}

	if(!isNickAvailable(nickname)){
		std::cout << "nickname already used, nickname = '" << nickname << "'" << std::endl;
		std::string errorNick = ":server 433 * " + nickname + " :Nickname is already in use\r\n";
		send(user->getSocket(), errorNick.c_str(), errorNick.length(), 0);
		return;
	}



	std::string oldNick = user->getNickname();
	if (!oldNick.empty() && oldNick == user->getNickname()) {
		this->nicknameMap.erase(oldNick);
	}
	
	this->nicknameMap[nickname] = user;
	user->setNickname(nickname);
	user->setIsNickSet(true);
	if (!user->getIsRegistered())
	{
		std::cout << "DEBUG: user is not registered, nickname = '" << nickname << "'" << std::endl;
		CanRegister(user);
		//user->setIsRegister(true);
		//return (user->setIsRegister(true));
	}
	std::string success = ":" + oldNick + " NICK :" + nickname + "\r\n";
	send(user->getSocket(), success.c_str(), success.length(), 0);
}

void Server::CanRegister(User *user)
{
	if (user->getIsPassOK() && user->getIsUserSet() && user->getIsNickSet())
	{
		user->setIsRegister(true);
		std::string welcomeMessage = ":" + this->_name + " 001 " + user->getNickname() + " :Welcome to the IRC server, " + user->getNickname() + "!\r\n";
		send(user->getSocket(), welcomeMessage.c_str(), welcomeMessage.length(), 0);
	}
}

void    Server::CommandUSER(User *user, std::string &message){

		if(!user->getUsername().empty()){
			std::string errorRegis = "ERROR : You cannot register\r\n";
			send(user->getSocket(), errorRegis.c_str(), errorRegis.length(), 0);
			return;
		}

		std::stringstream ss(message);
		std::string username, hostname, servername, realname;

		ss >> username >> hostname >> servername >> realname;

		if(username.empty() || hostname.empty() || realname.empty() || servername.empty()){
			std::string emptyEr = "ERROR : too few information (username | hostname | servername | realname)\r\n";
			send(user->getSocket(), emptyEr.c_str(), emptyEr.length(), 0);  
			return;
		}

		user->setHostname(hostname);
		user->setUsername(username);
		user->setRealName(realname);
		user->setIsUserSet(true);
		CanRegister(user);
}

int    Server::CommandPASS(User *user, std::string &message){

	if (message.empty())
	{
		std::string err = ":server 461 PASS :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return 1;
	}
	std::string pass = message.substr(1);
	if(user->getIsRegistered()){
		std::string errorRegis = ":server 462 " + user->getNickname() + " :You may not reregister\r\n";
		send(user->getSocket(), errorRegis.c_str(), errorRegis.length(), 0);
		return 1;
	}

	if(pass.empty()){   
		std::string errorPass = ":server 461 PASS :Not enough parameters\r\n";
		send(user->getSocket(), errorPass.c_str(), errorPass.length(), 0);
		return 1;
	}
	
	if(pass != this->_password){
		std::string badPass = ":server 464 " + user->getNickname() + " :Password incorrect\r\n";
		send(user->getSocket(), badPass.c_str(), badPass.length(), 0);
		return 1;
	}
	user->setIsPassOK(true);
	send(user->getSocket(), "OK\r\n", 4, 0); 
	CanRegister(user);
	return(0);
}

void    Server::CommandCAP(User *user, std::string &code){
	
	if (code.empty())
	{
		std::string err = ":server 461 CAP :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::string subcommand = code.substr(1);

	if(subcommand == "LS"){
		std::string capaciti = "multi-prefix";
		std::string reponse = this->_name + " CAP * LS :\r\n";
		send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
	}
	else if(subcommand == "END"){
		std::string response = "001 " + user->getNickname() + " :" + user->getNickname() + "\r\n";
		send(user->getSocket(), response.c_str(), response.length(), 0);
	}
	else{
		std::string reponse = "ERROR : CAP param not recognized\r\n";
		send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
	}
}

void Server::CommandWHOIS(User* user, std::string& nickname)
{
	if (nickname.empty())
	{
		std::string err = ":server 401 " + user->getNickname() + " :No nickname given\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::string nick = nickname.substr(1);
	std::string response;
	std::map<std::string, User*>::iterator it = this->nicknameMap.find(nick);

	if (it != this->nicknameMap.end())
	{
		User* target = it->second;
		response += ":" + this->_name + " 311 " + user->getNickname() + " " + target->getNickname() + " " +
					target->getUsername() + " " + target->getHostname() + " * " + target->getRealName() + "\r\n";
		response += ":" + this->_name + " 312 " + user->getNickname() + " " + target->getNickname() + " " +
					this->_name + "\r\n";
	}
	else
	{
		response += ":" + this->_name + " 401 " + user->getNickname() + " " + nick + " :No such nick/channel\r\n";
	}

	response += ":" + this->_name + " 318 " + user->getNickname() + " " + nick + " :End of /WHOIS list\r\n";

	send(user->getSocket(), response.c_str(), response.length(), 0);
}



void Server::CommandNAMES(User *user, std::string &message) {
    if (message.empty()) {
        std::string err = ":server 461 " + user->getNickname() + " NAMES :Not enough parameters\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    std::string channelName = message;
	channelName = channelName.substr(1);
	std::cout << "message : " <<channelName << std::endl;
    if (channelName[0] == ':')
        channelName = channelName.substr(1);
    Channel *channel = FindChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 " + user->getNickname() + " " + channelName + " :No such channel\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    std::string response = ":server 353 " + user->getNickname() + " = " + channel->getName() + " :";
    std::vector<std::string> nicknames = channel->getUserNicknames();
    for (std::vector<std::string>::iterator it = nicknames.begin(); it != nicknames.end(); ++it) {
        response += *it + " ";
    }
    response += "\r\n";

    send(user->getSocket(), response.c_str(), response.length(), 0);

    std::string endOfList = ":server 366 " + user->getNickname() + " " + channel->getName() + " :End of /NAMES list.\r\n";
    send(user->getSocket(), endOfList.c_str(), endOfList.length(), 0);
}

void    Server::CommandPRIVMSG(User *user, std::string &message){

	std::stringstream ss(message);
	std::string target, pvt;

	ss >> target;
	if (target.empty())
	{
		std::string err = ":server 411 " + user->getNickname() + " :No recipient given\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::getline(ss, pvt);
	if (pvt.empty())
	{
		std::string err = ":server 412 " + user->getNickname() + " :No text to send\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(target[0] == ':')
		target = target.substr(1);
	if(pvt[0] == ':')
		pvt = pvt.substr(1);

	if(target.empty()){
		std::string err = ":user not found\r\n";
		std::cout << err << std::endl;
		send(user->getSocket(), err.c_str(), err.length(), 0);
	}
	if(pvt.empty()){
		std::string err = ":write something to send\r\n";
		std::cout << err << std::endl;
		send(user->getSocket(), err.c_str(), err.length(), 0);
	}
	std::cout << "DEBUG: target = '"<< target << "'" << std::endl;
	if(target[0] == '#' ){
		Channel *channel = FindChannel(target);
		if(!channel){
			std::string err =  target + " :channel not found\r\n";
			std::cout << err << std::endl;
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		else{
			if(!channel->IsHere(user)){
				std::string err = ":server 404 " + user->getNickname() + " " + target + " :You're not on that channel\r\n";
				std::cout << err << std::endl;
				send(user->getSocket(), err.c_str(), err.length(), 0);
				return;
			}
			std::string reponse = ":" + user->getNickname() + " PRIVMSG " + target + pvt + "\r\n";
			std::cout << reponse << std::endl;
			channel->SendMsg(user, reponse);
		}
	}
	else{
		User *targetUser = nullptr;
		for (std::map<int, User*>::iterator it = UserTab.begin(); it != UserTab.end(); ++it)
		{
			if (it->second->getNickname() == target)
			{
				targetUser = it->second;
				break;
			}
		}
		if(targetUser){
			std::string reponse = ":" + user->getNickname() + " PRIVMSG " + target + pvt + "\r\n";
			send(targetUser->getSocket(), reponse.c_str(), reponse.length(), 0);
		}
		else {
			std::string reponse = target + " not found\r\n";
			std::cout << reponse << std::endl;
			send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
		}
	}

}

void    Server::CommandMODE(User *user, std::string &message){
	
	std::stringstream ss(message);
	std::string target, mode, param;
	ss >> target >> mode >> param;
	if (target.empty() || mode.empty())
	{
		std::cout << "DEBUG: target = '"<< target << "'" << std::endl;
		std::cout << "DEBUG: mode = '"<< mode << "'" << std::endl;
		std::cout << "DEBUG: param = '"<< param << "'" << std::endl;
		std::string err = ":server 461 MODE :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(target[0] == ':')
		target = target.substr(1);
	if(mode[0] == ':')
		mode = mode.substr(1);
	if(target.empty()){
			std::string err = ": no such target\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
	}
	if(mode.empty()){
			std::string err = ": no such mode\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
	}
	if (mode.length() == 2 && (mode[0] == '+' || mode[0] == '-') && (mode[1] == 'o' || mode[1] == 'k' || mode[1] == 'i' || mode[1] == 't' || mode[1] == 'l'))
	{
		std::cout << "DEBUG: Mode = " << mode << std::endl;
	}
	else {
		std::string err = ":server 472 " + user->getNickname() + " " + target + " :Unknown mode\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	std::cout << "DEBUG: Mode size = " << mode.size() << std::endl;

	if(target[0] == '#'){
		Channel *channel = FindChannel(target);
		if(!channel){
			std::string err = ":server 403 " + user->getNickname() + " " + target + " :No such channel\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (!channel->isOp(user)) {
			std::string err = ":server 482 " + user->getNickname() + " " + target + " :You're not channel operator\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (!channel->IsHere(user)){
			std::string err = ":server 442 " + user->getNickname() + " " + target + " :You're not on that channel\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (mode[1] == 'o' && param.empty()){
			std::string err = ":server 461 MODE :Not enough parameters\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (mode[1] == 'o'){
			User *targetUser = getUser(param);
			if (!targetUser){
				std::string err = ":server 401 " + user->getNickname() + " " + param + " :No such nick/channel\r\n";
				send(user->getSocket(), err.c_str(), err.length(), 0);
				return;
			}
			if (mode[0] == '+')
				channel->changeOp(targetUser, 1);
			else
				channel->changeOp(targetUser, 0);
		}
		else if (mode[1] == 'k') {
			channel->SetMode(mode[1], mode[0] == '+');
			if (!param.empty())
				channel->SetPassword(param);
		}
		else if (mode[1] == 'i')
			channel->SetMode(mode[1], mode[0] == '+');
		else if (mode[1] == 't')
			channel->SetMode(mode[1], mode[0] == '+');
		else if (mode[1] == 'l') {
			if (param.empty() && mode[0] == '+'){
				std::string err = ":server 461 MODE :Not enough parameters\r\n";
				send(user->getSocket(), err.c_str(), err.length(), 0);
				return;
			}
			else if (mode[0] == '-'){
				channel->SetMode(mode[1], 0);
				channel->SetUserLimit(0);
				return;
			}
			int limit = std::atoi(param.c_str());
			if (limit < 0){
				std::string err = "ERROR :Limit must be a positive number\r\n";
				send(user->getSocket(), err.c_str(), err.length(), 0);
				return;
			}
			channel->SetMode(mode[1], 1);
			channel->SetUserLimit(limit);
		}
		else {
			std::string err = ":server 472 " + user->getNickname() + " " + target + " :Unknown mode\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
		}
	}	
}

void    Server::CommandTOPIC(User *user, std::string &message){
	std::cout << "DEBUG:: TOPIC received | message : '" << message << "'" << std::endl;
	std::cout << "DEBUG:: user : " + user->getNickname() << std::endl;
	if (message.empty())
	{
		std::string err = ":server 461 TOPIC :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if (message[0] == ':')
		message = message.substr(1);
	std::stringstream ss(message);
	std::string channelName, topic;
	ss >> channelName;
	std::getline(ss, topic);
	if (topic[0] == ':')
		topic = topic.substr(1);
	if (channelName.empty()){
		std::string err = ": no such channel\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	Channel *channel = FindChannel(channelName);
	if (!channel){
		std::string err = ": no channel with the name : " + channelName + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if (!channel->IsHere(user)){
		std::string err = ": you are not a member of the channel : " + channelName + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if (channel->getMode('t') && !channel->isOp(user)) {
		std::cout << "DEBUG:: user : " + user->getNickname() + " is not op for TOPIC action" << std::endl;
		std::string err = ": you are not operator of this channel : " + channelName + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if (topic.empty()) {
		if (!channel->getTopic().empty()) {
			std::string rep = user->getNickname() + " " + channelName + channel->getTopic() + "\r\n";
			send(user->getSocket(), rep.c_str(), rep.length(), 0);
		} else {
			std::string rep = user->getNickname() + " " + channelName + " :No topic is set\r\n";
			send(user->getSocket(), rep.c_str(), rep.length(), 0);
		}
		return;
	}
	channel->setTopic(topic); 
	std::string rep = ": 332 " + user->getNickname() + " " + channelName + topic + "\r\n";
	send(user->getSocket(), rep.c_str(), rep.length(), 0);
	std::string reponse = ":" + user->getNickname() + " TOPIC " + channelName  + topic + "\r\n";
	channel->SendMsg(user, reponse);
}

void Server::CommandINVITE(User *user, std::string &message) {
    std::cout << "[DEBUG] CommandINVITE called with message: " << message << std::endl;

    std::stringstream ss(message);
    std::string nickname, channelname;
    ss >> nickname >> channelname;

    std::cout << "[DEBUG] Parsed nickname: |" << nickname << "|, channel: |" << channelname << "|" << std::endl;

    if (nickname.empty() || channelname.empty()) {
        std::cerr << "[ERROR] Missing nickname or channel name" << std::endl;
        std::string err = ":Invalid parameters\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    Channel *channel = FindChannel(channelname);
    if (!channel) {
        std::cerr << "[ERROR] Channel not found: " << channelname << std::endl;
        std::string err = ":No such channel " + channelname + "\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    if (!channel->IsHere(user)) {
        std::cerr << "[ERROR] User not in channel: " << user->getNickname() << std::endl;
        std::string err = ":You must be in the channel to invite someone\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    if (channel->getMode('i') && !channel->isOp(user)) {
        std::cerr << "[ERROR] Channel is invite-only and user is not operator: " << user->getNickname() << std::endl;
        std::string err = ":You're not channel operator\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    User *targetUser = getUser(nickname);
    if (!targetUser) {
        std::cerr << "[ERROR] Target user not found: " << nickname << std::endl;
        std::string err = ":User not found\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    channel->addUserInvite(targetUser);
    std::cout << "[DEBUG] User " << targetUser->getNickname() << " added to invite list of channel " << channelname << std::endl;

    std::string inviteMsg = ":" + user->getFullMask() + " INVITE " + targetUser->getNickname() + " :" + channelname + "\r\n";
    send(targetUser->getSocket(), inviteMsg.c_str(), inviteMsg.length(), 0);
    std::cout << "[DEBUG] Sent invite to " << targetUser->getNickname() << std::endl;

    std::string confirm = ":You invited " + nickname + " to " + channelname + "\r\n";
    send(user->getSocket(), confirm.c_str(), confirm.length(), 0);
    std::cout << "[DEBUG] Sent confirmation to inviter: " << user->getNickname() << std::endl;
}

void    Server::CommandKICK(User *user, std::string &message){
	std::stringstream ss(message);
	std::string channelname, nickname;
	ss >> channelname >> nickname;
	if (channelname.empty() || nickname.empty())
	{
		std::string err = ":server 461 KICK :Not enough parameters\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(channelname[0] == ':')
		channelname = channelname.substr(1);
	if(nickname[0] == ':')
		nickname = nickname.substr(1);
	
	if(channelname.empty()){
		std::string err = ": no such channel\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(nickname.empty()){
		std::string err = ": no such nickname\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}

	Channel *channel = FindChannel(channelname);
	if(!channel){
		std::string err = ": no channel with the name : " +channelname + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(!channel->IsHere(user)){
		std::string err = ": you are not a menber of the channel :" +channelname + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(!channel->isOp(user)){
		std::string err = ": you are not operator of this channel : " + channelname + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	User *targetUser = nullptr;
	for (std::map<int, User*>::iterator it = UserTab.begin(); it != UserTab.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
		{
			targetUser = it->second;
			break;
		}
	}
	if(!targetUser){
		std::string err = ":user not found" + nickname + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	if(!channel->IsHere(targetUser)){
		std::string err =  ": " + targetUser->getNickname() + "is not a member of the channel :" +channelname + "\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}

	channel->DelUser(targetUser);
	targetUser->setChannel("");
	std::string rep = ":" + user->getNickname() + " KICK " + channelname + " " + targetUser->getNickname() + "\r\n";
	send(user->getSocket(), rep.c_str(), rep.length(), 0);
	std::string rep2 = ":" + user->getNickname() + " KICK " + channelname + " " + targetUser->getNickname() + "\r\n";
	send(targetUser->getSocket(), rep2.c_str(), rep2.length(), 0);
}

void Server::CommandQUIT(User *user, std::string &message) {
    std::string quitMsg = "Client Quit";

    // Correct parsing of the message
    if (!message.empty()) {
        if (message[0] == ':')
            quitMsg = message.substr(1);
        else
            quitMsg = message;
    }

    std::string disconnectMsg = ":" + user->getFullMask() + " QUIT :" + quitMsg + "\r\n";

    // Notify all channels the user is part of
    for (std::map<std::string, Channel*>::iterator it = ChannelTab.begin(); it != ChannelTab.end(); ++it) {
        Channel* channel = it->second;

        if (channel->IsHere(user)) {
            channel->SendMsg(user, disconnectMsg); // send QUIT msg to other users
            channel->DelUser(user);                // remove user from the channel
        }
    }

    // Send QUIT to the user themselves (optional)
    send(user->getSocket(), disconnectMsg.c_str(), disconnectMsg.length(), 0);

    // Close and remove user
	for (size_t i = 1; i < poll_fds->size(); i++){
		if (poll_fds->at(i).fd == user->getSocket()){
			poll_fds->erase(poll_fds->begin() + i);
			break;
		}
	}
    close(user->getSocket());
	UserTab.erase(user->getSocket());
	nicknameMap.erase(user->getNickname());
    delete(user);
	
}