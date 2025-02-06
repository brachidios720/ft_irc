#pragma once

#include "Include.hpp"
#include "User.hpp"

class Channel
{
	public :
		Channel(std::string name);
		~Channel();
		std::string getName(void);
		void AddUser(User *user, std::string mdp, int super);
		void DelUser(User *user);
		std::string getStringUser(std::string name);
		void SendMsg(User *user, std::string message);
		bool IsHere(User *user);
		void SetPassword(std::string password);
		void SetMode(char mode, bool x);
		bool getMode(char mode);
		std::string getPassword(void);
		bool isEmpty();
		int isOp(std::string nickname);
		void changeOp(std::string nickname, int op);
		void SetUserLimit(int limit);
		int getNbUser();
		bool isPlace();
		void addUserInvite(User *user);
		bool IsInvite(User *user);
		std::vector<std::string> getUserNicknames();

	private :
		std::string _password;
		const std::string _name;
		std::map<User*, int> UserBook;
		std::vector<User*> UserInvite;
		int _userLimit;
		bool _modeK;
		bool _modeT;
		bool _modeL;
		bool _modeI;
};
