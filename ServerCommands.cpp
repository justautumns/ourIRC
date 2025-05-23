/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 14:15:57 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/23 03:19:22 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"


void Server::capHandle(Client &client, const std::vector<std::string>& args)
{
	if (!args.empty() && args[0] == "LS")
	{
		std::string reply = "CAP * LS :\r\n";
		send(client.getFd(), reply.c_str(), reply.size(), 0);
	}
	if (!args.empty() && args[0] == "END")
		client.set_cap_end(true);
}
void Server::passHandle(Client &client, const std::vector<std::string>& args)
{
	if (client.getIsRegistered())
	{
		Replies(client.getFd(), ERR_ALREADYREGISTERED, ":You may not reregister");
		return;
	}

	if (args.empty())
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return;
	}
	if (args[0].find(' ') != std::string::npos)
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "PASS :Password may not contain spaces");
		return;
	}
	if (args[0] == passwd)
	{
		client.setPassword(true);
		// Replies(client.getFd(), RPL_WELCOME, ":Password accepted");
		std::string reply = "Password accepted\r\n";
		send(client.getFd(), reply.c_str(), reply.size(), 0);
	} 
	else
	{
		Replies(client.getFd(), ERR_PASSWDMISMATCH, ":Password incorrect");
		// We have to find and disconnect this client
		for (size_t i = 0; i < fd_polls.size(); ++i)
		{
			if (fd_polls[i].fd == client.getFd())
			{
				removeClient(i);
				break;
			}
		}
	}
}

void Server::nickNameHandle(Client& client, const std::vector<std::string> &args)
{
	if (args.empty())
	{
		Replies(client.getFd(), ERR_NONICKNAMEGIVEN, ":No nickname given");
		return;
	}

	const std::string &newNick = args[0];

	// Nickname validation according to RFC 1459 it's really gettin started to be pain to check this documentary :/
	// BTW this Replies was also in these documentary
	if (newNick.empty() || newNick.length() > 9)
	{
		Replies(client.getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		return;
	}

	// This chars are considered as invalid in a nickname
	if (newNick.find_first_of(" ,*?!@.") != std::string::npos)
	{
		Replies(client.getFd(), ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname (a nickname cannot contain these charecters ;  ,*?!@.)");
		return;
	}

	// Checking if nickname is already in use
	for (size_t i = 0; i < cls.size(); ++i)
	{
		if (cls[i]->getNickname() == newNick && cls[i]->getFd() != client.getFd())
		{
			Replies(client.getFd(), ERR_NICKNAMEINUSE, client.getNickname() + " " + newNick + " :Nickname is already in use");
			return;
		}
	}

	std::string old_nick = client.getNickname();
	client.setNickname(newNick);
	if (client.getIsRegistered())
	{
		std::string msg = ":" + old_nick + "!" + client.getUsername() + "@" + client.getHostname() + " NICK :" + newNick + "\r\n";
		std::vector<std::string> channels = client.getJoinedChannelsName();
		if (channels.size() != 0)
		{
			for (size_t i = 0; i < channels.size(); i++)
			{
				Channel *cur_chan = findChannel(channels[i]);
				if (cur_chan)
					cur_chan->broadcast(msg);
			}
		}
		else
			send(client.getFd(), msg.c_str(), msg.size(), 0);
	}

	// /connect localhost 3555 pass
	// If all goes well it means registiration was a success and we send a welcome msg
	if (!client.getIsRegistered() && client.canRegister())
	{
		client.setRegistered(true);
		// Replies(client.getFd(), RPL_WELCOME, ":Welcome to the Internet Relay Network " + client.getNickname());
	}
}

void Server::userHandle(Client &client, const std::vector<std::string>& args)
{
	if (client.getIsRegistered())
	{
		Replies(client.getFd(), ERR_ALREADYREGISTERED, ":You may not reregister");
		return;
	}

	if (args.size() < 4)
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		return;
	}

	const std::string &username = args[0];
	// args[1] and args[2] are (hostname ve servername) in IRC it's kind of ignored, so user dont have to set those things 
	// Therefore we pass 0 * hostname is 0 and server name is * but we can also set these things IDK.
	// for realname I add this loop to grep spaces in between names.
	std::string realname = args[3];
	for (size_t i = 4; i < args.size(); ++i)
		realname += " " + args[i];

	// But in username it has to be one word so..
	if (username.empty() || username.find(' ') != std::string::npos)
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "USER :Invalid username");
		return;
	}

	client.setUserInfo(username, realname);

	if (!client.getIsRegistered() && client.canRegister())
	{
		client.setRegistered(true);
		Replies(client.getFd(), RPL_WELCOME, client.getNickname() + " :Welcome to the Internet Relay Network " + client.getNickname());
		Replies(client.getFd(), RPL_YOURHOST, client.getNickname() + " :Your host is " + serverName);
		Replies(client.getFd(), RPL_CREATED, client.getNickname() + " :This server was created by Emre and Michal at: " + getCreationTime());
		Replies(client.getFd(), RPL_MYINFO, client.getNickname() + " " + serverName + " 1.0 usrmd lktoi");
		// std::stringstream reply;
		// reply << ":" << serverName << " MODE " << client.getNickname() << " +i \r\n";
		// send(client.getFd(), reply.str().c_str(), reply.str().length(), 0);
		// Replies(client.getFd(), 422, client.getNickname() + " :MOTD File is missing");
	}
}


