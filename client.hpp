/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehmeyil <mehmeyil@student.42vienna.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 00:09:13 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/04/26 14:35:15 by mehmeyil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "server.hpp"
#include <vector>
#include <iostream>
class Client
{
	private :
	int	cFd;
	std::string nick;
	std::string user;
	std::string infos;
	std::string cbuffer;
	std::vector<std::string> ccommands;
	bool isAuth;
	public :
	Client(int fd);
	int getFd() const;
	void setNick(const std::string& nick);
	const std::string& getNick() const;
	void authenticate();
	bool isAuthenticated() const;
	~Client();
};
#endif
