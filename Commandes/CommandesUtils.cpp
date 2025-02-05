#include "../Includes/Server.hpp"

bool    Server::isNickAvailable(const std::string& nickname){
        return(NicknameMap.find(nickname) == NicknameMap.end());
}