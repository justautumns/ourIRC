/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:06:42 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/13 04:48:03 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int Server::signal = 0;
Server::Server(const int port_, std::string password_) : port(port_) , passwd(password_)
{
	serverName = "ourIRC";
	setupSignals();
}

void Server::startServer()
{

	// Create socket
	this->Fd = socket(AF_INET, SOCK_STREAM, 0);
	if (Fd < 0)
		throw std::runtime_error("socket cannot be created");
	fcntl(Fd, F_SETFL, O_NONBLOCK);
	// fcntl is for a non-blocking socket fcntl is defining socket mode actually. F_SETFL sets the file status flag , O_NONBLOCK nonblocking mode flag
	// if the flag is non-blocking Michal , it actually says it wont wait for accept, recv or send functions or if it fails
	// it will return EWOULDBLOCK / EAGAIN errors and Idk what they really are, we are using this cuz subject says we won't be able to use multi threads

	// Optimize socket
	int opt = 1;
	if (setsockopt(Fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		// SOL_SOCKET says that options to look for are in the socket level
		// SO_REUSEADDR says that port is reusable more clients can use same port, when we restart the server TCP has TIME_WAIT ports and they can be reusable.
		// opt is the adress of the option
		// sizeof(opt) basicly len of the option (socklen_t)
		close(Fd);
		throw std::runtime_error("setsockopt error");
	}
	// Start binding
	struct sockaddr_in adress;
	memset(&adress, 0, sizeof(adress));
	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = INADDR_ANY;
	adress.sin_port = htons(this->port);
	// What are the things above? Here are the explanations Im just writing all of these that I learnt from youtube tutorials
	// We neet to clear all bytes in padding that's the reason we use memset above
	// sin_family is the family of the adresses s in the begining is saying this is a struct type it is technically struct in_family actually
	// in or IN is short name for internet btw
	// AF_INET is for IPv4 usage for IPv6 it could've been AF_INET6 but Im not using it.
	// sin_addr struct in_addr type IP adress structer
	// s.addr is saying adress is 32byte binary form it's basicly an integer that holds the IPv4 adress
	// INADDR_ANY says that we will listen all IP adresses
	// htons() Host to Network Short it actually removes if there's any problem of storing the bytes
	// CPU can store the port Little Endian type or Big Endian type but we'll need Big Endian type that's why we use htons 16-bit

	if (bind(Fd, (struct sockaddr*)&adress, sizeof(adress)))
	{
		close(Fd);
		throw std::runtime_error("bind failure");
	}
	if (listen(Fd, MAX_CLIENT))
	{
		close(Fd);
		throw std::runtime_error("listen failure");
	}
	// Seting up the Poll
	struct pollfd server_poll;
	server_poll.fd = Fd; // We assign pollfd's fd to out socket Fd so we can accept new connections via socket Fd
	server_poll.events = POLLIN; // we listen to the events it will notice us when there's new connection on the socket
	this->fd_polls.push_back(server_poll); // As we will deal with multiple connection we have to store them I used vector 
	// If we decide to use select() we would use accept()/ recv() functions instead of POLLIN 

}

void Server::Routine()
{
	while (true)
	{
		int poll_returnValue = poll(fd_polls.data(), fd_polls.size(), 100);
		
		if (poll_returnValue < 0 && signal == 0)
		throw std::runtime_error("poll error");
		else if (signal)
		{
			std::cout << "Server shutting down..." << std::endl;
			break;
		}
		
		for (size_t i = 0; i < fd_polls.size(); ++i)
		{
			if (fd_polls[i].revents & POLLIN)
			{
				if (fd_polls[i].fd == Fd)
				addClient(); // we add the client which is new
				else 
				{
					// SADECE HATA DURUMLARINDA removeClient ÇAĞIR
					if (fd_polls[i].revents & (POLLERR | POLLHUP))
					removeClient(i);
					else
					sendAndReceiveClient(i);
				}
			}
		}
	}
}
void Server::addClient()
{
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int client_fd = accept(Fd, (struct sockaddr*)&client_addr, &addr_len);

	if (client_fd < 0)
	{
		std::cerr << "accept error: " << strerror(errno) << std::endl;
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Adding to poll list
	struct pollfd new_poll;
	new_poll.fd = client_fd;
	new_poll.events = POLLIN;
	fd_polls.push_back(new_poll);

	// push back new client I just didn't get back to using stack actually I also get error I tried
	cls.push_back(new Client(client_fd));

	std::cout << "New client connected. FD: " << client_fd << std::endl;
}

void Server::sendAndReceiveClient(int poll_index)
{
	char buffer[BUFFER_SIZE];
	int client_fd = fd_polls[poll_index].fd;
	Client *client = cls[poll_index - 1];
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <= 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			removeClient(poll_index);
		return;
	}

	buffer[bytes_read] = '\0';
	client->appendToBuffer(buffer);

	// Process complete commands
	size_t pos;
	while ((pos = client->getBuffer().find("\r\n")) != std::string::npos)
	{
		std::string command = client->getBuffer().substr(0, pos);
		client->eraseFromBuffer(0, pos + 2);
		parseHandleCmd(*client, command);
	}
}

// This function was a IRC standart one shows the localtime of server creation
std::string Server::getCreationTime() const
{
	time_t now = time(0);
	char buf[80];
	strftime(buf, sizeof(buf), "%a %b %d %Y at %H:%M:%S", localtime(&now));
	return buf;
}
void Server::nickNameHandle(Client& client, const std::vector<std::string>& args)
{
	if (args.empty())
	{
		numericReplies(client.getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		return;
	}

	const std::string &newNick = args[0];

	// Nickname validation according to RFC 1459 it's really gettin started to be pain to check this documentary :/
	// BTW this numericReplies was also in these documentary
	if (newNick.empty() || newNick.length() > 9)
	{
		numericReplies(client.getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		return;
	}

	// This chars are considered as invalid in a nickname
	if (newNick.find_first_of(" ,*?!@.") != std::string::npos)
	{
		numericReplies(client.getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname (a nickname cannot contain these charecters ;  ,*?!@.)");
		return;
	}

	// Checking if nickname is already in use
	for (size_t i = 0; i < cls.size(); ++i)
	{
		if (cls[i]->getNickname() == newNick && cls[i]->getFd() != client.getFd())
		{
			numericReplies(client.getFd(), ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
			return;
		}
	}

	// If client was already registered, I try to broadcast to other clients but Idk
	if (client.getIsRegistered())
	{
		std::string msg = ":" + client.getNickname() + " NICK :" + newNick + "\r\n";
		broadcast(client.getNickname() + " NICK :" + newNick);
	}

	// Set new nickname
	client.setNickname(newNick);

	// If all goes well it means registiration was a success and we send a welcome msg
	if (!client.getIsRegistered() && client.canRegister())
	{
		client.setRegistered(true);
		numericReplies(client.getFd(), RPL_WELCOME, ":Welcome to the Internet Relay Network " + client.getNickname());
	}
}

void Server::userHandle(Client &client, const std::vector<std::string>& args)
{
	if (client.getIsRegistered())
	{
		numericReplies(client.getFd(), ERR_ALREADYREGISTERED, ":You may not reregister");
		return;
	}

	if (args.size() < 4)
	{
		numericReplies(client.getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		return;
	}

	const std::string &username = args[0];
	const std::string &realname = args[3];

	// Set user info
	client.setUserInfo(username, realname);

	// If this also goes well which gave me lot of errors than, we send our welcome message
	if (!client.getIsRegistered() && client.canRegister())
	{
		client.setRegistered(true);
		numericReplies(client.getFd(), RPL_WELCOME, ":Welcome to the Internet Relay Network " + client.getNickname());
		numericReplies(client.getFd(), RPL_YOURHOST, ":Your host is " + serverName);
		numericReplies(client.getFd(), RPL_CREATED, ":This server was created by Emre and Michal at: " + getCreationTime());
	}
}
void Server::parseHandleCmd(Client &client, const std::string &command) 
{
	std::vector<std::string> args;
	std::istringstream iss(command);
	std::string arg;

	while (iss >> arg)
		args.push_back(arg);

	if (args.empty()) 
		return;

	std::string cmd = args[0];
	args.erase(args.begin());

	std::cout << cmd << std::endl;
	if (cmd == "PASS") passHandle(client, args);
	else if (cmd == "NICK") nickNameHandle(client, args);
	else if (cmd == "USER") userHandle(client, args);
	// I will have to add other handles :)
	// If the cmd is unknown goes into else
	else
		numericReplies(client.getFd(), ERR_UNKNOWNCOMMAND, cmd + " :Unknown command");
}


void Server::numericReplies(int fd, int code, const std::string &message) const
{
	std::stringstream reply;
	reply << ":" << serverName << " " << std::setw(3) << std::setfill('0') 
			<< code << " " << message << "\r\n";
	send(fd, reply.str().c_str(), reply.str().length(), 0);
}

void Server::passHandle(Client &client, const std::vector<std::string>& args)
{
	if (client.getIsRegistered())
	{
		numericReplies(client.getFd(), ERR_ALREADYREGISTERED, ":You may not reregister");
		return;
	}

	if (args.empty())
	{
		numericReplies(client.getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return;
	}
	if (args[0].find(' ') != std::string::npos)
	{
		numericReplies(client.getFd(), ERR_NEEDMOREPARAMS, "PASS :Password may not contain spaces");
		return;
	}
	if (args[0] == passwd)
	{
		client.setPassword(true);
		numericReplies(client.getFd(), RPL_WELCOME, ":Password accepted");
	} 
	else
	{
		numericReplies(client.getFd(), ERR_PASSWDMISMATCH, ":Password incorrect");
		// We have to find and disconnect this client
		for (size_t i = 0; i < fd_polls.size(); ++i)
		{
			if (fd_polls[i].fd == client.getFd())
			{
				removeClient(i);
				break;
			}
		}
	}
}
// void Server::checkClientTimeouts()
// {
// 	for (size_t i = 0; i < cls.size(); ++i)
// 	{
// 		if (!cls[i]->getIsRegistered() && cls[i]->shouldDisconnect())
// 			removeClient(i + 1); // +1 because 0 is server fd
// 	}
// }
void Server::broadcast(const std::string& message, int exclude_fd)
{
	for (size_t i = 0; i < cls.size(); ++i)
	{
		if (cls[i]->getFd() != exclude_fd)
			send(cls[i]->getFd(), message.c_str(), message.size(), 0);
	}
}
void Server::removeClient(int poll_index)
{
	if (poll_index == 0)
	{
		std::cerr << "Attempted to remove server socket!" << std::endl;
		return;
	}

	if (poll_index < 0 || (size_t)poll_index >= fd_polls.size())
	{
		std::cerr << "Invalid poll_index: " << poll_index << std::endl;
		return;
	}

	int client_fd = fd_polls[poll_index].fd;
	close(client_fd);

	for (size_t i = 0; i < cls.size(); ++i)
	{
		if (cls[i]->getFd() == client_fd)
		{
			// I tried to announce leaving msg but didn't work for now.
			broadcast(":" + cls[i]->getNickname() + " QUIT :Disconnected", client_fd);
			cls.erase(cls.begin() + i);
			break;
		}
	}

	fd_polls.erase(fd_polls.begin() + poll_index);
	std::cout << "Client removed. FD: " << client_fd << std::endl;
}

void Server::handleSignal(int sig)
{
	Server::signal = sig; // Static değişkeni güncelle
}
void Server::setupSignals()
{
	std::signal(SIGINT, &Server::handleSignal);  // CTRL+C
	std::signal(SIGTERM, &Server::handleSignal); // kill komutu
}

Server::~Server()
{}
