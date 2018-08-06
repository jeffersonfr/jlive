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
#include "client.h"
#include "source.h"

#include "jnetwork/jdatagramsocket.h"
#include "jexception/jruntimeexception.h"

#include <sstream>

namespace mlive {

Client::Client(jnetwork::Socket *socket, Source *source)
{
	if ((void *)socket == NULL) {
		throw jexception::RuntimeException("Invalid reference to socket");
	}

	this->source = source;
	request = dynamic_cast<jnetwork::Connection *>(socket);
	type = HTTP_CLIENT_TYPE;
  start_time = std::chrono::steady_clock::now();
  sent_bytes = 0LL;

	this->response = dynamic_cast<jnetwork::Connection *>(socket);
}

Client::Client(jnetwork::Socket *socket, std::string ip, int port, Source *source)
{
	if ((void *)socket == NULL) {
		throw jexception::RuntimeException("Invalid reference to socket");
	}

	this->source = source;
	type = UDP_CLIENT_TYPE;
	request = dynamic_cast<jnetwork::Connection *>(socket);
  start_time = std::chrono::steady_clock::now();
  sent_bytes = 0LL;

	this->response = dynamic_cast<jnetwork::Connection *>(new jnetwork::DatagramSocket(ip, port, true));
}

Client::~Client()
{
  Stop();
}

Client::client_type_t Client::GetType()
{
	return type;
}

jnetwork::Connection * Client::GetConnection()
{
	return response;
}

uint64_t Client::GetStartTime()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch()).count();
}

uint64_t Client::GetSentBytes()
{
	return sent_bytes;
}

void Client::Release()
{
	try {
		if (request != response) {
			if ((void *)request != NULL) {
				request->Close();
				delete request;
			}

			if ((void *)response != NULL) {
				response->Close();
				delete response;
			}
		} else {
			if ((void *)request != NULL) {
				request->Close();
				delete request;
			}
		}

		request = NULL;
		response = NULL;
	} catch (...) {
	}
}

bool Client::IsClosed()
{
	return _is_running == false;
}

int Client::GetOutputRate()
{
  std::chrono::time_point<std::chrono::steady_clock>
    current = std::chrono::steady_clock::now();
  uint64_t
    elapsed = std::chrono::duration_cast<std::chrono::microseconds>((current - start_time)).count();
	uint64_t 
		rate = (sent_bytes*8LL)/(elapsed);

	return (int)(rate);
}

void Client::Stop()
{
  if (_is_running == false) {
    return;
  }

	_is_running = false;
	
	// source->GetBuffer()->Write((const uint8_t *)"\0", 1);

  _thread.join();

	Release();
}

void Client::Start()
{
  if (_is_running == true) {
    return;
  }

  _thread = std::thread(&Client::Run, this);
}

void Client::ProcessHTTPClient() 
{
	{
		// sending response to client
		std::ostringstream o;

		o << "HTTP/1.0 200 OK" << "\r\n";
		o << "Server: Rex/9.0.0.2980" << "\r\n";
		o << "Cache-Control: no-cache" << "\r\n";
		o << "Pragma: no-cache" << "\r\n";
		o << "Pragma: client-id=26500" << "\r\n";
		o << "Pragma: features=\"broadcast\"" << "\r\n";
		o << "Content-Type: video/mpeg" << "\r\n";
		o << "Connection: close" << "\r\n";
		o << "\r\n";

		if (response->GetOutputStream()->Write((const char *)o.str().c_str(), o.str().size()) < 0) {
			return;
		}

		if (response->GetOutputStream()->Flush() < 0) {
			return;
		}
	}

	jshared::IndexedBuffer *buffer = source->GetBuffer();
	jio::OutputStream *o = response->GetOutputStream();
	jshared::jbuffer_chunk_t data;
	int r;

	buffer->GetIndex(&data);

	do {
		// WARNNING:: a excecao lancada pelo wait estah causando FATAL:: not rethrow
		r = buffer->Read(&data);

		if (r < 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds((100)));

			buffer->GetIndex(&data);
		} else {
			if (o->Write((const char *)data.data, data.size) < 0) {
				break; 
			}

			sent_bytes += data.size;
		}
	} while (_is_running == true);
}

void Client::ProcessUDPClient() 
{
	jshared::IndexedBuffer *buffer = source->GetBuffer();
	jshared::jbuffer_chunk_t data;
	jio::OutputStream *o = response->GetOutputStream();
	int r;

	buffer->GetIndex(&data);

	do {
		r = buffer->Read(&data);

		if (r < 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds((100)));

			buffer->GetIndex(&data);
		} else {
			if (o->Write((const char *)data.data, data.size) < 0) {
				break; 
			}

			sent_bytes += data.size;
		}
	} while (_is_running == true);
}

void Client::Run() 
{
	_is_running = true;

	if (type == HTTP_CLIENT_TYPE) {
		ProcessHTTPClient();
	} else if (type == UDP_CLIENT_TYPE) {
		ProcessUDPClient();
	}

	_is_running = false;
}

}
