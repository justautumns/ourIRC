/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 22:12:38 by mtrojano          #+#    #+#             */
/*   Updated: 2025/05/16 19:59:02 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name_) : name(name_)
{
	this->invite_only = false;
	this->user_limit = false;
	this->pass_key = false;
}

void Channel::setPw(std::string pass)
{
	this->pw = pass;
	this->pass_key = true;
}

void Channel::setTopic(std::string topic_)
{
	this->topic = topic_;
}
void Channel::addUser(Client *client)
{
	channelUsers.push_back(client);
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
bool Channel::isUserInChannel(Client *client) const
{
	std::vector<Client*>::iterator iter;
	while (iter != channelUsers.end())
	{
		if (*iter == client)
			return true;
	}
	return false;
}

bool Channel::isOperator(Client *client) const
{
	std::vector<Client*>::iterator iter;
	while (iter != ops.end())
	{
		if (*iter == client)
			return true;
	}
	return false;
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
	return (this->channelUsers);
}

size_t Channel::getUserCount() const
{
	size_t i = channelUsers.size() + ops.size();
	return i;
}

void Channel::removePass()
{
	this->pw.clear();
	this->pass_key = false;
}
Channel::~Channel(){}
