#include "../Includes/Server.hpp"


void    Server::CommandPING(User *user, std::string message){
    std::string server = message.substr(5);
    std::string pong = "PONG" + server + "\r\n";
    send(user->getSocket(), pong.c_str(), pong.length(), 0);
}


void    Server::CommandNICK(User *user, std::string message){

    std::string nickname = message.substr(5);

    if(nickname.empty()){
        std::string errorMessage = "ERROR :Nickname cannot be empty\r\n";
        send(user->getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
        return;
    }

    if(nickname.length() > 9 || nickname.length() < 3 || !isalpha(nickname[0])){
        std::string errorFormat = "ERROR :Nickname have norme error\r\n";
        send(user->getSocket(), errorFormat.c_str(), errorFormat.length(), 0);
        return;
    }

    if(!isNickAvailable(nickname)){
        std::string errorDouble = "ERROR :Nickname is already use\r\n";
        send(user->getSocket(), errorDouble.c_str(), errorDouble.length(), 0);
        return;
    }
    
    user->setNickname(nickname);
    std::string sucess = "NICK :" + nickname + "\r\n";
    send(user->getSocket(), sucess.c_str(), sucess.length(), 0);
}

void    Server::CommandJOIN(User *user, std::string message){

    std::string canal = exctracteChannelName(message);

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
        channel = new Channel(canal);
        ChannelTab[canal] = channel;
    }
    if(channel->isInviteOnly() && !channel->IsInvite(user)){
        std::string err = "ERROR : you are not invite to join this channel\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
    }
    if(!channel->getPassword().empty()){
        std::string mdp = exctracteMdp(message);

        if(mdp.empty() || channel->getPassword() != mdp){
            std::string mpdError = "ERROR :invalid password\r\n";
            send(user->getSocket(), mpdError.c_str(), mpdError.length(), 0);
            return;
        }
    }
    user->setChannel(canal);
    channel->AddUser(user, mdp, 0);

    std::string valid = user->getNickname() + " JOIN " + canal + "\r\n";
    send(user->getSocket(), valid.c_str(), valid.length(), 0); 
}

void    Server::CommandUSER(User *user, std::string message){

        if(!user->getUsername().empty()){
            std::string errorRegis = "ERROR : You cannot reregister\r\n";
            send(user->getSocket(), errorRegis.c_str(), errorRegis.empty(), 0);
            return;
        }

        std::stringstream ss(message);
        std::string command, username, hostname, servername, realname;

        ss >> command;
        ss >> username >> hostname >> servername >> realname;

        if(username.empty() || hostname.empty() || realname.empty() || servername.empty()){
            std::string emptyEr = "ERROR : to few information\r\n";
            send(user->getSocket(), emptyEr.c_str(), emptyEr.length(), 0);  
            return;
        }

        user->setUsername(hostname);
        user->setUsername(username);

        std::string reponse = "Welcome to IRC " + user->getNickname() + username + "\r\n";
        send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
}

int    Server::CommandPASS(User *user, std::string pass){
        
        if(!user->getUsername().empty()){
            std::string errorRegis = "ERROR : You cannot reregister\r\n";
            send(user->getSocket(), errorRegis.c_str(), errorRegis.empty(), 0);
            return 0;
        }
        if(pass.empty()){   
            std::string errorPass = "ERROR : pass is empty\r\n";
            send(user->getSocket(), errorPass.c_str(), errorPass.length(), 0);
            return 0;
        }
        if(pass != _password){
            std::string badPass = "ERROR : bad password\r\n";
            send(user->getSocket(), badPass.c_str(), badPass.length(), 0);
            return 0;
        }

        user->setIsRegister(true);
        return(1);
}

void    Server::CommandCAP(User *user){

    std::string command = user->getbuffCommand();
    
    std::stringstream ss(command);
    std::string subcommand, params;

    ss >> subcommand;

    std::getline(ss, params);
    if(!params.empty()){
        params = params.substr(1);
    } 
    if(subcommand == "LS"){
        std::string capaciti = "multi-prefix";
        std::string reponse = ":server CAP * LS : " + capaciti + "\r\n";
        send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
    }
    else if(subcommand == "REQ"){
        
        if(params == "multiprefix"){
            std::string reponse = ":server CAP * ACK : " + params + "\r\n";
            send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
        }
        else{
            std::string reponse = ":server CAP * NAK : " + params + "\r\n";
            send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
        }
    }
    else if(subcommand == "ACK"){
        std::string reponse = ":server CAP * ACK : " + params + "\r\n";
        send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
    }
    else if(subcommand == "END"){
        return;
    }
    else{
        std::string reponse = "command not found\r\n";
        send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
    }
}

