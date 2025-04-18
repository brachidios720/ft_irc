/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commandes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tlegendr <tlegendr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 17:44:33 by hehuang           #+#    #+#             */
/*   Updated: 2025/04/18 21:23:57 by tlegendr         ###   ########.fr       */
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

	if(canal[0] != '#' && canal[0] != '&'){
		std::cout << "DEBUG: canal = " << canal << " ERROR: Channel name norme error" << std::endl;
		std::string normErr = "ERROR :Channel name norme error\r\n";
		send(user->getSocket(), normErr.c_str(), normErr.length(), 0);
		return;
	}
	if(canal.empty()){
		std::cout << "DEBUG: canal = " << canal << " ERROR: canal name empty" << std::endl;
		std::string empty = "ERROR :Channel cannot be empty\r\n";
		send(user->getSocket(), empty.c_str(), empty.length(), 0);
		return;
	}
	if(canal.length() > 32){
		std::cout << "DEBUG: canal = " << canal << " ERROR: canal name too long" << std::endl;
		std::string toHigh = "ERROR :Channel size to big\r\n";
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
	if(!channel->getPassword().empty()){
		if(mdp.empty() || channel->getPassword() != mdp){
			std::string mpdError = ":server 475 " + user->getNickname() + " " + canal + " :Invalid password\r\n";
			send(user->getSocket(), mpdError.c_str(), mpdError.length(), 0);
			return;
		}
	}
	std::string reponse = "Welcome to the channel " + canal + "\r\n";
	send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
	user->setChannel(canal);
	channel->AddUser(user, mdp, isFirst);

	std::string valid = user->getNickname() + " JOIN " + canal + "\r\n";
	send(user->getSocket(), valid.c_str(), valid.length(), 0); 
	std::string reponse2 = ":" + user->getNickname() + " JOIN " + canal + "\r\n";
	channel->SendMsg(user, reponse2);
	std::map<User*, int> users = channel->getUsers();
	std::string name_list = ":server 353 " + user->getNickname() + " = " + canal + " :";
	for (std::map<User*, int>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->first != user)
		{
			if (channel->isOp(it->first))
				name_list += "@" + it->first->getNickname() + " ";
			else
				name_list += it->first->getNickname() + " ";
		}
	}
	name_list += "\r\n";
	send(user->getSocket(), name_list.c_str(), name_list.length(), 0);
	std::string end_list = ":server 366 " + user->getNickname() + " " + canal + " :End of NAMES list\r\n";
	std::cout << "DEBUG: canal = " << canal << " JOINED" << std::endl;
	if (channel->getTopic().empty())
	{
		std::string topic = "No topic set\r\n";
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
	std::string server = message.substr(5);
	std::string pong = ":PONG " + message + "\r\n";
	send(user->getSocket(), pong.c_str(), pong.length(), 0);
}

void    Server::CommandNICK(User *user, std::string &message){

	std::string nickname = message.substr(1);
	std::cout << "NICK received | nickname : '" << nickname << "'" << std::endl;
	
	if(nickname.empty()){
		std::cout << "empty nick" << std::endl;
		std::string errorMessage = "ERROR :Nickname cannot be empty\r\n";
		std::cout << "username = " << nickname << std::endl;
		send(user->getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
		return;
	}

	if(nickname.length() > 9 || nickname.length() < 3 || !isalpha(nickname[0])){
		std::string errorFormat = "ERROR :Nickname have norme error(start with letter and size between 3 and 9)\r\n";
		send(user->getSocket(), errorFormat.c_str(), errorFormat.length(), 0);
		return;
	}

	if(!isNickAvailable(nickname)){
		std::cout << "nickname already used, nickname = '" << nickname << "'" << std::endl;
		std::string errorNick = ":server 433 * " + nickname + " :Nickname is already in use\r\n";
		send(user->getSocket(), errorNick.c_str(), errorNick.length(), 0);
		return;
	}

	if (!user->getIsRegistered())
	{
		std::cout << "DEBUG: user is not registered, nickname = '" << nickname << "'" << std::endl;
		std::string sucess = this->_name + " 001 " + nickname + " :Welcome to the IRC server, " + nickname + "!\r\n";
		send(user->getSocket(), sucess.c_str(), sucess.length(), 0);
		user->setIsRegister(true);
		//return (user->setIsRegister(true));
	}

	std::string oldNick = user->getNickname();
	if (!oldNick.empty() && oldNick == user->getNickname()) {
		this->nicknameMap.erase(oldNick);
	}
	
	this->nicknameMap[nickname] = user;
	user->setNickname(nickname);
	std::string sucess = ":" + oldNick + " NICK :" + nickname + "\r\n";
	send(user->getSocket(), sucess.c_str(), sucess.length(), 0);
}


/*void    Server::CommandJOIN(User *user, std::string &message){

	std::string canal = message.substr(1);

	if(canal[0] != '#' || canal[0] != '&'){
		std::string normErr = "ERROR :Chanel name norme error\r\n";
		send(user->getSocket(), normErr.c_str(), normErr.length(), 0);
		return;
	}
	if(canal.empty()){
		std::string empty = "ERROR :Channel cannot be empty\r\n";
		send(user->getSocket(), empty.c_str(), empty.length(), 0);
		return;
	}
	if(canal.length() > 32){
		std::string toHigh = "ERROR :Channel size to big\r\n";
		send(user->getSocket(), toHigh.c_str(), toHigh.length(), 0);
		return;
	}

	Channel *channel = FindChannel(canal);
	if(!channel){
		std::string create = "Channel " + canal + " created\r\n";
		std::cout << create << std::endl;
		channel = new Channel(canal);
		ChannelTab[canal] = channel;
	}
	if(channel->isInviteOnly() && !channel->IsInvite(user)){
		std::string err = "ERROR : you are not invite to join this channel\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
	}
	std::string mdp = extractMdp(message);
	if(!channel->getPassword().empty()){
		if(mdp.empty() || channel->getPassword() != mdp){
			std::string mpdError = "ERROR :invalid password\r\n";
			send(user->getSocket(), mpdError.c_str(), mpdError.length(), 0);
			return;
		}
	}
	std::string reponse = "Welcome to the channel " + canal + "\r\n";
	send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
	user->setChannel(canal);
	channel->AddUser(user, mdp, 0);

	std::string valid = user->getNickname() + " JOIN " + canal + "\r\n";
	send(user->getSocket(), valid.c_str(), valid.length(), 0); 
}*/

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

		std::string reponse = "Welcome to IRC " + user->getNickname() + " " + username + "\r\n";
		send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
}

