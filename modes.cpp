#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"


void Server::executeModes(Client &client, const std::vector<std::string>& args, Channel *channel)
{
	// need params +-o +k +l
	// no params i t
	// args[0] = channel name
	// args[1] = mode
	// args[2.......] parameters
	char prefix = ':';
	char current_mode = 'z';
	std::string known_modes = "itkol";
	std::vector<std::string> params;

	if ((args[1].size() < 2) || (args[1][0] != '+' && args[1][0] != '-'))
	{
		Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " Not enough parameters");
		return;
	}

	for (size_t i = 2; i < args.size(); i++)
		params.push_back(args[i]);

	for (size_t i = 0; i < args[1].size(); i++)
	{
		if (args[1][i] == '+' || args[1][i] == '-')
		{
			prefix = args[1][i];
			continue;
		}

		current_mode = args[1][i];
		if (known_modes.find_first_of(current_mode) != std::string::npos)
		{
			if (current_mode == 'o')
			{
				if (params.empty())
					Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " Not enough parameters");
				else
				{
					prefix == '+' ? mode_Plus_O(client, params[0], channel)
								: mode_Minus_O(client, params[0], channel);
					params.erase(params.begin());
				}
			} 
		
			if (current_mode == 'k')
			{
				if (prefix == '+' && params.empty())
					Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " Not enough parameters");
				else
				{
					if (prefix == '+')
					{
						mode_Plus_K(client, params[0], channel);
						params.erase(params.begin());
					}
					else
						mode_Minus_K(client, channel);
				}
			}

			if (current_mode == 'l')
			{
				if (prefix == '+' && params.empty())
					Replies(client.getFd(), ERR_NEEDMOREPARAMS, client.getNickname() + " Not enough parameters");
				else
				{
					if (prefix == '+')
					{
						mode_Plus_L(client, params[0], channel);
						params.erase(params.begin());
					}
					else
						mode_Minus_L(client, channel);
				}
			}

			if (current_mode == 'i')
				prefix == '+' ? mode_Plus_I(client, channel) : mode_Minus_I(client, channel);
			
			if (current_mode == 't')
				prefix == '+' ? mode_Plus_T(client, channel) : mode_Minus_T(client, channel);
		}
		else
			Replies(client.getFd(), ERR_UNKNOWNMODE, client.getNickname() + " " + current_mode + " :is unknown mode char to me");
	}
}

void Server::mode_Plus_O(Client &client, std::string param, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	Client* target = findClientByNick(param);
	if (target)
	{
		if ((target->getNickname() == client.getNickname()) || (channel->isOperator(target)))
			return;
		channel->addOperator(target);
		std::string modeMsg = client_info + " MODE " + channel->getName() + " +o " + target->getNickname() + "\r\n";
		channel->broadcast(modeMsg);
		return;
	}
	Replies(client.getFd(), ERR_NOSUCHNICK, client.getNickname() + " " + param + " :No such nick");
}

void Server::mode_Minus_O(Client &client, std::string param, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	Client* target = findClientByNick(param);
	if (target)
	{
		if ((target->getNickname() == client.getNickname()) || (!channel->isOperator(target)))
			return;
		channel->removeOperator(target);
		std::string modeMsg = client_info + " MODE " + channel->getName() + " -o " + target->getNickname() + "\r\n";
		channel->broadcast(modeMsg);
		return;
	}
	Replies(client.getFd(), ERR_NOSUCHNICK, client.getNickname() + " " + param + " :No such nick");
}

void Server::mode_Plus_I(Client &client, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->setModes('i');
	std::string modeMsg = client_info + " MODE " + channel->getName() + " +i" + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Minus_I(Client &client, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->removeMode('i');
	std::string modeMsg = client_info + " MODE " + channel->getName() + " -i" + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Plus_K(Client &client, std::string param, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->setModes('k');
	channel->setPw(param);
	std::string modeMsg = client_info + " MODE " + channel->getName() + " +k" + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Minus_K(Client &client, Channel *channel) 
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->removePass();
	channel->removeMode('k');
	std::string modeMsg = client_info + " MODE " + channel->getName() + " -k" + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Plus_L(Client &client, std::string param, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	for (size_t i = 0; i < param.size(); i++)
	{
		if (i == 0 && param[i] == '+')
			continue;
		if (!isdigit(param[i]))
		{
			std::stringstream error;
			error << ":" << serverName << " Parameter of MODE +l command has to be a positive integer\r\n";
			send(client.getFd(), error.str().c_str(), error.str().length(), 0);
			return;
		}
	}

	if (param.length() > 3)
	{
		std::stringstream error;
		error << ":" << serverName << " The max number of clients you can set is " << MAX_CLIENT << " \r\n";
		send(client.getFd(), error.str().c_str(), error.str().length(), 0);
		return;
	}

	if (std::atoi(param.c_str()) > MAX_CLIENT)
	{
		std::stringstream error;
		error << ":" << serverName << " The max number of clients you can set is " << MAX_CLIENT << " \r\n";
		send(client.getFd(), error.str().c_str(), error.str().length(), 0);
		return;
	}

	channel->setModes('l');
	channel->setUserLimit(std::atoi(param.c_str()));
	std::string modeMsg = client_info + " MODE " + channel->getName() + " +l " + param + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Minus_L(Client &client, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->removeMode('l');
	std::string modeMsg = client_info + " MODE " + channel->getName() + " -l" + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Plus_T(Client &client, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->setModes('t');
	std::string modeMsg = client_info + " MODE " + channel->getName() + " +t" + "\r\n";
	channel->broadcast(modeMsg);
}

void Server::mode_Minus_T(Client &client, Channel *channel)
{
	std::string client_info = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
	channel->removeMode('t');
	std::string modeMsg = client_info + " MODE " + channel->getName() + " -t" + "\r\n";
	channel->broadcast(modeMsg);
}
