/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 00:09:13 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/19 21:59:01 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include "Server.hpp"
#include <ctime>

class Client 
{
	private:
	int fd;
	std::string nickname;
	std::string username;
	std::string realname;
	std::string hostname;
	std::string buffer;
	std::vector<std::string> joinedChannels;
	std::vector<std::string> invitations;
	time_t last_activity;
	int invalid_password_attempts;
	bool hasPassword;
	bool isRegistered;
	bool isInChannel;

	public:
	Client(int fd);
	
	// Getters
	int getFd() const;
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;
	const std::string& getHostname() const;
	bool getHasPassword() const;
	bool getIsRegistered() const;
	// bool isMemberOfChannel() const;
	std::vector<std::string> &getJoinedChannelsName();
	bool hasInvitation(std::string channel_name);
	
	// Setters
	void setNickname(const std::string& nick);
	void setUserInfo(const std::string& user, const std::string& real);
	void setPassword(bool status);
	void setRegistered(bool status);
	void updateLastActivity();
	void setInvalidPasswordAttempt();
	void addJoinedChannel(std::string channel_name);
	void addInvitation(std::string channel_name);

	// Buffer management
	void appendToBuffer(const std::string& data);
	void clearBuffer();
	const std::string& getBuffer() const;
	void eraseFromBuffer(size_t pos, size_t len); 
	
	// Registration check
	bool canRegister() const;
	bool shouldDisconnect() const;
	
	~Client();
};

#endif
