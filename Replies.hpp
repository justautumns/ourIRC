/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtrojano <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 01:18:50 by mehmeyil          #+#    #+#             */
/*   Updated: 2025/05/26 21:43:34 by mtrojano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLIES_HPP
#define REPLIES_HPP


#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_TOOMANYCHANNELS 405
#define ERR_TOOMANYTARGETS 407
#define ERR_NOORIGIN 409
#define ERR_NORECIPIENT 411
#define ERR_NOTEXTTOSEND 412
#define ERR_UNKNOWNCOMMAND 421
#define ERR_NONICKNAMEGIVEN 431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE 433
#define ERR_USERNOTINCHANNEL 441
#define ERR_NOTONCHANNEL 442
#define ERR_USERONCHANNEL 443
#define ERR_NOTREGISTERED 451
#define ERR_NEEDMOREPARAMS 461
#define ERR_ALREADYREGISTERED 462
#define ERR_PASSWDMISMATCH 464
#define ERR_CHANNELISFULL 471
#define ERR_INVITEONLYCHAN 473
#define ERR_BADCHANNELKEY 475
#define ERR_BADCHANMASK 476
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_UNKNOWNMODE 472
// Server Replies (RFC 1459 Section 6.1)
#define RPL_MYINFO 004
#define RPL_ISUPPORT 005

// Channel Related (RFC 1459 Section 6.2)
#define RPL_TOPIC 332  // Kanal konusu gönderildiğinde
#define RPL_TOPICWHOTIME 333  // Konuyu kimin ne zaman değiştirdiği (opsiyonel)
#define RPL_NAMREPLY 353  // Kanaldaki kullanıcı listesi
#define RPL_ENDOFNAMES 366  // Kullanıcı listesi sonu
#define RPL_INVITING 341 // confirmation that the invitation to a channel has been sent
#define RPL_CHANNELMODEIS 324 // replying for available channel modes

// Error Replies (RFC 1459 Section 6.3)
#define ERR_NOSUCHCHANNEL   403  // Kanal bulunamadı
#define ERR_TOOMANYCHANNELS 405  // Çok fazla kanala katılım
#define ERR_BADCHANNELKEY   475  // Yanlış kanal şifresi
#define ERR_CHANNELISFULL   471  // Kanal dolu
#define ERR_INVITEONLYCHAN  473  // Sadece davetli kanal


#endif
