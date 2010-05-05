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
#include "jruntimeexception.h"
#include "jdate.h"
#include "jdatagramsocket.h"

#include <sstream>

#include <errno.h>

namespace mlive {

Client::Client(jsocket::Socket *socket, Source *source):
	jthread::Thread()	
{
	if ((void *)socket == NULL) {
		throw jcommon::RuntimeException("Invalid reference to socket");
	}

	this->source = source;
	read_index = 0;
	pass_index = 0;
	request = dynamic_cast<jsocket::Connection *>(socket);
	type = HTTP_CLIENT_TYPE;
	start_time = jcommon::Date::CurrentTimeMillis(), sent_bytes = 0LL;
	_is_closed = false;

	this->response = dynamic_cast<jsocket::Connection *>(socket);
}

Client::Client(jsocket::Socket *socket, std::string ip, int port, Source *source):
	jthread::Thread()	
{
	if ((void *)socket == NULL) {
		throw jcommon::RuntimeException("Invalid reference to socket");
	}

	this->source = source;
	read_index = 0;
	pass_index = 0;
	type = UDP_CLIENT_TYPE;
	request = dynamic_cast<jsocket::Connection *>(socket);
	start_time = jcommon::Date::CurrentTimeMillis(), sent_bytes = 0LL;
	_is_closed = false;

	this->response = dynamic_cast<jsocket::Connection *>(new jsocket::DatagramSocket(ip, port, true));
}

Client::~Client()
{
}

Client::client_type_t Client::GetType()
{
	return type;
}

jsocket::Connection * Client::GetConnection()
{
	return response;
}

long long Client::GetStartTime()
{
	return start_time;
}

long long Client::GetSentBytes()
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
	return _is_closed;
}

int Client::GetOutputRate()
{
	long long current = jcommon::Date::CurrentTimeMillis(),
		 rate = (sent_bytes*8LL)/(current-start_time);

	return (int)(rate);
}

void Client::Stop()
{
	_running = false;
	
	// source->GetBuffer()->Write((const uint8_t *)"\0", 1);

	Interrupt();
	WaitThread();
	Release();
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

	jthread::IndexedBuffer *buffer = source->GetBuffer();
	jio::OutputStream *o = response->GetOutputStream();
	jthread::jringbuffer_t data;
	int r;

	buffer->GetIndex(&read_index, &pass_index);

	do {
		// WARNNING:: a excecao lancada pelo wait estah causando FATAL:: not rethrow
		r = buffer->Read(&data, &read_index, &pass_index);

		if (r < 0) {
			Thread::Sleep(100);

			buffer->GetIndex(&read_index, &pass_index);
		} else {
			if (o->Write((const char *)data.data, data.size) < 0) {
				break; 
			}

			sent_bytes += data.size;
		}
	} while (_running == true);
}

void Client::ProcessUDPClient() 
{
	jthread::IndexedBuffer *buffer = source->GetBuffer();
	jio::OutputStream *o = response->GetOutputStream();
	jthread::jringbuffer_t data;
	int r;

	buffer->GetIndex(&read_index, &pass_index);

	do {
		r = buffer->Read(&data, &read_index, &pass_index);

		if (r < 0) {
			Thread::Sleep(100);

			buffer->GetIndex(&read_index, &pass_index);
		} else {
			if (o->Write((const char *)data.data, data.size) < 0) {
				break; 
			}

			sent_bytes += data.size;
		}
	} while (_running == true);
}

void Client::Run() 
{
	_running = true;

	if (type == HTTP_CLIENT_TYPE) {
		ProcessHTTPClient();
	} else if (type == UDP_CLIENT_TYPE) {
		ProcessUDPClient();
	}

	_running = false;
	_is_closed = true;
}

}
