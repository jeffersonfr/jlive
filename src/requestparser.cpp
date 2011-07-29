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
#include "requestparser.h"
#include "jstringtokenizer.h"
#include "jruntimeexception.h"
#include "jurl.h"
#include "jstringutils.h"

#include <sstream>

#include <strings.h>
#include <errno.h>

namespace mlive {

RequestParser::RequestParser(std::string s)
{
	jcommon::StringTokenizer lines(s, "\r\n", jcommon::JTT_STRING);
	jcommon::StringTokenizer request(lines.GetToken(0), " ", jcommon::JTT_STRING);

	if (request.GetSize() != 3) {
		throw jcommon::RuntimeException("Invalid request");
	}

	if (strcasecmp(request.GetToken(0).c_str(), "get") != 0) {
		throw jcommon::RuntimeException("Invalid request protocol");
	}

	jcommon::StringTokenizer url(request.GetToken(1), "?", jcommon::JTT_STRING);

	if (request.GetToken(1)[0] != '/') {
		throw jcommon::RuntimeException("Invalid url resource");
	}

	_vars["request_url"] = request.GetToken(1);

	std::string file_name = url.GetToken(0).substr(1, url.GetToken(0).size()-1);

	_vars["event_name"] = file_name;

	jcommon::StringTokenizer params(url.GetToken(1), "&", jcommon::JTT_STRING);

	for (int i=0; i<params.GetSize(); i++) {
		jcommon::StringTokenizer t(params.GetToken(i), "=", jcommon::JTT_STRING);

		if (t.GetSize() > 1 && t.GetToken(1) != "") {
			_vars[t.GetToken(0)] = t.GetToken(1);
		}
	}
}

RequestParser::~RequestParser()
{
}

std::string RequestParser::GetParameter(std::string key_)
{
	for (std::map<std::string, std::string>::iterator i=_vars.begin(); i!=_vars.end(); i++) {
		if (strcasecmp(i->first.c_str(), key_.c_str()) == 0) {
			return jcommon::URL::Decode(i->second, "ISO-8859-1"); 
		}
	}

	return "";
}

RequestParser::request_parser_type_t RequestParser::GetRequestType()
{
	if (GetParameter("type") == "info") {
		return INFO_REQUEST;
	} else if (GetParameter("type") == "getconfig") {
		return GETCONFIG_REQUEST;
	} else if (GetParameter("type") == "setconfig") {
		return SETCONFIG_REQUEST;
	} else if (GetParameter("type") == "stream") {
		return VIDEO_REQUEST;
	}

	return UNKNOWN_REQUEST; 
}

std::string RequestParser::GetSourceProtocol()
{
	jcommon::StringTokenizer params(GetParameter("source"), "-", jcommon::JTT_STRING);

	return jcommon::StringUtils::ToLower(params.GetToken(0));
}

std::string RequestParser::GetSourceHost()
{
	jcommon::StringTokenizer params(GetParameter("source"), "-", jcommon::JTT_STRING);

	if (params.GetSize() >= 2) {
		return params.GetToken(1);
	}

	return "";
}

int RequestParser::GetSourcePort()
{
	jcommon::StringTokenizer params(GetParameter("source"), "-", jcommon::JTT_STRING);

	if (params.GetSize() >= 3) {
		return atoi(params.GetToken(2).c_str());
	}

	return -1;
}

std::string RequestParser::GetDestinationProtocol()
{
	jcommon::StringTokenizer params(GetParameter("destination"), "-", jcommon::JTT_STRING);

	return jcommon::StringUtils::ToLower(params.GetToken(0));
}

std::string RequestParser::GetDestinationHost()
{
	jcommon::StringTokenizer params(GetParameter("destination"), "-", jcommon::JTT_STRING);

	if (params.GetSize() == 3) {
		return params.GetToken(1);
	}

	return "";
}

int RequestParser::GetDestinationPort()
{
	jcommon::StringTokenizer params(GetParameter("destination"), "-", jcommon::JTT_STRING);

	if (params.GetSize() == 3) {
		return atoi(params.GetToken(2).c_str());
	}

	return -1;
}

std::string RequestParser::GetEventName()
{
	return _vars["event_name"];
}

std::string RequestParser::GetResource()
{
	std::string r = _vars["resource"];

	if (r == "") {
		return "/";
	}

	return r;
}

std::string RequestParser::GetDestinationResource()
{
	jcommon::StringTokenizer params(GetParameter("source"), "-", jcommon::JTT_STRING);

	if (params.GetSize() == 3) {
		return GetResource();
	}

	if ((params.GetSize()-1) % 2 == 0) {
		std::string r = "/" + GetEventName() + "?source=" + GetSourceProtocol();

		if (r == "") {
			return GetResource();
		}

		for (int i=3; i<params.GetSize(); i++) {
			r = r + "-" + params.GetToken(i);
		}

		r = r + "&destination=" + GetParameter("destination") + "&type=" + GetParameter("type") + "&resource=" + GetResource();
	}

	return GetResource();
}

}
