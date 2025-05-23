/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 17:00:59 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/23 19:30:20 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Channel.hpp"
#include <unistd.h>

Client::Client(int fd)
{
	this->fd = fd;
	this->hasPassword = false;
	this->isRegistered = false;
	this->isInChannel = false;
	this->got_cap_end = false;
	// this->isOnline = true;
	this->hostname = "localhost";
}

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
	//return hasPassword && !getNickname().empty() && !getUsername().empty();
	return !nickname.empty() && !username.empty();
}

std::vector<std::string> &Client::getJoinedChannelsName()
{
	return (this->joinedChannels);
}

// std::vector<std::string> &Client::getJoinedChannelsName() {return this->invitations;}
void Client::addJoinedChannel(std::string channel_name) {this->joinedChannels.push_back(channel_name);}
void Client::addInvitation(std::string channel_name)
{
	for (size_t i = 0; i < this->invitations.size(); i++)
	{
		if (this->invitations[i].compare(channel_name) == 0)
			return;
	}
	this->invitations.push_back(channel_name);
}

bool Client::hasInvitation(std::string channel_name)
{
	for (size_t i = 0; i < this->invitations.size(); i++)
	{
		if (this->invitations[i].compare(channel_name) == 0)
			return true;
	}
	return false;
}

void Client::setisOnline(bool status)
{
	this->isOnline = status;
}

bool Client::getIsOnline() const
{
	return (this->isOnline);
}

bool Client::is_cap_end_received() {return this->got_cap_end;}
void Client::set_cap_end(bool status) {this->got_cap_end = status;}
