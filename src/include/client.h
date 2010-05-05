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
#ifndef JLIVE_CLIENT_H
#define JLIVE_CLIENT_H

#include "jcondition.h"
#include "jmutex.h"
#include "jthread.h"
#include "jsocket.h"

#include <unistd.h>
#include <stdio.h>

namespace mlive {

class Source;

/**
 * \brief Client.
 *
 * 	Handle clients.
 *
 */
class Client : public jthread::Thread {
	
	public:
		enum client_type_t {
			HTTP_CLIENT_TYPE,
			UDP_CLIENT_TYPE
		};

	private:
		jsocket::Connection *request,
			*response;
		Source *source;
		client_type_t type;
		jthread::Mutex mutex;
		int read_index,
				pass_index;
		long long start_time;
		long long sent_bytes;
		bool _running,
				 _is_closed;

	public:
		Client(jsocket::Socket *socket, Source *source);
		Client(jsocket::Socket *socket, std::string ip, int port, Source *source);
		virtual ~Client();

		jsocket::Connection * GetConnection();
		client_type_t GetType();
		void Release();
		bool IsClosed();
		int GetOutputRate();
		long long GetStartTime();
		long long GetSentBytes();
		void ProcessHTTPClient();
		void ProcessUDPClient();
		void Stop();
		virtual void Run();
		
};

}

#endif
