/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:06:42 by mehmeyil          #+#    #+#             */
/*   Updated: 2026/03/28 13:11:11 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int Server::signal = 0;
Server::Server(const int port_, std::string password_) : port(port_) , passwd(password_)
{
	// serverName = " IRC Server by Emre & Michal";
	serverName = "irc.em.org";
	setupSignals();
}

void Server::startServer()
{
	// Create socket
	this->Fd = socket(AF_INET, SOCK_STREAM, 0);
	if (Fd < 0)
		throw std::runtime_error("socket cannot be created");
	fcntl(Fd, F_SETFL, O_NONBLOCK);
	// fcntl is for a non-blocking socket, fcntl is defining socket mode actually. F_SETFL sets the file status flag , O_NONBLOCK nonblocking mode flag
	// if the flag is non-blocking, it actually says it wont wait for accept, recv or send functions or if it fails
	// it will return EWOULDBLOCK / EAGAIN errors

	// Optimize socket
	int opt = 1;
	if (setsockopt(Fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		// SOL_SOCKET says that options to look for are in the socket level
		// SO_REUSEADDR says that port is reusable, more clients can use same port, when we restart the server TCP has TIME_WAIT ports and they can be reusable.
		// opt is the address of the option
		// sizeof(opt) basicly len of the option (socklen_t)
		close(Fd);
		throw std::runtime_error("setsockopt error");
	}
	// Start binding
	struct sockaddr_in adress;
	memset(&adress, 0, sizeof(adress));
	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = inet_addr("127.0.0.1"); //0x0100007F
	adress.sin_port = htons(this->port); //(return (port >> 8) | (port << 8)) 

	// We need to clear all bytes in padding that's the reason we use memset above
	// sin_family is the family of the adresses, 's' in the begining is saying this is a struct type
	// in or IN is short name for internet
	// AF_INET is for IPv4 usage, for IPv6 it would've been AF_INET6
	// sin_addr struct in_addr type IP adress structer
	// s.addr is saying adress is 32byte binary form, it's basicly an integer that holds the IPv4 adress
	// INADDR_ANY says that we will listen to all IP adresses
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
	server_poll.fd = Fd; // We assign our socket Fd to pollfd's fd, so we can accept new connections via socket Fd
	server_poll.events = POLLIN; // we listen to the events, it will notify us when there's new connection on the socket
	this->fd_polls.push_back(server_poll); // We store multiple connection in a vector
}

void Server::Routine()
{
	while (true)
	{
		int poll_returnValue = poll(fd_polls.data(), fd_polls.size(), 100);
		// Poll will listen to the sockets(Clients) in a non-blocking way. POLLIN will detect from which socket(client)
		// we are receiving data, POLLER will tell us there's an error on data receiving, POLLHUP will say connection is lost. data() is a variable of the poll_fd's
		// size() says how many sockets are there. 100 is time in ms, in which if no data is received from the client, connection is lost.
		
		if (poll_returnValue < 0 && signal == 0)
			throw std::runtime_error("poll error");
		else if (signal)
		{
			std::cout << "\nServer shutting down..." << std::endl;
			if (cls.size() > 0)
			{
				for (size_t i = 0; i < cls.size(); ++i)
					delete cls[i];
			}
			for (size_t i = 0; i < channels.size(); ++i)
			{
				delete channels[i];
			}
			channels.clear();
			cls.clear();
			close(Fd);
			break;
		}
		
		for (size_t i = 0; i < fd_polls.size(); ++i)
		{
			if (fd_polls[i].revents & POLLIN) // here revents are the events that has happend, like POLLIN POLLOUT etc. It is checkin at least one event have happened.
			{
				if (fd_polls[i].fd == Fd)
					addClient(); // we add the client which is new if the socket FD matches
				else 
				{
					if (fd_polls[i].revents & (POLLERR | POLLHUP))
					{
						Client *client = cls[i - 1];
						std::vector<std::string> arg;
						arg.push_back(":lost connection\r\n");
						quitHandle(*client, arg);
						// If there's an error or connection lost, we remove the client
					}
					else // Else we start examine the data that our socket receives from the clients.
						sendAndReceiveClient(i);
				}
			}
		}
		checkPingTimeouts();
	}
}
void Server::addClient()
{
	struct sockaddr_in client_addr; // This holds IP and Port info
	socklen_t addr_len = sizeof(client_addr); // This size is needed for accept()
	int client_fd = accept(Fd, (struct sockaddr*)&client_addr, &addr_len); // Here we create an Fd for the client and we tell our main socket to accept it.
	// (struct sockaddr*)&client_addr - we typecast that as sockaddr* cuz accept checks the memory location of the client_addr via a pointer.
	if (client_fd < 0)
	{
		std::cerr << "accept error: " << strerror(errno) << std::endl;
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Adding to poll list this time since we already create our main socket we create poll fd for clients.
	struct pollfd new_poll;
	new_poll.fd = client_fd;
	new_poll.events = POLLIN;
	new_poll.revents = 0;
	fd_polls.push_back(new_poll);

	Client *yeniUser = new Client(client_fd);
	// clientC = yeniUser->getIsOnline();
	cls.push_back(yeniUser);

	std::cout << "New client connected FD number is : " << yeniUser->getFd() <<  std::endl;
}

void Server::sendAndReceiveClient(int poll_index)
{
	char buffer[BUFFER_SIZE];
	int client_fd = fd_polls[poll_index].fd;
	Client *client = cls[poll_index - 1]; // - 1 cause we have server fd in our fd_poll indexes
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0); // now we can receive data from clients

	if (bytes_read <= 0)
	{
		if (errno != EWOULDBLOCK)
		{
			client->setGotSignal(true);
			std::vector<std::string> arg;
			arg.push_back(":lost terminal\r\n");
			quitHandle(*client, arg);
		}
		return;
	}
	if (bytes_read > 0)
		client->updateLastActivity();

	buffer[bytes_read] = '\0';
	client->appendToBuffer(buffer);
	to_remove = false;
	
	if (client->getBuffer().length() > 511)
	{
		std::vector<std::string> arg;
		arg.push_back(":Msg is too long, you're now disconnected\r\n");
		quitHandle(*client, arg);
		return;
	}

	// Process complete commands
	size_t pos;
	// "COMMAND1\nCOMMAND2\n" can also be tried that's why we are using a loop
	while (!to_remove && (pos = client->getBuffer().find("\r\n")) != std::string::npos) 
	{
		std::string command = client->getBuffer().substr(0, pos);
		client->eraseFromBuffer(0, pos + 2);
		parseHandleCmd(*client, command);
	}
}

// This function was a IRC standard one, it shows the localtime of server creation
std::string Server::getCreationTime() const
{
	time_t now = time(0);
	char buf[80];
	strftime(buf, sizeof(buf), "%a %b %d %Y at %H:%M:%S", localtime(&now));
	return buf;
}

std::vector<std::string> splitIRCMessage(const std::string& message)
{
	std::vector<std::string> args;
	std::istringstream iss(message);
	std::string arg;
	bool inTrailing = false;

	while (iss >> arg)
	{
		if (!inTrailing && arg[0] == ':')
		{
			inTrailing = true;
			std::string trailingArg;
			std::getline(iss, trailingArg);
			arg = arg.substr(1) + trailingArg;
			args.push_back(arg);
			break;
		}
		args.push_back(arg);
	}
	return args;
}


Channel* Server::findOrCreateChannel(const std::string& name)
{
	for(size_t i = 0; i < channels.size(); ++i)
	{
		if (channels[i]->getName() == name)
			return (channels[i]);
	}

	Channel* newChannel = new Channel(name);
	channels.push_back(newChannel);
	return newChannel;
}
Channel* Server::findChannel(const std::string& name)
{
	for(size_t i = 0; i < channels.size(); ++i)
	{
		if (channels[i]->getName() == name)
			return (channels[i]);
	}
	return NULL;
}

void Server::parseHandleCmd(Client &client, const std::string &command)
{
	std::vector<std::string> args = splitIRCMessage(command);

	if (args.empty()) return;

	std::string cmd = args[0];
	args.erase(args.begin());

	for (size_t i = 0; i < cmd.size(); i++)
		cmd[i] = std::toupper(cmd[i]);

	if (cmd == "PASS") passHandle(client, args);
	else if (cmd == "CAP") capHandle(client, args);
	else if (cmd == "NICK") nickNameHandle(client, args);
	else if (cmd == "USER") userHandle(client, args);
	else if (cmd == "JOIN") joinHandle(client, args);
	else if (cmd == "PRIVMSG" || cmd == "MSG") privmsgHandle(client, args);
	else if (cmd == "PING") pingHandle(client, args);
	else if (cmd == "QUIT") quitHandle(client, args);
	else if(cmd == "KICK" || cmd == "PART" || cmd == "MODE"
			|| cmd == "INVITE" || cmd == "TOPIC") chanCommands(client, cmd, args);
	else
	{
		if (client.getIsRegistered())
		{
			std::string msg = client.getNickname() + " :" + cmd + "\r\n";
			// broadcast(msg, client.getFd());
		}
		else
			Replies(client.getFd(), ERR_UNKNOWNCOMMAND, cmd + " :Unknown command");
	}
}

void Server::Replies(int fd, int code, const std::string &message) const
{
	std::stringstream reply;
	reply << ":" << serverName << " " << std::setw(3) << std::setfill('0')
			<< code << " " << message << "\r\n";
	send(fd, reply.str().c_str(), reply.str().length(), 0);
}


void Server::checkPingTimeouts()
{
	time_t current_time = time(NULL);

	for (size_t i = 0; i < cls.size(); ++i)
	{
		Client* client = cls[i];

		if (!client || client->getLastPingTime() == 0) continue;

		// Eğer client PING bekliyorsa ve zaman aşımına uğradıysa mefta oldu diyebiliriz
		if (client->isWaitingForPong() && 
			(current_time - client->getLastPingTime() > PING_TIMEOUT))
		{
			std::vector<std::string> args;
			args.push_back(":Ping timeout");
			quitHandle(*client, args);
			continue;
		}
		
		// Eğer belirli bir süredir aktivite yoksa PING gönderiyoruz
		if (current_time - client->getLastActivity() > PING_INTERVAL)
		{
			std::string ping_msg = "PING :" + serverName + "\r\n";
			send(client->getFd(), ping_msg.c_str(), ping_msg.length(), 0);
			client->setWaitingForPong(true);
			client->setLastPingTime(current_time);
		}
	}
}
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
			broadcast(":" + serverName + " " + cls[i]->getNickname() + " has been disconnected\r\n", client_fd);
			delete cls[i];
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
	std::signal(SIGQUIT, &Server::handleSignal); // \ kill
}

Server::~Server()
{
	for (size_t i = 0; i < cls.size(); i++)
	{
		delete cls[i];
	}
	cls.clear();
	for (size_t i = 0; i < channels.size(); i++)
	{
		delete channels[i];
	}
	channels.clear();
	close(Fd);
}
