/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 16:15:57 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/22 22:38:12 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Replies.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <exception>
#include <sstream>
#include <limits>
#include <cstring> // for memset usage
#include <sys/socket.h> // socket(), bind(), listen(), accept(), etc. NOT TO FORGET WHICH FUNCTION DEFINED WHERE :)
#include <netinet/in.h> // sockaddr_in, htonl(), htons(), etc.
#include <arpa/inet.h> // inet_addr(), inet_ntoa()
#include <netdb.h> // getaddrinfo(), gethostbyname()
#include <unistd.h> // close(), lseek()
#include <fcntl.h> // fcntl(), O_NONBLOCK
#include <poll.h> // poll()
#include <signal.h> // signal(), sigaction()
#include <csignal>
#include <errno.h>
#include <iomanip>
#include <ctime>
#include <cstdlib>

#define MAX_CLIENT 50
#define BUFFER_SIZE 1024

class Client;
class Channel;

class Server 
{
	private :
	bool client_removed;
	std::string	serverName;
	int port;
	std::string passwd;
	int	Fd;
	static int signal; // for signal function
	std::vector<Client *> cls;
	std::vector<struct pollfd>	fd_polls;
	std::map<std::string, Channel*> channels;
	bool isCap;
	
	//Functions 
	static void handleSignal(int sig);
	void	addClient();
	void	sendAndReceiveClient(int poll_index);
	void	removeClient(int poll_index);

	// Client commands
	void	Replies(int fd, int code, const std::string &msj) const;
	void	parseHandleCmd(Client &client, const std::string &cmd);
	void	passHandle(Client &client, const std::vector<std::string> &arguments);
	void	nickNameHandle(Client &client, const std::vector<std::string> &arguments);
	void	userHandle(Client &client, const std::vector<std::string> &arguments);
	void	broadcast(const std::string& message, int exclude_fd = -1);
	void	checkClientTimeouts(); // This function took me so long to understand why server removes client immidiately I comment it out for now
	void	privmsgHandle(Client &client, const std::vector<std::string>& args);
	void	capHandle(Client &client, const std::vector<std::string>& args);
	void	pingHandle(Client &client, const std::vector<std::string>& args);
	void	pongHandle(Client &client, const std::vector<std::string>& args);
	void	joinHandle(Client &client, const std::vector<std::string>& args);
	void	quitHandle(Client &client, const std::vector<std::string>& args);
	void	chanComments(Client &client, std::string &cmd, const std::vector<std::string>& args);
	void	executeKICK(Channel &channel, Client &client, const std::string& to_kick, std::string reason);
	public :
	void	Routine();
	Server(const int port_,std::string password_);
	void setupSignals();
	void startServer();
	std::string getCreationTime() const;
	Channel *findOrCreateChannel(const std::string& name);
	Channel *findChannel(const std::string& name);
	Client *findClientByNick(const std::string &kc);
	~Server();
};

#endif
