/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelComments.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 14:16:54 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/19 16:19:50 by mehmeyil         ###   ########.fr       */
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
bool	Server::isChannelOperatorCmd(std::string cmd)
{
	if (cmd.compare("KICK") == 0 || cmd.compare("INVITE") == 0 || cmd.compare("TOPIC") == 0 || cmd.compare("MODE") == 0)
		return true;
	return false;
}
void Server::executeKICK(Channel &channel, Client &client, const std::string& to_kick)
{
	Client* target = findClientByNick(to_kick);
	if (!target)
	{
		Replies(client.getFd(), ERR_NOSUCHNICK, to_kick + " :No such nick");
		return;
	}
	
	if (!channel.isUserInChannel(target))
	{
		Replies(client.getFd(), ERR_USERNOTINCHANNEL, to_kick + " " + channel.getName() + " :They aren't on that channel");
		return;
	}

	std::string kickMsg = ":" + client.getNickname() + " KICK " + channel.getName() + " " + to_kick + " :"  "Kicked" + "\r\n";
	broadcast(kickMsg, -1);
	
	channel.removeUser(target);

	std::cout << client.getNickname() << " kicked out " << to_kick << " from " << channel.getName() << "\n";
}
void Server::chanComments(Client &client, std::string &cmd, const std::vector<std::string>& args)
{
	if (args.empty())
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "Not enough parameters");
		return;
	}

	Channel* channel = findChannel(args[0]);

	if (!channel)
	{
		Replies(client.getFd(), ERR_NOSUCHCHANNEL, args[1] + " :No such channel");
		return;
	}

	if (cmd == "KICK")
	{
		if (args.size() < 3)
		{
			Replies(client.getFd(), ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
			return;
		}

		if (!channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, args[1] + " :You're not channel operator");
			return;
		}

		Client* target = findClientByNick(args[2]);
		if (!target)
		{
			Replies(client.getFd(), ERR_NOSUCHNICK, args[2] + " :No such nick");
			return;
		}
		executeKICK(*channel, client, args[2]);
	}
	if (cmd == "TOPIC")
	{
		if (args.empty())
		{
			Replies(client.getFd(), ERR_NEEDMOREPARAMS, "TOPIC :Channel name required");
			return;
		}

		std::string channelName = args[0];
		Channel* channel = findChannel(channelName);

		if (!channel)
		{
			Replies(client.getFd(), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
			return;
		}

		// Kanal operatör kontrolü (veya +t modu)
		if (channel->hasMode('t') && !channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
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
		std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
								" TOPIC " + channelName + " :" + newTopic + "\r\n";
		channel->broadcast(topicMsg, &client);
	}
	else if (cmd == "PART")
	{
		if (!client.isMemberOfChannel())
		{
			Replies(client.getFd(), ERR_NOTONCHANNEL, args[0] + " :You're not on that channel");
			return;
		}

		channel->removeUser(&client);
		std::string partMsg = ":" + client.getNickname() + " PART " + args[0] + "\r\n";
		send(client.getFd(), partMsg.c_str(), partMsg.length(), 0);
	}
	else if (cmd == "MODE")
	{
		if (!channel->isOperator(&client))
		{
			Replies(client.getFd(), ERR_CHANOPRIVSNEEDED, args[0] + " :You're not channel operator");
			return;
		}

		if (args.size() >= 2 && args[1] == "+o")
		{
			Client* target = findClientByNick(args[2]);
			if (target)
			{
				channel->addOperator(target);
				std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
				broadcast(modeMsg, client.getFd());
			}
		}
		if (args.size() >= 2 && args[1] == "-o")
		{
			Client* target = findClientByNick(args[2]);
			if (target)
			{
				channel->removeOperator(target);
				std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
				broadcast(modeMsg, client.getFd());
			}
		}
		// I DIDN'T DO THE REST MODES YET
		// if (args.size() >= 2 && args[1] == "+i")
		// {
		// 	Client* target = findClientByNick(args[2]);
		// 	if (target)
		// 	{
		// 		channel->removeOperator(target);
		// 		std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
		// 		broadcast(modeMsg, client.getFd());
		// 	}
		// }
		// if (args.size() >= 2 && args[1] == "-i")
		// {
		// 	Client* target = findClientByNick(args[2]);
		// 	if (target)
		// 	{
		// 		channel->removeOperator(target);
		// 		std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
		// 		broadcast(modeMsg, client.getFd());
		// 	}
		// }
		// if (args.size() >= 2 && args[1] == "+k")
		// {
		// 	Client* target = findClientByNick(args[2]);
		// 	if (target)
		// 	{
		// 		channel->removeOperator(target);
		// 		std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
		// 		broadcast(modeMsg, client.getFd());
		// 	}
		// }
		// if (args.size() >= 2 && args[1] == "-k")
		// {
		// 	Client* target = findClientByNick(args[2]);
		// 	if (target)
		// 	{
		// 		channel->removeOperator(target);
		// 		std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
		// 		broadcast(modeMsg, client.getFd());
		// 	}
		// }
		// if (args.size() >= 2 && args[1] == "+l")
		// {
		// 	Client* target = findClientByNick(args[2]);
		// 	if (target)
		// 	{
		// 		channel->removeOperator(target);
		// 		std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
		// 		broadcast(modeMsg, client.getFd());
		// 	}
		// }
		// if (args.size() >= 2 && args[1] == "-l")
		// {
		// 	Client* target = findClientByNick(args[2]);
		// 	if (target)
		// 	{
		// 		channel->removeOperator(target);
		// 		std::string modeMsg = ":" + client.getNickname() + " MODE " + args[1] + " +o " + args[3] + "\r\n";
		// 		broadcast(modeMsg, client.getFd());
		// 	}
		// }
	}
}