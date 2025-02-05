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
