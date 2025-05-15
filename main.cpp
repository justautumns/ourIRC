/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 16:16:05 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/15 21:58:33 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


int checkPortisValid(std::string &port)
{
	for (size_t i = 0; i < port.size(); i++)
	{
		if (port[0] == '0')
			return (-1);
		if (!isdigit(port[i]))
			return (-1);
	}
	std::stringstream ss;
	ss << port;
	long k;
	ss >> k;
	if (k > std::numeric_limits<int>::max() || k < std::numeric_limits<int>::min())
		return (-1);
	int portint = static_cast<int>(k);
	if (portint < 1024 || portint > 65535)
		return (-1);
	return (portint);
}

int main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::runtime_error("Arguments have to be port passowrd");
		else
		{
			std::string pass(av[2]);
			std::string port(av[1]);
			int portint = checkPortisValid(port);
			if (portint == -1)
				throw std::runtime_error("Invalid port detected");
			Server server(portint, pass);
			server.startServer();
			server.Routine();
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}
