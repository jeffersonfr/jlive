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
#include "configuration.h"

#include "jcommon/jproperties.h"

#include <iostream>
#include <sstream>

#include <errno.h>

namespace mlive {

Configuration *Configuration::instance = NULL;

Configuration::Configuration()
{
	try {
		jcommon::Properties p("/etc/mlive/mlive.properties");

    p.Load();

		SetProperty("mlive-name", p.GetTextParam("mlive-name", "Mlive Server v0.01"));
		SetProperty("mlive-id", p.GetTextParam("mlive-id", "1234567890"));
		SetProperty("max-input-rate", p.GetTextParam("max-input-rate", "10000"));
		SetProperty("max-output-rate", p.GetTextParam("max-output-rate", "10000"));
		SetProperty("max-sources", p.GetTextParam("max-sources", "4"));
		SetProperty("max-source-clients", p.GetTextParam("max-source-clients", "4"));
		SetProperty("update-time", p.GetTextParam("update-time", "60"));
		SetProperty("buffer-size", p.GetTextParam("buffer-size", "256"));
		SetProperty("error-video", p.GetTextParam("error-video", ""));
		SetProperty("config-update", p.GetTextParam("config-update", "no"));
	} catch (...) {
		std::cout << "Configuration file mlive.properties not found" << std::endl;

		throw;
	}
}

Configuration::~Configuration()
{
}

Configuration * Configuration::GetInstance()
{
	if (instance == NULL) {
		instance = new Configuration();
	}

	return instance;
}

std::string Configuration::GetProperty(std::string key)
{
	return _properties[key];
}

void Configuration::SetProperty(std::string key, std::string value)
{
	_properties[key] = value;
}

std::map<std::string, std::string> & Configuration::GetProperties()
{
	return _properties;
}

}
