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
    if(canal.length() > 30){
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
        std::string mpdError = "ERROR :invalid password";
        send(user->getSocket(), mpdError.c_str(), mpdError.length(), 0);
    }

    user->setChannel(canal);
    channel->AddUser(user, mdp, 0);

    std::string valid = user->getNickname() + " JOIN " + canal + "\r\n";
    send(user->getSocket(), valid.c_str(), valid.length(), 0); 
}