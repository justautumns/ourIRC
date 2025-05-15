/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 22:12:38 by mtrojano          #+#    #+#             */
/*   Updated: 2025/05/15 22:33:14 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name_) : name(name_) {
	this->invite_only = false;
	this->user_limit = false;
	this->pass_key = false;
}

void Channel::setPw(std::string pass)
{
	this->pw = pass;
	this->pass_key = true;
}

void Channel::setTopic(std::string topic_)
{
	this->topic = topic_;
}

void Channel::removePass()
{
	this->pw.clear();
	this->pass_key = false;
}
Channel::~Channel(){}
