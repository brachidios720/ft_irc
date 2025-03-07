#include "../Includes/Server.hpp"

bool    Server::isNickAvailable(const std::string& nickname){
        return(NicknameMap.find(nickname) == NicknameMap.end());
}

std::string     Server::exctracteChannelName(const std::string &message){
        std::stringstream ss(message);
        std::string command, channelName;

        ss >> command;
        ss >> channelName;

        return channelName;
}

std::string     Server::exctracteMdp(const std::string &message){
        std::stringstream ss(message);
        std::string command, channelName, mdp;

        ss >> command;
        ss >> channelName;
        ss >> mdp;

        return mdp;
}