/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelCommands.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 14:16:54 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/23 00:01:01 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

Client *Server::findClientByNick(const std::string &kc)
{
	for (size_t i = 0; i < cls.size(); ++i)
	{
		if (cls[i]->getNickname() == kc)
			return (cls[i]);
	}
	return (NULL);
}

void Server::executeKICK(Channel &channel, Client &client, const std::string& to_kick, std::string reason)
{
	Client* target = findClientByNick(to_kick);
	if (!target)
	{
		Replies(client.getFd(), ERR_NOSUCHNICK, client.getNickname() + " " + to_kick + " :No such nick");
		return;
	}
	
	if (!channel.isUserInChannel(target))
	{
		Replies(client.getFd(), ERR_USERNOTINCHANNEL, client.getNickname() + " " + to_kick + " " + channel.getName() + " :They aren't on that channel");
		return;
	}

	std::string kickMsg = ":" + client.getNickname() +  "!" + client.getUsername() + "@" + client.getHostname()
							+ " KICK " + channel.getName() + " " + to_kick + (reason[0] == ':' ? " " : " :") + reason + "\r\n";
	channel.broadcast(kickMsg);
	if (channel.isOperator(target))
		channel.removeOperator(target);
	channel.removeUser(target);

}
void Server::chanComments(Client &client, std::string &cmd, const std::vector<std::string>& args)
{
	if (args.empty())
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + "Not enough parameters");
		return;
	}

	Channel* channel = (cmd == "KICK" || cmd == "TOPIC" || cmd == "PART") ? findChannel(args[0]) : findChannel(args[1]); // temporary solution for INVITE check, but we're gonna have to change that

	if (!channel)
	{
		if (args[0].compare(client.getNickname()) == 0) // for the MODE command being sent by irssi -> (MODE <nickname> +i)
			return;
		Replies(client.getFd(), ERR_NOSUCHCHANNEL, client.getNickname() + " " + args[0] + " :No such channel");
		return;
	}

	if (cmd == "KICK")
	{
		if (args.size() < 2)
		{
			Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " KICK :Not enough parameters");
			return;
		}

		if (!channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, client.getNickname() + " " + args[0] + " :You're not channel operator");
			return;
		}
		std::string reason;
		if (args.size() >= 3)
		{
			if (args[2][0] == ':')
			{
				for (size_t i = 2; i < args.size(); i++)
				{
					if (i > 2)
						reason += " ";
					reason += args[i];
				}
			}
			else
				reason = args[2];
		}
		executeKICK(*channel, client, args[1], reason);
	}

	if (cmd == "TOPIC")
	{
		if (args.empty())
		{
			Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " TOPIC :Channel name required");
			return;
		}

		std::string channelName = args[0];

		if (!channel)
		{
			Replies(client.getFd(), ERR_NOSUCHCHANNEL, client.getNickname() + " " + channelName + " :No such channel");
			return;
		}

		// Kanal operatör kontrolü (veya +t modu)
		if (channel->hasMode('t') && !channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, client.getNickname() + " " + channelName + " :You're not channel operator");
			return;
		}


		if (args.size() == 1)
		{
			Replies(client.getFd(), RPL_TOPIC, client.getNickname() + " " + channelName + " :" + channel->getTopic());
			return;
		}
		// Topic'i birleştir (args[1] ve sonrası)
		std::string newTopic;
		for (size_t i = 1; i < args.size(); ++i)
		{
			if (i > 1) newTopic += " "; // Boşluk ekle (ilk argümandan sonra)
			// ':' ile başlıyorsa, IRC protokolüne göre kalan kısmı tek string olarak al
			if (i == 1 && !args[i].empty() && args[i][0] == ':')
			{
				newTopic += args[i].substr(1); // ':' işaretini atla
				// Kalan tüm argümanları ekle
				for (size_t j = i + 1; j < args.size(); ++j)
					newTopic += " " + args[j];
				break;
		}
		newTopic += args[i];
	}

		// Topic'i güncelle
		channel->setTopic(newTopic);

		// IRC protokolüne uygun mesaj formatı
		std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@"
								+ client.getHostname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
		channel->broadcast(topicMsg);
	}

	else if (cmd == "PART")
	{
		if (!channel->isUserInChannel(&client))
		{
			Replies(client.getFd(), ERR_NOTONCHANNEL, client.getNickname() + " " + args[0] + " :You're not on that channel");
			return;
		}

		std::string reason;

		if (args.size() > 1)
		{
			if (args[1][0] == ':')
			{
				for (size_t i = 1; i < args.size(); i++)
				{
					if (i > 1)
						reason += " ";
					reason += args[i];
				}
			}
			else
				reason = args[1];
		}

		std::string partMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@"
								+ client.getHostname() + " PART " + args[0] + (reason[0] == ':' ? " " : " :") + reason + "\r\n";

		channel->broadcast(partMsg);
		channel->removeUser(&client);

		if (channel->isOperator(&client))
		{
			if (channel->getUserCount() >= 1 && channel->getOperatorCount() == 1)
			{
				std::string to_promote = channel->promoteToOP_andReturnNick();
				std::string prom_msg = ":" + serverName + " MODE " + args[0] + " +o " + to_promote + "\r\n";
				channel->broadcast(prom_msg);
			}
			channel->removeOperator(&client);
		}
	}

	else if (cmd == "INVITE")
	{
		if (args.size() < 2)
		{
			Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " INVITE :Not enough parameters");
			return;
		}

		Client* target = findClientByNick(args[0]);

		if (!target)
		{
			Replies(client.getFd(), ERR_NOSUCHNICK, client.getNickname() + " " + args[0] + " :No such nick");
			return;
		}

		std::string channelName = args[1];
		channel = findChannel(channelName);

		if (!channel)
		{
			Replies(client.getFd(), ERR_NOSUCHCHANNEL, client.getNickname() + " " + channelName + " :No such channel");
			return;
		}

		if (!channel->isUserInChannel(&client))
		{
			Replies(client.getFd(), ERR_NOTONCHANNEL, client.getNickname() + " " + channelName + " :You're not on that channel");
			return;
		}

		if (!channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, client.getNickname() + " " + channelName + " :You're not channel operator");
			return;
		}

		if (channel->isUserInChannel(target))
		{
			Replies(client.getFd(), ERR_USERONCHANNEL, client.getNickname() + " " + target->getNickname() + " " + channelName + " :is already on channel");
			return;
		}

		target->addInvitation(channel->getName());
		std::string invite_msg = ":" + client.getNickname() + "!" + client.getUsername() + "@"
								+ client.getHostname() + " INVITE " + target->getNickname() + " :" + channel->getName() + "\r\n";
		send(target->getFd(), invite_msg.c_str(), invite_msg.length(), 0);
		Replies(client.getFd(), RPL_INVITING, client.getNickname() + " " + target->getNickname() + " " + channelName);

	}

	else if (cmd == "MODE")
	{
		if (!channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, client.getNickname() + " " + args[0] + " :You're not channel operator");
			return;
		}

		if (args.size() >= 2 && args[1] == "+o")
		{
			Client* target = findClientByNick(args[2]);
			if (target)
			{
				channel->addOperator(target);
				std::string modeMsg = ":" + client.getNickname() + " MODE " + args[2] + " +o " + "\r\n";
				broadcast(modeMsg, client.getFd());
			}
		}

		if (args.size() >= 2 && args[1] == "-o")
		{
			Client* target = findClientByNick(args[2]);
			if (target)
			{
				channel->removeOperator(target);
				std::string modeMsg = ":" + client.getNickname() + " MODE " + args[2] + " -o " + "\r\n";
				broadcast(modeMsg, client.getFd());
			}
		}

		if (args.size() >= 2 && args[1] == "+i")
		{
			channel->setModes(args[1][1]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " +i " + "\r\n";
			channel->broadcast(modeMsg);
		}

		if (args.size() >= 2 && args[1] == "-i")
		{
			channel->removeMode(args[1][1]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " -i " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}

		if (args.size() >= 3 && args[1] == "+k")
		{
			// password syntax check?
			channel->setModes(args[1][1]);
			channel->setPw(args[2]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " +k " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}

		if (args.size() >= 3 && args[1] == "-k")
		{
			channel->removePass();
			channel->removeMode(args[1][1]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " -k " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}

		if (args.size() >= 3 && args[1] == "+l")
		{
			for (size_t i = 0; i < args[2].size(); i++)
			{
				if (i == 0 && args[2][i] == '+')
					continue;
				if (!isdigit(args[2][i]))
				{
					std::stringstream error;
					error << "Parameter of MODE +l command has to be a positive integer\r\n";
					send(client.getFd(), error.str().c_str(), error.str().length(), 0);
					return;
				}
			}

			if (args[2].length() > 3)
			{
				std::stringstream error;
				error << "The max number of clients you can set is " << MAX_CLIENT << " \r\n";
				send(client.getFd(), error.str().c_str(), error.str().length(), 0);
				return;
			}

			if (std::atoi(args[2].c_str()) > MAX_CLIENT)
			{
				std::stringstream error;
				error << "The max number of clients you can set is " << MAX_CLIENT << " \r\n";
				send(client.getFd(), error.str().c_str(), error.str().length(), 0);
				return;
			}

			channel->setUserLimit(std::atoi(args[2].c_str()));
			std::string modeMsg = ":" + client.getNickname() + " MODE " + args[2] + " +l " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}

		if (args.size() >= 2 && args[1] == "-l")
		{
			channel->removeMode(args[1][1]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " -l " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}

		if (args.size() >= 2 && args[1] == "+t")
		{
			channel->setModes(args[1][1]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " +t " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}

		if (args.size() >= 2 && args[1] == "-t")
		{
			channel->removeMode(args[1][1]);
			std::string modeMsg = ":" + client.getNickname() + " MODE " + " -t " + "\r\n";
			broadcast(modeMsg, client.getFd());
		}
	}
}
