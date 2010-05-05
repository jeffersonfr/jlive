/***************************************************************************
 *   Copyright (C) 2005 by Jeff Ferr                                       *
 *   root@sat                                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef JLIVE_REQUESTPARSER_H
#define JLIVE_REQUESTPARSER_H

#include <string>
#include <map>

#include <unistd.h>
#include <stdio.h>

namespace mlive {

/**
 * \brief Request parser.
 *
 *
 */
class RequestParser{

	private:
		std::map<std::string, std::string> _vars;

	public:
		enum request_parser_type_t {
			VIDEO_REQUEST,
			INFO_REQUEST,
			GETCONFIG_REQUEST,
			SETCONFIG_REQUEST,
			UNKNOWN_REQUEST
		};

	public:
		RequestParser(std::string s);
		~RequestParser();

		std::string GetParameter(std::string key_);
		request_parser_type_t GetRequestType();
		std::string GetSourceProtocol();
		std::string GetSourceHost();
		int GetSourcePort();
		std::string GetDestinationProtocol();
		std::string GetDestinationHost();
		int GetDestinationPort();
		std::string GetEventName();
		std::string GetResource();
		std::string GetDestinationResource();

};

}

#endif