int    Server::CommandPASS(User *user, std::string &message){
	

	std::string pass = message.substr(1);
	if(!user->getUsername().empty()){
		std::string errorRegis = "ERROR : You're already on our server\r\n";
		send(user->getSocket(), errorRegis.c_str(), errorRegis.length(), 0);
		return 1;
	}

	if(pass.empty()){   
		std::string errorPass = "ERROR : pass is empty\r\n";
		send(user->getSocket(), errorPass.c_str(), errorPass.length(), 0);
		return 1;
	}
	if(pass != this->_password){
		std::string badPass = "ERROR : bad password\r\n";
		send(user->getSocket(), badPass.c_str(), badPass.length(), 0);
		return 1;
	}

	send(user->getSocket(), "OK\r\n", 4, 0); // PEUT ETRE DELETE OU MODIFIER
	return(0);
}

void    Server::CommandCAP(User *user, std::string &code){
	
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



void    Server::CommandNAMES(User *user, std::string &message){
	std::string channelName = message.substr(1);
	if(channelName[0] == ':')
		channelName = channelName.substr(1);
	if(channelName.empty()){
		std::string err = ": no such channel\r\n";
		send(user->getSocket(), err.c_str(), err.length(), 0);
		return;
	}
	Channel *channel = FindChannel(channelName);
	if(!channel){
		std::string reponse = ":channel not found\r\n";
		send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
		return;
	}

	std::string reponse = ": " + channel->getName() + " user : ";
	std::vector<std::string> nicknames = channel->getUserNicknames();
	for(std::vector<std::string>::iterator it = nicknames.begin(); it != nicknames.end(); ++it){
		reponse += *it + " ";
	}
	reponse = "\r\n";

	send(user->getSocket(), reponse.c_str(), reponse.length(), 0);

	std::string listefinish = channel->getName() + "end of the liste\r\n";
	send(user->getSocket(), listefinish.c_str(), listefinish.length(), 0);
}

void    Server::CommandPRIVMSG(User *user, std::string &message){

	std::stringstream ss(message);
	std::string target, pvt;

	ss >> target;
	std::getline(ss, pvt);
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
				std::string err = ":" + user->getNickname() + "ERROR : you are not in this channel\r\n";
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

	if(target[0] == '#'){
		Channel *channel = FindChannel(target);
		if(!channel){
			std::string err = ": no such channel\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (!channel->isOp(user)) {
			std::string err = ": you are not operator of this channel\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (!channel->IsHere(user)){
			std::string err = ": you are not in this channel\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (mode[1] == 'o' && param.empty()){
			std::string err = ": no such param\r\n";
			send(user->getSocket(), err.c_str(), err.length(), 0);
			return;
		}
		if (mode[1] == 'o'){
			User *targetUser = getUser(param);
			if (!targetUser){
				std::string err = ": no such user\r\n";
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
			channel->SetPassword(param);
		}
		else if (mode[1] == 'i')
			channel->SetMode(mode[1], mode[0] == '+');
		else if (mode[1] == 't')
			channel->SetMode(mode[1], mode[0] == '+');
	}	
}

void    Server::CommandTOPIC(User *user, std::string &message){
	std::cout << "DEBUG:: TOPIC received | message : '" << message << "'" << std::endl;
	std::cout << "DEBUG:: user : " + user->getNickname() << std::endl;
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

// void    Server::CommandQUIT(User *user, std::string message){

//     std::stringstream ss(message);
//     std::string mess;
//     ss << mess;

//     std::string quitmess = "client quit";
//     if(mess.empty()){
//         quitmess = mess.substr(1);
//     }

//     std::string disconnectMess = ":" + user->getGetNick() + " QUIT " + quitmess + "\r\n";

//     for(std::map<std::string, Channel*>::iterator it = ChannelTab.begin();it != ChannelTab.end(); ++it){
//         Channel *channel = it->second;

//         if(channel->IsHere(user)){
//             channel->SendMsg(user, disconnectMess);
//             channel->DelUser(user);
//         }
//     }

//     send(user->getSocket, disconnectMess.c_str(), disconnectMess.lenght(), 0);
//     RemoveUser(user->)


// }