/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:00:59 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/04/25 00:53:10 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "client.hpp"

Client::Client(int fd) : cFd(fd), isAuth(false) {}

void Client::setNick(const std::string& nickName)
{
	nick = nickName;
}

const std::string& Client::getNick() const
{
	return nick;
}

void Client::authenticate()
{
	isAuth = true;
}

bool Client::isAuthenticated() const
{
	return isAuth;
}

int Client::getFd() const
{
	return cFd;
}

Client::~Client()
{
	close(cFd);
}
