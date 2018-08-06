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
#ifndef JLIVE_SOURCE_H
#define JLIVE_SOURCE_H

#include "client.h"

#include "jshared/jindexedbuffer.h"

#include <thread>

#include <unistd.h>
#include <stdio.h>

namespace mlive {

class RequestParser;
class Server;

/**
 * \brief Source.
 *
 * 	Abstract data source:
 *
 */
class Source {
	
	public:
		enum source_type_t {
			HTTP_SOURCE_TYPE,
			UDP_SOURCE_TYPE
		};

	private:
		std::vector<Client *> 
      clients;
		jshared::IndexedBuffer 
      *_buffer;
    std::thread
      _thread;
    std::mutex 
      _mutex;
		jnetwork::Connection 
      *_source;
		Server 
      *_server;
		std::string 
      _event,
		  _resource;
		source_type_t 
      _type;
    std::chrono::time_point<std::chrono::steady_clock>
      _start_time;
		uint64_t 
		  _sent_bytes;
		bool 
      _is_running;

	public:
		Source(std::string ip, int port, std::string source_name, source_type_t type, Server *server, std::string resource);
		virtual ~Source();

		std::vector<Client *> & GetClients();
		int IsClosed();
		void Release();
		int GetIncommingRate();
		int GetOutputRate();
		int GetNumberOfClients();
		jshared::IndexedBuffer * GetBuffer();
		std::string GetSourceName();
		bool AddClient(jnetwork::Socket *socket, RequestParser &parser);
		void ReadStream();
		void RemoveClients();
		void Stop();
		void Start();
		void Run();
		
};

}

#endif
