/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:00:59 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/15 21:58:00 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Client.hpp"
#include <unistd.h>

Client::Client(int fd) : fd(fd), hasPassword(false), isRegistered(false) {}

Client::~Client()
{
	close(fd);
}

int Client::getFd() const
{
	return fd;
}

const std::string& Client::getNickname() const
{
	return nickname;
}

const std::string& Client::getUsername() const
{
	return username;
}

const std::string& Client::getRealname() const
{
	return realname;
}

const std::string& Client::getHostname() const
{
	return hostname;
}

bool Client::getHasPassword() const
{
	return hasPassword;
}

bool Client::getIsRegistered() const
{
	return isRegistered;
}

void Client::setNickname(const std::string& nick)
{
	if (nick.empty())
		throw std::invalid_argument("Nickname cannot be empty");
	nickname = nick;
}

void Client::setUserInfo(const std::string& user, const std::string& real)
{
	username = user;
	realname = real;
}

void Client::setPassword(bool status)
{
	hasPassword = status;
}

void Client::setRegistered(bool status)
{
	isRegistered = status;
}

void Client::setInvalidPasswordAttempt()
{
	invalid_password_attempts++; 
	updateLastActivity();
}

void Client::appendToBuffer(const std::string& data)
{
	buffer += data;
}

void Client::clearBuffer()
{
	buffer.clear();
}

void Client::eraseFromBuffer(size_t pos, size_t len)
{
	buffer.erase(pos, len);
}

const std::string& Client::getBuffer() const
{
	return buffer;
}
void Client::updateLastActivity()
{
	last_activity = time(NULL); 
}

bool Client::shouldDisconnect() const
{
	return (invalid_password_attempts >= 3) || 
			(time(NULL) - last_activity > 30); // 30 sn timeout
}
bool Client::canRegister() const
{
	return hasPassword && !getNickname().empty() && !getUsername().empty();
}
