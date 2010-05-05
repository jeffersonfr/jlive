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
#ifndef JLIVE_CONFIGURATION_H
#define JLIVE_CONFIGURATION_H

#include <string>
#include <map>

#include <unistd.h>
#include <stdio.h>

namespace mlive {

/**
 * \brief Configuration.
 *
 * 	Abstract global configuration.
 *
 */
class Configuration{
	
	private:
		static Configuration *instance;

		std::map<std::string, std::string> _properties;

	public:
		Configuration();
		virtual ~Configuration();

		static Configuration * GetInstance();

		void SetProperty(std::string key, std::string value);
		std::string GetProperty(std::string key);
		std::map<std::string, std::string> & GetProperties();
		
};

}

#endif