void    Server::CommandNAMES(User *user, Channel *channel){
    
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

void    Server::CommandPRIVMSG(User *user, std::string message){

    std::stringstream ss(message);
    std::string target, pvt;

    ss << target;
    getline(ss, pvt);

    if(target.empty()){
        std::string err = ":user not found\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
    }
    if(pvt.empty()){
        std::string err = ":write something to send\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
    }
    if(target[0] == '#' ){
        Channel *channel = FindChannel(target);
        if(!channel){
            std::string err =  target + " :channel not found\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        else{
            std::string reponse = ":" + user->getNickname() + " send PRVTMSG " + target + " :" + pvt.substr(1) + "\r\n" ;
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
            std::string reponse = ":" + user->getNickname() + " send PRVTMSG " + target + " :" + pvt.substr(1) + "\r\n"; 
            send(targetUser->getSocket(), reponse.c_str(), reponse.length(), 0);
        }
        else {
            std::string reponse = target + " not found\r\n";
            send(user->getSocket(), reponse.c_str(), reponse.length(), 0);
        }
    }

}

void    Server::CommandPART(User *user, std::string message){

    std::stringstream ss(message);
    std::string  channelName;

    ss << channelName;

    if(channelName.empty()){
        std::string rep = ": empty channel name\r\n";
        send(user->getSocket(), rep.c_str(), rep.length(), 0);
        return;
    }

    Channel *channel  = FindChannel(channelName);
    if(!channel){
        std::string rep = ": no such channel\r\n";
        send(user->getSocket(), rep.c_str(), rep.length(), 0);
        return;
    }

    if(!channel->IsHere(user)){
        std::string rep = ": your not in this channel\r\n";
        send(user->getSocket(), rep.c_str(), rep.length(), 0);
        return;
    }

    channel->DelUser(user);
    user->setChannel("");

    std::string rep = ": " + user->getNickname() + " PART " + channelName + "\r\n";
    send(user->getSocket(), rep.c_str(), rep.length(), 0);
}

void    Server::CommandMODE(User *user, std::string message){
    std::stringstream ss(message);
    std::string target, mode;

    ss << target << mode;

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
        if(mode == "+k" || mode == "-k")
            ModeK(user, channel, "", (mode == "+k" ? 1 : 0));
        else if (mode == "+i" || mode == "-i")
            ModeI(user, channel, (mode == "+i" ? 1 : 0));
        else if(mode == "+o" || mode == "-o")
            ModeO(user, channel, "", (mode == "+o" ? 1 : 0));
        else if(mode == "+t" || mode == "-t")
            ModeT(user, channel, (mode == "+t" ? 1 : 0));
        else if(mode == "+l" || mode == "-l")
            ModeL(user, channel, "", (mode == "+l" ? 1 : 0));
    }
    else{
        if(mode == "+o" || mode == "-o")
            ModeO(user, nullptr, "", (mode == "+o" ? 1 : 0));
    }
}

