#pragma once
#include "Include.hpp"
#include <string>

class User
{
	public :
		User(int socket);
		~User();
		bool operator==(const User &other) const;
		std::string getUsername(void);
		std::string getNickname(void);
		std::string getChannel(void);
		std::string getHostname(void);
		int getSocket(void);
		int getisDown(void);
		bool getGetNick(void);
		std::string getID(void);
		std::string getbuffCommand(void);
		std::string getIp(void);
		std::string	getRealName();
		bool getIsRegistered();
		void setisDown(int down);
		void setUsername(std::string username);
		void setNickname(std::string nickname);
		void setChannel(std::string channel);
		void setHostname(std::string hostname);
		void setRealName(std::string realname);
		void setGetNick();
		void setbuffCommand(std::string command);
		void joinbuffCommand(std::string command);
		void incrementisDown();
		void setIp(std::string ip);
		void setIsRegister(bool registered);
		std::string getFullMask() const;
	private :
		int _isDown;
		const int _socket;
		bool	_isregistered;
		std::string _username;
		std::string _nickname;
		std::string _channel;
		std::string _hostname;
		std::string _buffCommand;
		std::string _ip;
		std::string _realname;
		bool _getNick;
};
