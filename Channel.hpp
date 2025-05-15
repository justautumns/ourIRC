/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 21:49:05 by mtrojano          #+#    #+#             */
/*   Updated: 2025/05/15 22:41:51 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <iostream>
#include "Client.hpp"
#include <vector>

// name
// password
// topic
// invite only...
// restrictions for Topic command
// connected user limit
// operators

class Channel
{
	std::string name;
	std::string pw;
	std::string topic;
	bool invite_only;
	bool pass_key;
	bool user_limit;
	int maxUser;
	std::vector<Client *> ops;
	std::vector<Client *> channelUsers;
	public:
		Channel(std::string name);
		~Channel();
		void setPw(std::string pass);
		void setTopic(std::string topic_);
		void removePass();
};
#endif