void    Server::CommandTOPIC(User *user, std::string message){
        std::stringstream ss(message);
        std::string channelname , newtopic;

        ss << channelname << newtopic;

        if(channelname.empty()){
            std::string err = ": no such channel\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }

        Channel *channel = FindChannel(channelname);
        if(!channel){
            std::string err = ": no channel with the name : " + channelname + "\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        if(channel->isTopicRestricted() == true){
            std::string err = ": the topic is restricted, you can't set topic\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        if(newtopic.empty()){

            std::string topic = channel->getTopic();
            if(topic.empty()){
                std::string rep = channelname + " : don't have currently topic";
                send(user->getSocket(), rep.c_str(), rep.length(), 0);
            }
            else {
                std::string rep = channelname + " :" + topic + "\r\n";
                send(user->getSocket(), rep.c_str(), rep.length(), 0);
            }
        }
        else {
            if(!channel->isOp(user->getNickname()))
            {
                std::string rep = ": you don't have the permission to creat a topic\r\n";
                send(user->getSocket(), rep.c_str(), rep.length(), 0);
                return;
            }
            else{
                channel->setTopic(newtopic);
                std::string rep = ":" + user->getNickname() + " set a new Topic for the " + channelname + ": " + newtopic.substr(1) + "\r\n";
                channel->SendMsg(user, rep);
            }
        }
}

void    Server::CommandINVITE(User *user, std::string message){

        std::stringstream ss(message);
        std::string nickname, channelname;
        ss << nickname << channelname;

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
        if(channel->getUserLimite() != -1 && channel->getNbUser() == channel->getUserLimite()){
            std::string err = ": you can't invite this user because the userlimite has been reached\r\n";
            send(user->getSocket, err.c_str(), err.length(), 0);
            return;
        }
        if(channel->IsHere(user)){
            std::string err = ": not valid invitation\r\n";
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
        if((channel->getMode('i') == true) && !channel->isOp(user->getNickname())){
            std::string err = ": you are not operator of this channel : " + channelname + "\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }

        channel->addUserInvite(targetUser);
        std::string rep = ": " + user->getNickname() + " invite you to join the " + channelname + " channel " + "\r\n";
        send(targetUser->getSocket(), rep.c_str(), rep.length(), 0);
        std::string repp = ": " + nickname + " has been received the invitation to the channel : " + channelname + "\r\n"; 
        send(user->getSocket(), repp.c_str(), repp.length(), 0);
}

void    Server::CommandKICK(User *user, std::string message){
    std::stringstream ss(message);
    std::string channelname, nickname;
    ss << channelname << nickname;

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
    if(!channel->isOp(user->getNickname())){
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
        std::string err =  ": " + targetUser->getNickname() + "is not a menber of the channel :" +channelname + "\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }

    channel->DelUser(targetUser);

    std::string rep = ": the user " + targetUser->getNickname() + " has been kicked of the channel by " + user->getNickname() + "\r\n";
    channel->SendMsg(user, rep);
    send(targetUser->getSocket(), rep.c_str(), rep.length(), 0);
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


void 	Server::ModeK(User *user, Channel *channel, std::string message, int i){
    if(i == 1){
        if(message.empty()){
            std::string err = "Error: Pasword required\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        channel->SetPassword(message);
        std::string rep = ": password set\r\n";
        send(user->getSocket(), rep.c_str(), rep.lenght(), 0);
    }
    else{
        channel->SetPassword("");
        std::string rep = ": password unset\r\n";
        send(user->getSocket(), rep.c_str(), rep.length(), 0);
    }
}

void 	Server::ModeI(User *user, Channel *channel, int i){
    if(i == 1){
        channel->setInviteOnly(true)
        std::string mess = "invite only mode activate\r\n";
        send(user->getSocket(), mess.c_str(), mess.length(), 0);
    }
    else{
        channel->setInviteOnly(false);
        std::string mess = "invite only mode desacative\r\n";
        send(user->getSocket(), mess.c_str(), mess.lenght(), 0);
    }
}

void 	Server::ModeO(User *user, Channel *channel, std::string message, int i){
    std::stringstream ss(message);
    std::string mode, channelPrint, mode, targetUser;
    
    ss << mode << channelPrint << mode << targetUser;

    if(channelPrint.empty() || !mode.empty() || !targetUser.empty()){
        std::string err = "ERROR: part of the message is incomplet\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }
    if(!channel.FindChannel(channelPrint)){
        std::string err = "ERROR: no such channel\r\n";
        send(user->getSocket(), err.c_str(), err.lenght(), 0);
        return;
    }
    if(!channel->isOp(user)){
        std::string err = "ERROR: you are not operator\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }
    user *target = channel->getName(targetUser);
    if(!channel->getName(targetUser)){
        std::string err = "ERROR: this user in already not in this channel\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }
    if(i == 1){
        if(channel->isOp(targetUser)){
            std::string err = "ERROR: this target user is already operator\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        channel->changeOp(targetUser, 1);
        std::string mess = ": " + targetUser + " is now already operator of this channel\r\n"; 
        channel->SendMsg(user, mess);
    }
    else{
        if(!channel->isOp(targetUser)){
            std::string err = "ERROR: this user is not an operator\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        channel->changeOp(targetUser, 0);
        std::string mess = ": " + targetUser + " is no longer admin\r\n";
        channel->SendMsg(user, mess);
    }
}


void 	Server::ModeT(User *user, Channel *channel, int i){

    if(!channel->isOp(user->getGetNick)){
        std::string err = "ERROR: you are not an operator in this channel\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }
    if(i == 1){
        if(channel->isTopicRestricted()){
            std::string err = " ERROR: channel is already topic restricted\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        std::string mess = ": the Topic is now restricted\r\n";
        channel->setTopicRestricted(true);
        channel->SendMsg(user, mess);
    }
    else{
        if(!channel->isTopicRestricted()){
            std::string err = "ERROR: the channel is already not topic restricted\r\n";
            send(user->getSochet(), err.c_str(), err.length(), 0);
            return;
        }
        std::string mess = ": the Topic is now unrestricted\r\n";
        channel->setTopicRestricted(false);
        channel->SendMsg(user, mess);
    }
}

void 	Server::ModeL(User *user, Channel *channel, std::string message, int i){

    if(!channel->isOp(user)){
        std::string err = "ERROR: you are not a channel operator\r\n";
        send(user->getSocket(), err.c_str(), err.length(), 0);
        return;
    }
    if(i == 1){
        std::istringstream iss(message);
        int limit;
        iss >> limit;

        if(limit <= 0){
            std::string err = "ERROR: no limite number\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        std::string mess = "the actualy number limite is now " + limte + "\r\n";
        channel->SetUserLimit(limit);
        channel->SendMsg(user, mess);
    }
    else
        if(channel->getUserLimite() == -1){
            std::string err = "ERROR: the userlimite is already unset\r\n";
            send(user->getSocket(), err.c_str(), err.length(), 0);
            return;
        }
        std::string mess = "there is no actualy userLimite\r\n";
        channel->SetUserLimit(-1);
        channel->SendMsg(user, mess);
}