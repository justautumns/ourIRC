/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:06:42 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/23 19:28:34 by mtrojano         ###   ########.fr       */
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
	server_poll.fd = Fd; // We assign pollfd's fd to our socket Fd so we can accept new connections via socket Fd
	server_poll.events = POLLIN; // we listen to the events it will notice us when there's new connection on the socket
	this->fd_polls.push_back(server_poll); // As we will deal with multiple connection we have to store them I used vector 
	// If we decide to use select() we would use accept()/ recv() functions instead of POLLIN 

}

void Server::Routine()
{
	while (true)
	{
		int poll_returnValue = poll(fd_polls.data(), fd_polls.size(), 100);
		// Here we use poll because we need multiple sockets to be connected and listen`ed to, it is the only way if we're not gonna
		//use multi Threads. Instead of using different threads for each client we use poll or we could have used select but to be honest
		// I never check how we could use that. Poll will listen to the sockets(Clients) as a non-blocking way. POLLIN will detect from which socket I mean
		//client are we receiving data, POLLER will tell us there's an error on data receiving, POLLHUP will say connection is lost. data() is variables of the poll_fd's
		// size says how many sockets are there. 100 is ms if no data received from the client in 100 milisecond that means connection is lost.
		
		if (poll_returnValue < 0 && signal == 0)
			throw std::runtime_error("poll error");
		else if (signal)
		{
			std::cout << "Server shutting down..." << std::endl;
			if (cls.size() > 0)
			{
				for (size_t i = 0; i < cls.size(); ++i)
					delete cls[i];
			}
			for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); it++)
			{
				delete it->second;
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
					addClient(); // we add the client which is new if the socket FD matches that we create in the function up there, I mean the previous function(startServer)
				else 
				{
					if (fd_polls[i].revents & (POLLERR | POLLHUP)) // IF revents which means an event accoured and there's an error or connection lost we remove the client
						removeClient(i);
					else // Else we start examine the data that our socket receives from the clients.
						sendAndReceiveClient(i);
				}
			}
		}
	}
	// BTW I noticed that while I am writing explanation I sometimes fuck the indent level up, so in out fd_polls there are two types of FD's the first FD in our
	// fd_polls array at index 0 is our main socket, if fd_polls[i] is 0 we are calling accept() function in our addClient function, and there we add an FD for each new client.
	// In our routine function we use a while loop because we don't know which event will occure but we keep listening. So if the FD is something else than our main socket Fd
	// That means we need to use recv/send functions in order to find our what data ve have from the client and what data our server will response. And we have functions for them
	// below.
}
void Server::addClient()
{
	struct sockaddr_in client_addr; // This actually will hold IP and Port infos
	socklen_t addr_len = sizeof(client_addr); // And this is the size of it it's needed by accept function
	int client_fd = accept(Fd, (struct sockaddr*)&client_addr, &addr_len); // Here we create an Fd for the client and we tell our main socket to accept it.
	// (struct sockaddr*)&client_addr we typecast that as sockaddr cuz accept checks the memory location of the client_addr via a pointer.
	if (client_fd < 0)
	{
		std::cerr << "accept error: " << strerror(errno) << std::endl; // Here I actually should throw an error but it's to be done later.
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Adding to poll list This time since we already create our main socket we create poll fd for clients. I called it new_poll but we can call it client_poll also
	struct pollfd new_poll;
	new_poll.fd = client_fd;
	new_poll.events = POLLIN;
	new_poll.revents = 0;
	fd_polls.push_back(new_poll);

	// push back new client I just didn't get back to using stack actually I also get error I tried
	Client *yeniUser = new Client(client_fd);
	yeniUser->setisOnline(true);
	cls.push_back(yeniUser);

	std::cout << "New client connected FD number is : " << yeniUser->getFd() <<  std::endl;
}

void Server::sendAndReceiveClient(int poll_index)
{
	char buffer[BUFFER_SIZE];
	int client_fd = fd_polls[poll_index].fd;
	Client *client = cls[poll_index - 1]; // I had a few time to find this - 1 cuz we have server fd in our fd_poll indexes it was painfull
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0); // now we can receive data from clients

	if (bytes_read <= 0)
	{
		if (errno != EWOULDBLOCK)
			removeClient(poll_index);
		return;
	}

	buffer[bytes_read] = '\0';
	client->appendToBuffer(buffer);

	// Process complete commands
	size_t pos;
	// "COMMAND1\nCOMMAND2\n" can also bw tried that's why I used a loop but still not sure about \r
	while (client->getIsOnline() && (pos = client->getBuffer().find("\r\n")) != std::string::npos) 
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
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end())
		return it->second;

	Channel* newChannel = new Channel(name);
	channels[name] = newChannel;
	return newChannel;
}
Channel* Server::findChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end())
		return it->second;
	return NULL;
}

void Server::parseHandleCmd(Client &client, const std::string &command)
{
	std::cout << "Received: '" << command << "'\n";
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
	else if (cmd == "PONG") pongHandle(client, args);
	else if (cmd == "QUIT") quitHandle(client, args);
	else if(cmd == "KICK" || cmd == "PART" || cmd == "MODE"
			|| cmd == "INVITE" || cmd == "TOPIC") chanComments(client, cmd, args);
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
			broadcast(":" + serverName + " " + cls[i]->getNickname() + " has been disconnected\r\n", client_fd);
			cls[i]->setisOnline(false);
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
}

Server::~Server()
{
	for (size_t i = 0; i < cls.size(); i++)
	{
		delete cls[i];
	}
	cls.clear();
}
