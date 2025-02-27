#pragma once

#include "Include.hpp"
#include "Channel.hpp"
#include "User.hpp"

static bool Stop = 0;

struct s_socket
{
	int fd;
	struct sockaddr_in info; 
};

class Server
{
	public :
		Server(std::string const &port, std::string const &password);
		~Server();
		
	private :
		Server();
		int _port;
		const std::string _password;
		s_socket SClient;
		s_socket SServer;

		std::vector<pollfd> * poll_fds;
		int numConnection;

		std::map<int, User*> UserTab;
		std::map<std::string, User*> NicknameMap;
		std::map<std::string, Channel*> ChannelTab;
		
		void	HandleMessage(User *user, int num, std::vector<pollfd> client_fds);
		int		FindCommand(User *user, std::string command);
		void	ConnectClient();
		bool 	Server_start();
		bool	Server_loop();
		void 	RemoveUser(int fd);
		void	CommandCAP(User *user); //
		int		CommandPASS(User *user, std::string pass);//
		void	CommandNICK(User *user, std::string message);//
		void	CommandJOIN(User *user, std::string message);//
		void 	CommandJOIN2(User *user, std::string nameChannel, std::string mdp);
		void	CommandUSER(User *user, std::string message);//
		void	CommandNAMES(User *user, Channel *channel);//
		void	CommandPRIVMSG(User *user, std::string message);//
		void	CommandPART(User *user, std::string message);//
		void	CommandMODE(User *user, std::string message);//
		void	CommandMODE2(User *user, char channel, int status, std::string supmode, std::string nameChannel);
		void	CommandTOPIC(User *user, std::string message);//
		void	CommandINVITE(User *user, std::string message);//
		void	CommandKICK(User *user, std::string message);//
		void	CommandPING(User *user, std::string message);//
		void	CommandQUIT(User *user, std::string message);

		void 	ModeK(User *user, Channel *channel, std::string message, int i);//
		void 	ModeI(User *user, Channel *channel, int i);//
		void 	ModeO(User *user, Channel *channel, std::string message, int i);//
		void 	ModeT(User *user, Channel *channel, int i);//
		void 	ModeL(User *user, Channel *channel, std::string message, int i);


//		COMMAND UTILS
		bool	isNickAvailable(const std::string& nickname);
		std::string	exctracteChannelName(const std::string &message);
		std::string exctracteMdp(const std::string &message);
		
		void	timeOut(User *user);
		void	SendMessage(User *user, Channel *channel, std::string mes);
		Channel	*FindChannel(std::string search);
		static void	handle_signal(int signal);
};
