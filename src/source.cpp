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
#include "source.h"
#include "server.h"
#include "requestparser.h"
#include "configuration.h"

#include "jnetwork/jdatagramsocket.h"
#include "jexception/jruntimeexception.h"

#include <iostream>
#include <sstream>

#include <errno.h>

namespace mlive {

Source::Source(std::string ip, int port, std::string source_name, source_type_t type, Server *server, std::string resource)
{
	if ((void *)socket == NULL) {
		throw jexception::RuntimeException("Invalid reference to socket");
	}

	_sent_bytes = 0LL;
	_server = server;
	_resource = resource;
	_type = type;
	_event = source_name;

	_buffer = new jshared::IndexedBuffer(atoi(Configuration::GetInstance()->GetProperty("buffer-size").c_str()), 4096);

	if (_type == HTTP_SOURCE_TYPE) {
		_source = dynamic_cast<jnetwork::Connection *>(new jnetwork::Socket(ip, port));
	} else {
		_source = dynamic_cast<jnetwork::Connection *>(new jnetwork::DatagramSocket(port));
	}

	_start_time = std::chrono::steady_clock::now();
}

Source::~Source()
{
  Stop();
}

std::vector<Client *> & Source::GetClients()
{
	return clients;
}

void Source::Release() 
{
  _mutex.lock();

	for (int i=0; i<(int)GetNumberOfClients(); i++) {
		Client *c = clients[i];

		c->Stop();

		delete c;
	}

	clients.clear();

	if ((void *)_source != NULL) {
		try {
			_source->Close();
			delete _source;
			_source = NULL;
		} catch (...) {
		}
	}

	if ((void *)_buffer != NULL) {
		delete _buffer;
		_buffer = NULL;
	}
  
  _mutex.unlock();
}

int Source::IsClosed()
{
	return _is_running == false;
}

jshared::IndexedBuffer * Source::GetBuffer() 
{
	return _buffer;
}

void Source::Stop()
{
  if (_is_running == false) {
    return;
  }

	_is_running = false;

  _thread.join();

	Release();
}

void Source::Start()
{
  if (_is_running == true) {
    return;
  }

  _thread = std::thread(&Source::Run, this);
}

int Source::GetIncommingRate()
{
	uint64_t
    elapsed = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::steady_clock::now() - _start_time)).count();

	return (int)((_sent_bytes*8LL)/(elapsed));
}

int Source::GetOutputRate()
{
	int rate = 0;

	for (std::vector<Client *>::iterator i=clients.begin(); i!=clients.end(); i++) {
		rate += (*i)->GetOutputRate();
	}

	return rate;
}

int Source::GetNumberOfClients()
{
	int n = 0;

  _mutex.lock();

	for (std::vector<Client *>::iterator i=clients.begin(); i!=clients.end(); i++) {
		if ((*i)->IsClosed() == false) {
			n = n + 1;
		}
	}

  _mutex.unlock();

	return n; // (int)clients.size();
}

std::string Source::GetSourceName()
{
	return _event;
}

bool Source::AddClient(jnetwork::Socket *socket, RequestParser &parser)
{
	if (IsClosed() == true) {
		return false;
	}

	Client *client = NULL;

	{
		if (GetNumberOfClients() > atoi(Configuration::GetInstance()->GetProperty("max-source-clients").c_str())) {
			return false;
		}

		if (parser.GetDestinationProtocol() == "http") {
			client = new Client(socket, this);

			std::cout << "Add HTTP client::[source=" << GetSourceName() << "] [client=0x" << std::hex << (unsigned long)(client) << "]" << std::dec << std::endl;
		} else if (parser.GetDestinationProtocol() == "udp") {
			if (parser.GetDestinationHost() != "" && parser.GetDestinationPort() > 1024) {
				client = new Client(socket, parser.GetDestinationHost(), parser.GetDestinationPort(), this);

				std::cout << "Add UDP client::[source=" << GetSourceName() << "] [client=0x" << std::hex << (unsigned long)(client) << "]" << std::dec << std::endl;
			}
		}

		if ((void *)client != NULL) {
      _mutex.lock();

			clients.push_back(client);

			client->Start();

      _mutex.unlock();

			return true;
		}
	}

	return false;
}

void Source::ReadStream() 
{
	jnetwork::SocketOptions *opt = NULL;
	std::string host = "localhost";
	int r,
		port;

	if (_source->GetType() == jnetwork::JCT_TCP) {
		jnetwork::Socket *s = dynamic_cast<jnetwork::Socket *>(_source);

		opt = s->GetSocketOptions();
		
		host = s->GetInetAddress()->GetHostAddress();
		port = s->GetPort();
	} else if (_source->GetType() == jnetwork::JCT_UDP) {
		jnetwork::DatagramSocket *s = dynamic_cast<jnetwork::DatagramSocket *>(_source);

		opt = s->GetSocketOptions();

		host = s->GetInetAddress()->GetHostAddress();
		port = s->GetPort();
	} else {
		return;
	}

	opt->SetSendTimeout(6000);
	opt->SetReceiveTimeout(6000);
	opt->SetReceiveMaximumBuffer(0x0200000);

	delete opt;

	if (_source->GetType() == jnetwork::JCT_TCP) {
		{
			std::ostringstream o;

			o << "GET " << _resource << " HTTP/1.0" << "\r\n";
			o << "Accept: */*\r\n";
			o << "User-Agent: NSPlayer/4.1.0.3928" << "\r\n";
			o << "Host: " << host << ":" << port << "\r\n";
			o << "Pragma: no-cache, rate=1.000000, stream-time=0, stream-offset=4294967295, request-context=2, max-duration=0" << "\r\n";
			o << "Pragma: xPlayStrm=" << "1" << "\r\n";
			o << "Pragma: xClientGUID={1FA17E66-096A-4192-B68A-025357348EDA}" << "\r\n";
			o << "Pragma: stream-switch-count" << "6" << "\r\n";
			o << "Pragma: stream-switch-entry=ffff:0:0 ffff:1:0 ffff:2:0 ffff:3:0 ffff:4:0 ffff:5:0";
			o << "\r\n";
			o << "\r\n";

			if (_source->GetOutputStream()->Write((const char *)o.str().c_str(), o.str().size()) < 0) {
				return;
			}

			if (_source->GetOutputStream()->Flush() < 0) {
				return;
			}
		}
	}

	jio::InputStream *i = _source->GetInputStream();
	char receive[4096];

	do {
		r = i->Read(receive, 4096);

		if (r < 0) {
			break;
		}

		_buffer->Write((uint8_t *)receive, r);

		_sent_bytes += r;
	} while (_is_running == true);

	std::cout << "[source=" << GetSourceName() << "] stream was closed" << std::endl;
}

void Source::RemoveClients()
{
	do {
    _mutex.lock();

		for (std::vector<Client *>::iterator i=clients.begin(); i!=clients.end(); i++) {
      Client *c = (*i);

      if (c->IsClosed() == true) {
        std::cout << "Remove client::[source=" << GetSourceName() << "] [client=0x" << std::hex << (unsigned long)(c) << "]" << std::dec << std::endl;

        clients.erase(i);

        c->Stop();
        delete c;

        _mutex.unlock();

        break;
      }
    }

    if (clients.empty() == true) {
      _mutex.unlock();

      break;
    }

    _mutex.unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(atoi(Configuration::GetInstance()->GetProperty("update-time").c_str())));
	} while (_is_running == true);
}

void Source::Run()
{
	_is_running = true;

	ReadStream();

	_is_running = false;
}

}