void Server::joinHandle(Client &client, const std::vector<std::string>& args)
{
	if (args.empty())
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
		return;
	}
	
	if (!client.getIsRegistered())
	{
		if (client.is_cap_end_received())
			Replies(client.getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	std::string channelName = args[0];
	if (channelName[0] != '#')
	{
		Replies(client.getFd(), ERR_BADCHANNELKEY, channelName + " :Bad Channel Mask");
		return;
	}

	// Kanalı bul veya oluştur
	Channel* channel = findOrCreateChannel(channelName);

	if (channel->hasMode('i') && !client.hasInvitation(channel->getName()))
	{
		std::string inform_msg = "You need an invitaion to join this channel\r\n";
		send(client.getFd(), inform_msg.c_str(), inform_msg.length(), 0);
		return;
	}

	channel->addUser(&client);
	client.addJoinedChannel(channelName);

	// 1. JOIN mesajını gönder (RFC 1459 uyumlu)
	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " JOIN :" + channelName + "\r\n";
	channel->broadcast(joinMsg);

	// 2. Kanal konusu (RPL_TOPIC 332) // THIS IS PROBABLY NOT COMPLETED.
	if (!channel->getTopic().empty())
	{
		Replies(client.getFd(), RPL_TOPIC, channel->getName() + " :" + channel->getTopic());
	}

	// 3. Kullanıcı listesi (RPL_NAMREPLY 353)
	std::string userList;
	const std::vector<Client*>& users = channel->getUsers();
	if (users.size() == 1)
		channel->addOperator(&client);
	for (size_t i = 0; i < users.size(); ++i)
	{
		if (i > 0) userList += " ";
		userList += std::string(channel->isOperator(users[i]) ? "@" : "") + users[i]->getNickname();
	}
	Replies(client.getFd(), RPL_NAMREPLY, client.getNickname() + " = " + channelName + " :" + userList);
	Replies(client.getFd(), RPL_ENDOFNAMES, client.getNickname() + " " + channelName + " :End of NAMES list");
}

void Server::privmsgHandle(Client &client, const std::vector<std::string>& args)
{
	if (!client.getIsRegistered()) {
		Replies(client.getFd(), ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	if (args.size() < 2) {
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
		return;
	}

	const std::string &target = args[0];
	std::string message;

	for (size_t i = 1; i < args.size(); ++i)
	{
		if (i > 1) message += " ";
		message += args[i];
	}

	if (target[0] == '#')
	{
		Channel* channel = findChannel(target);
		if (!channel)
		{
			Replies(client.getFd(), ERR_NOSUCHCHANNEL, target + " :No such channel");
			return;
		}

		if (!channel->isUserInChannel(&client))
		{
			Replies(client.getFd(), ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
			return;
		}

		std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + 
							" PRIVMSG " + target + " :" + message + "\r\n";
		channel->broadcast1(msg, &client);
	}
	else
	{
		Client* targetClient = NULL;
		for (size_t i = 0; i < cls.size(); ++i)
		{
			if (cls[i]->getNickname() == target)
			{
				targetClient = cls[i];
				break;
			}
		}

		if (!targetClient)
		{
			Replies(client.getFd(), ERR_NOSUCHNICK, target + " :No such nick/channel");
			return;
		}

		std::string msg = ":" + client.getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		send(targetClient->getFd(), msg.c_str(), msg.length(), 0);
	}
}
void Server::quitHandle(Client &client, const std::vector<std::string>& args)
{
	std::vector<std::string> aa = client.getJoinedChannelsName();
	for (size_t i = 0; i < aa.size(); ++i)
	{
		Channel *channel = findChannel(aa[i]);
		if (channel)
		{
			channel->removeUser(&client);
			std::string partMsg = ":" + client.getNickname() + " PART " + args[0] + "\r\n";
			send(client.getFd(), partMsg.c_str(), partMsg.length(), 0);
		}
	}
	for (size_t i = 0; i < fd_polls.size(); ++i)
	{
		if (fd_polls[i].fd == client.getFd())
		{
			std::string quitMsg = ":" + client.getNickname() + " QUIT :" + (args.empty() ? "Client quit" : args[0]) + "\r\n";
			broadcast(quitMsg, client.getFd());
			removeClient(i);
		}
	}
}

void Server::pingHandle(Client &client, const std::vector<std::string>& args)
{
	if (args.empty())
	{
		Replies(client.getFd(), ERR_NOORIGIN, ":No origin specified");
		return;
	}

	// Örnek: PING :123456789 → PONG sunucu_adı :123456789
	// std::string pongMsg = ":" + serverName + " PONG " + serverName + " " + args[0] + "\r\n";
	std::string pongMsg = ":" + serverName + " PONG " + args[0] + "\r\n";
	send(client.getFd(), pongMsg.c_str(), pongMsg.length(), 0);

	// Just to check event activity
	client.updateLastActivity();
}

void Server::pongHandle(Client &client, const std::vector<std::string>& args)
{
	(void)args;
	client.updateLastActivity();
}
