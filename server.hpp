/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 16:15:57 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/04/25 01:43:49 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP


#include "client.hpp"
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

#define MAX_CLIENT 10
#define BUFFER_SIZE 512

class Client;

class Server 
{
	private :
	std::string	serverName;
	int port;
	std::string passwd;
	int	Fd;
	static int signal; // for signal function
	std::vector<Client*> cls;
	std::vector<struct pollfd>	fd_polls;
	
	//Functions 
	static void handleSignal(int sig);
	void	addClient();
	void	sendAndReceiveClient(int poll_index);
	void	removeClient(int poll_index);
	void	response(int client_fd, const std::string& message);
	// void	isNewConnection();
	// void	newClientData(size_t poll_index);
	// void	clientCommands(int cFd, const std::string &actualCommand);
	// void	welcomeClients(int cFd);
	
	public :
	void	Routine();
	Server(const int port_,std::string password_);
	void setupSignals();
	void startServer();
	~Server();
};

#endif
