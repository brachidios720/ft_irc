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

    std::string mdp = exctracteMdp(message);

    if(mdp.empty() || channel->getPassword() != mdp){
        std::string mpdError = "ERROR :invalid password\r\n";
        send(user->getSocket(), mpdError.c_str(), mpdError.length(), 0);
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