/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:06:42 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/04/26 14:34:57 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int Server::signal = 0;
Server::Server(const int port_, std::string password_) : port(port_) , passwd(password_)
{
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
				addClient();
			else {
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
		throw std::runtime_error("accept error");
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	struct pollfd new_poll;
	new_poll.fd = client_fd;
	new_poll.events = POLLIN;
	fd_polls.push_back(new_poll);

	cls.push_back(new Client(client_fd));

	std::cout << "New client connected. FD: " << client_fd << std::endl;
}

void Server::sendAndReceiveClient(int poll_index)
{
	char buffer[1024];
	int client_fd = fd_polls[poll_index].fd;
	size_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <= 0)
	{
		// SADECE gerçek hata durumlarında removeClient çağır
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			removeClient(poll_index);
	}
	 else
	{
		buffer[bytes_read] = '\0';
		response(client_fd, std::string(buffer));
	}
}
void Server::response(int client_fd, const std::string& message)
{
	// Basit echo örneği (IRC komut işleme burada yapılacak)
	std::string reply = "server: " + message;
	send(client_fd, reply.c_str(), reply.size(), 0);

	// Örnek komut ayrıştırma
	if (message.find("NICK") == 0)
	{
		// Nick değiştirme işlemleri
	}
	else if (message.find("USER") == 0)
	{
		// Nick değiştirme işlemleri
	}
	// I'll modify below later
	// else if (message.find("NICK") == 0)
	// {
	// 	// Nick değiştirme işlemleri
	// }
	// else if (message.find("NICK") == 0)
	// {
	// 	// Nick değiştirme işlemleri
	// }
	// else if (message.find("NICK") == 0)
	// {
	// 	// Nick değiştirme işlemleri
	// }
}
void Server::removeClient(int poll_index)
{
	// Sunucu soketini asla silme
	if (poll_index == 0)
	{
		std::cerr << "Attempted to remove server socket!" << std::endl;
		return;
	}

	// Geçerlik kontrolü
	if (poll_index < 0 || (size_t)poll_index >= fd_polls.size())
	{
		std::cerr << "Invalid poll_index: " << poll_index << std::endl;
		return;
	}

	int client_fd = fd_polls[poll_index].fd;
	close(client_fd);

	// Client listesinden kaldır (poll_index-1 çünkü ilk eleman sunucu)
	if ((size_t)(poll_index-1) < cls.size())
	{
		delete cls[poll_index-1];
		cls.erase(cls.begin() + poll_index - 1);
	}

	fd_polls.erase(fd_polls.begin() + poll_index);
	std::cout << "Client removed. FD: " << client_fd << std::endl;
}

// server.cpp'de
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
	for (size_t i = 0; i < cls.size(); ++i)
		delete cls[i];
}
