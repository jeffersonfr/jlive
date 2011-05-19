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
#include "server.h"
#include "joptions.h"
#include "jsocketlib.h"

#include <sstream>

#include <errno.h>

void help()
{
	std::cout << "./mlive [-h] [-p <port:1024..65535>] [-a] [-v]" << std::endl;

	exit(0);
}

int main(int argc, char **argv)
{
	InitWindowsSocket();

	jcommon::Options o(argc, argv);

	o.SetOptions("hp:");

	// default port
	int port = 3200;

	if (o.ExistsOption("h") == true) {
		help();
	}

	if (o.ExistsOption("p") == true) {
		port = atoi(o.GetArgument("p").c_str());

		if (port < 1024 || port > 65535) {
			help();
		}
	}

	try {
		mlive::Server server(port);

		server.Start();
		server.WaitThread();
	} catch (...) {
		std::cout << "jLive Stream Closed" << std::endl;
	}

	ReleaseWindowsSocket();

	return 0;
}


