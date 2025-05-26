/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 22:12:38 by mtrojano          #+#    #+#             */
/*   Updated: 2025/05/26 19:15:21 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name_) : name(name_)
{
	this->hostName = "ourIrc.sanane.org";
	this->maxUser = 0;
	this->topic = "";
}

void Channel::setPw(std::string pass)
{
	this->pw = pass;
}

bool Channel::passwordCorrect(std::string pass)
{
	if (this->pw.compare(pass) == 0)
		return true;
	return false;
}

void Channel::setTopic(std::string topic_)
{
	this->topic = topic_;
}

void Channel::setUserLimit(int max_limit)
{
	this->maxUser = max_limit;
}

int Channel::getUserLimit() {return this->maxUser;}

void Channel::addUser(Client *client)
{
	for (std::vector<Client*>::iterator it = channelUsers.begin(); it != channelUsers.end(); ++it)
	{
		if (*it == client)
			return;
	}
	channelUsers.push_back(client);
}
std::string& Channel::getHostName()
{
	return this->hostName;
}
void Channel::removeUser(Client *client)
{
	for (std::vector<Client*>::iterator it = channelUsers.begin(); it != channelUsers.end(); ++it)
	{
		if (*it == client)
		{
			channelUsers.erase(it);
			break;
		}
	}
}

void Channel::addOperator(Client *client)
{
	for (size_t i = 0; i < ops.size(); ++i)
	{
		if (ops[i] == client)
			return;
	}
	ops.push_back(client);
}

void Channel::removeOperator(Client *client)
{
	for (std::vector<Client*>::iterator it = ops.begin(); it != ops.end(); ++it)
	{
		if (*it == client)
		{
			ops.erase(it);
			break;
		}
	}
}
void Channel::setModes(char x)
{
	// if (x == 'i' || x == 't' || x == 'k' || x == 'l')
	// {
	// 	modes.push_back(x);
	// }
	if (this->hasMode(x))
		return;
	modes.push_back(x);
}

void Channel::removeMode(char x)
{
	for (size_t i = 0; i < modes.size(); i++)
	{
		if (modes[i] == x)
		{
			modes.erase(modes.begin() + i);
			return;
		}
	}
}

std::string Channel::getModes()
{
	std::string enabled_modes;

	for (size_t i = 0; i < this->modes.size(); i++)
		enabled_modes += modes[i];
	return enabled_modes;
}

bool Channel::hasMode(char x)
{
	for (size_t i = 0; i < modes.size(); ++i)
	{
		if (x == modes[i])
			return true;
	}
	return false;
}
bool Channel::isUserInChannel(Client *client) const
{
	for (size_t i = 0; i < channelUsers.size(); ++i)
	{
		if (channelUsers[i] == client)
			return true;
	}
	return false;
}

bool Channel::isOperator(Client *client) const
{
	for (size_t i = 0; i < ops.size(); ++i)
	{
		if (ops[i] == client)
			return true;
	}
	return false;
}


void Channel::broadcast(const std::string& message)
{
	for (size_t i = 0; i < channelUsers.size(); ++i)
		send(channelUsers[i]->getFd(), message.c_str(), message.length(), 0);
}

void Channel::broadcast1(const std::string& message, Client *ptr)
{
	if (isUserInChannel(ptr))
	{
		for (size_t i = 0; i < channelUsers.size(); i++)
		{
			if (channelUsers[i] == ptr)
				continue;
			send(channelUsers[i]->getFd(), message.c_str(), message.length(), 0);
		}
	}
}
const std::string& Channel::getName() const
{
	return this->name;
}

const std::string& Channel::getTopic() const
{
	return this->topic;
}

const std::vector<Client*>& Channel::getUsers() const
{
	return this->channelUsers;
}

int Channel::getUserCount() const
{
	return channelUsers.size();
}

int Channel::getOperatorCount() {return this->ops.size();}

std::string Channel::promoteToOP_andReturnNick()
{
	addOperator(this->channelUsers[0]);
	return this->channelUsers[0]->getNickname();
}

void Channel::removePass()
{
	this->pw.clear();
	size_t i = 0;
	while (i < modes.size())
	{
		if (modes[i] == 'k')
			modes.erase(modes.begin() + i);
		else
			++i;
	}
	
}

Channel::~Channel(){}
