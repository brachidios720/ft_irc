#include "../Includes/Server.hpp"

bool    Server::isNickAvailable(const std::string& nickname){
        return(nicknameMap.find(nickname) == nicknameMap.end());
}

std::string     Server::extractMdp(const std::string &message){
        std::stringstream ss(message);
        std::string command, channelName, mdp;

        ss >> command;
        ss >> channelName;
        ss >> mdp;

        return mdp;
}