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
#ifndef JLIVE_SERVER_H
#define JLIVE_SERVER_H

#include "source.h"
#include "jthread.h"

#include <unistd.h>
#include <stdio.h>

namespace mlive {

/**
 * \brief Stream server. Listen request at port 3200 and response with a live stream.
 *
 * 	Stream request unicast:
 * 		http://host:port/event.xxx?source=<udp,http>-ip-port&destination=<udp-ip-port,http>&type=stream[&resource=/]
 *
 * 	Stream request multicast (application level):
 * 		http://host:port/event.xxx?source=<udp,http>-ip1-port1-ip2-port2-ip3-port3[...]&destination=<udp-ip-port,http>&type=stream[&resource=/]
 *
 * 	Webservice interface:
 * 		http://host:port/?type=info
 *
 * 	Set configuration:
 * 		http://host:port/?input-rate=256&output-rate=256&max-clients=4&max-sources=4&buffer-size=1024&update-time=2&type=setconfig
 *
 * 	Get configuration:
 * 		http://host:port/?type=getconfig
 *
 */
class Server : public jthread::Thread {
	private:
		enum current_server_status_t {
			HANDLEREQUESTS	= 0,
			REMOVECONTEXTS	= 1
		};

		current_server_status_t current;
		std::vector<Source *> sources;
		jthread::Mutex _mutex;
		int port;

	public:
		Server(int port);
		virtual ~Server();

		void HandleRequests();
		int GetNumberOfSources();
		bool ProcessClient(jsocket::Socket *socket, std::string receive);
		void RemoveSources();
		void Run();

};

}

#endif
