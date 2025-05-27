/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 00:09:13 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/27 14:36:27 by mehmeyil         ###   ########.fr       */
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
	bool isOnline;
	int fd;
	bool got_cap_end;
	std::string nickname;
	std::string username;
	std::string realname;
	std::string hostname;
	std::string buffer;
	std::vector<std::string> joinedChannels;
	std::vector<std::string> invitations;
	time_t last_activity;
	bool waiting_for_pong;  // PING gönderildi ama PONG alınmadı durumu
	time_t last_ping_time;
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
	bool getIsOnline() const;
	bool getHasPassword() const;
	bool getIsRegistered() const;
	std::vector<std::string> &getJoinedChannelsName();
	int getInvalidPasswordAttempt();
	time_t getLastActivity();
	time_t getLastPingTime() const;
	bool hasInvitation(std::string channel_name);
	bool isWaitingForPong() const;
	
	// Setters
	void setNickname(const std::string& nick);
	void setUserInfo(const std::string& user, const std::string& real);
	void setPassword(bool status);
	void setRegistered(bool status);
	void setisOnline(bool status);
	void updateLastActivity();
	void setInvalidPasswordAttempt();
	void addJoinedChannel(std::string channel_name);
	void removeJoinedChannel(std::string channel_name);
	void addInvitation(std::string channel_name);
	void removeInvitation(std::string channel_name);
	void setWaitingForPong(bool status);
	void setLastPingTime(time_t time);


	// Buffer management
	void appendToBuffer(const std::string& data);
	void clearBuffer();
	const std::string& getBuffer() const;
	void eraseFromBuffer(size_t pos, size_t len); 
	
	// Registration check
	bool canRegister() const;
	bool shouldDisconnect() const;
	bool is_cap_end_received();
	void set_cap_end(bool status);
	
	~Client();
};

#endif
