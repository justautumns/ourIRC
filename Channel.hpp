/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 21:49:05 by mtrojano          #+#    #+#             */
/*   Updated: 2025/05/22 22:16:42 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <iostream>
#include "Client.hpp"
#include "Server.hpp"
#include <vector>

// name
// password
// topic
// invite only...
// restrictions for Topic command
// connected user limit
// operators
class Client;

class Channel
{
	std::string name;
	std::string pw;
	std::string topic;
	std::string hostName;
	std::vector<char>modes;
	int maxUser;
	std::vector<Client *> ops;
	std::vector<Client *> channelUsers;
	public:
		Channel(std::string name);
		~Channel();
		void setPw(std::string pass);
		void setTopic(std::string topic_);
		void setUserLimit(int max_limit);
		void removePass();
		void addUser(Client* client);
		void removeUser(Client* client);
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool isUserInChannel(Client* client) const;
		bool isOperator(Client* client) const;

		// // Mod ayarları
		// void setInviteOnly(bool status);
		// void setTopicRestricted(bool status);
		// void setUserLimit(size_t limit);
		void setModes(char x);
		void removeMode(char x);
		std::string promoteToOP_andReturnNick();

		// Getters
		const std::string& getName() const;
		const std::string& getTopic() const;
		std::string& getHostName();
		const std::vector<Client*>& getUsers() const;
		size_t getUserCount() const;
		int getOperatorCount();
		void broadcast(const std::string& message);
		void broadcast1(const std::string& message, Client *ptr);
		bool hasMode(char x);

};
#endif
