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
#include "configuration.h"
#include "requestparser.h"

#include <iostream>
#include <sstream>

#include <string.h>
#include <errno.h>

namespace mlive {

Server::Server(int port)
{
	this->port = port;
}

Server::~Server()
{
}

void Server::Run() 
{
	try {
		HandleRequests();
	} catch (...) {
		std::cout << "Mlive Server:: " << strerror(errno) << std::endl;
	}
}

void Server::HandleRequests() 
{
	jnetwork::Socket *socket = NULL;
	jnetwork::ServerSocket server(port, 10);

	std::cout << "Mlive Streamer Ready [" << server.GetInetAddress()->GetHostName() << ":" << port << "]\r\n" << std::endl;

	do {
    for (std::vector<Source *>::iterator i=sources.begin(); i!=sources.end(); i++) {
      Source *source = (*i);

      source->RemoveClients();
    }

		RemoveSources();

		try {
			std::cout << "Waiting for connection ..." << std::endl;

			socket = NULL;
			socket = server.Accept();

			if (socket != NULL) {
				std::cout << "Handle request::[" << socket->GetInetAddress()->GetHostAddress() << ":" << socket->GetPort() << "]" << std::endl;

				int ch, 
					k = 0,
					count = 0;
				char end[4],
					 receive[4098+1];

				// read client request
				do {
					ch = socket->GetInputStream()->Read();

					if (ch < 0) {
						break;
					}

					if (ch == '\r' && k != 2) {
						k = 0;
					}

					end[k] = ch;
					k = (k+1)%4;

					if (end[0] == '\r' &&
							end[1] == '\n' &&
							end[2] == '\r' &&
							end[3] == '\n') {
						break;
					}

					receive[count++] = (char)ch;
				} while (count < 4096);

				receive[count] = '\0';

				if (ch > 0) {
					if (ProcessClient(socket, receive) == false) {
						std::cout << "Cancel request::[" << socket->GetInetAddress()->GetHostAddress() << ":" << socket->GetPort() << "]" << std::endl;

						if ((void *)socket != NULL) {
							socket->Close();
							delete socket;
							socket = NULL;
						}
					}
				} else {
					if ((void *)socket != NULL) {
						socket->Close();
						delete socket;
						socket = NULL;
					}
				}
			}
		} catch (...) {
			if ((void *)socket != NULL) {
				socket->Close();
				delete socket;
				socket = NULL;
			}
		}
	} while (true);
}

int Server::GetNumberOfSources()
{
	int n = 0;

  _mutex.lock();

	for (std::vector<Source *>::iterator i=sources.begin(); i!=sources.end(); i++) {
		if ((*i)->IsClosed() == false) {
			n = n + 1;
		}
	}

  _mutex.unlock();

	return n; // (int)sources.size();
}

bool Server::ProcessClient(jnetwork::Socket *socket, std::string receive)
{
	RequestParser parser(receive);

	RequestParser::requestparser_method_t method = parser.GetMethod();
	
	try {
		if (method == RequestParser::GETINFO_METHOD) {
			std::cout << "Response INFO to [" << socket->GetInetAddress()->GetHostAddress() << ":" << socket->GetPort() << "]" << std::endl;

			std::ostringstream o;

			o << "HTTP/1.0 200 OK" << "\r\n";
			o << "Cache-Control: no-cache" << "\r\n";
			o << "Pragma: no-cache" << "\r\n";
			o << "Content-Type: text/plain" << "\r\n";
			o << "Connection: close" << "\r\n";
			o << "\r\n";
			o << "<mlive type=\"info\">" << "\r\n";
			
			// o << "\t<server host=\"" << "127.0.0.1" << "\" port=\"" << "80" << "\" />" << "\r\n";

			if (sources.size() > 0) {
				o << "\t<events>" << "\r\n";

				for (std::vector<Source *>::iterator i=sources.begin(); i!=sources.end(); i++) {
					Source *source = (*i);

					o << "\t\t<event " << 
						"name=\"" << source->GetSourceName() << "\" " << 
						"input-rate=\"" << source->GetIncommingRate() << "\" " << 
						"output-rate=\"" << source->GetOutputRate() << "\" " << 
						"clients=\"" << source->GetNumberOfClients() << "\" ";

					if (source->IsClosed() == false) {
						o << "status=\"" << "started" << "\" ";
					} else {
						o << "status=\"" << "closed" << "\" ";
					}

					o << ">" << "\r\n";

					std::vector<Client *> &clients = (*i)->GetClients();

					for (std::vector<Client *>::iterator j=clients.begin(); j!=clients.end(); j++) {
						Client *client = (*j);

						o << "\t\t\t<client " << 
							"id=\"" << client << "\" " << 
							"output-rate=\"" << client->GetOutputRate() << "\" " << 
							"start-time=\"" << client->GetStartTime() << "\" " << 
							"sent-bytes=\"" << client->GetSentBytes() << "\" ";

						if (client->GetType() == Client::HTTP_CLIENT_TYPE) {
							o << "type=\"" << "http" << "\" ";
						} else if (client->GetType() == Client::UDP_CLIENT_TYPE) {
							o << "type=\"" << "udp" << "\" ";
						}

						if (client->IsClosed() == false) {
							o << "status=\"" << "started" << "\" ";
						} else {
							o << "status=\"" << "closed" << "\" ";
						}

						o << "/>" << "\r\n";
					}

					o << "\t\t</event>" << "\r\n";
				}

				o << "\t</events>" << "\r\n";
			}

			o << "</mlive>" << "\r\n";
			o << "\r\n" << std::flush;

			socket->Send((const char *)o.str().c_str(), o.str().size());

			return false;
		} else if (method == RequestParser::GETCONFIG_METHOD) {
			std::cout << "Response GETCONFIG to [" << socket->GetInetAddress()->GetHostAddress() << ":" << socket->GetPort() << "]" << std::endl;

			std::ostringstream o;

			o << "HTTP/1.0 200 OK" << "\r\n";
			o << "Cache-Control: no-cache" << "\r\n";
			o << "Pragma: no-cache" << "\r\n";
			o << "Content-Type: text/plain" << "\r\n";
			o << "Connection: close" << "\r\n";
			o << "\r\n";
			o << "<mlive type=\"config\">" << "\r\n";

			for (std::map<std::string, std::string>::iterator i=
				Configuration::GetInstance()->GetProperties().begin(); i!=Configuration::GetInstance()->GetProperties().end(); i++) {
				o << "\t<param name=\"" << i->first << "\" value=\"" << Configuration::GetInstance()->GetProperty(i->first) << "\" />" << "\r\n";
			}

			o << "</mlive>" << "\r\n";
			o << "\r\n" << std::flush;

			socket->Send((const char *)o.str().c_str(), o.str().size());

			return false;
		} else if (method == RequestParser::SETCONFIG_METHOD) {
			if (Configuration::GetInstance()->GetProperty("update-config") == "false") {
				std::cout << "SETCONFIG DISABLED" << std::endl;
			} else {
				std::cout << "Response SETCONFIG to [" << socket->GetInetAddress()->GetHostAddress() << ":" << socket->GetPort() << "]" << std::endl;

				for (std::map<std::string, std::string>::iterator i=
					Configuration::GetInstance()->GetProperties().begin(); i!=Configuration::GetInstance()->GetProperties().end(); i++) {
					if (parser.GetParameter(i->first)  != "") {
						Configuration::GetInstance()->SetProperty(i->first, parser.GetParameter(i->first).c_str());
					}
				}
			}
		} else if (method == RequestParser::STREAM_METHOD) {
			std::cout << "Response stream to [" << socket->GetInetAddress()->GetHostAddress() << ":" << socket->GetPort() << "]" << std::endl;

			{
				Source *source = NULL;

        std::unique_lock<std::mutex> lock(_mutex);

				for (std::vector<Source *>::iterator i=sources.begin(); i!=sources.end(); i++) {
					if ((*i)->GetSourceName() == parser.GetEventName()) {
						if ((*i)->IsClosed() == true) {
							return false;
						}

						source = (*i);

						break;
					}
				}

				int in_rate = 0, 
					out_rate = 0;

				for (std::vector<Source *>::iterator i=sources.begin(); i!=sources.end(); i++) {
					if ((*i)->IsClosed() == false) {
						in_rate += (*i)->GetIncommingRate();
						out_rate += (*i)->GetOutputRate();
					}
				}

				if ((void *)source == NULL) {
					if (GetNumberOfSources() >= atoi(Configuration::GetInstance()->GetProperty("max-sources").c_str())) {
						return false;
					}

					if (in_rate >= atoi(Configuration::GetInstance()->GetProperty("max-input-rate").c_str())) {
						return false;
					}

					if (parser.GetSourceHost() != "" && parser.GetSourcePort() > 1024) {
						if (parser.GetSourceProtocol() == "http") {
							source = new Source(
									parser.GetSourceHost(), 
									parser.GetSourcePort(), 
									parser.GetEventName(),
									Source::HTTP_SOURCE_TYPE,
									this,
									parser.GetDestinationResource());

							std::cout << "Create HTTP source::[" << parser.GetEventName() << " at " << parser.GetSourceHost() << ":" << parser.GetSourcePort() << "]" << std::endl;
						} else if (parser.GetSourceProtocol() == "udp") {
							source = new Source(
									parser.GetSourceHost(), 
									parser.GetSourcePort(), 
									parser.GetEventName(),
									Source::UDP_SOURCE_TYPE,
									this,
									parser.GetDestinationResource());
							std::cout << "Create UDP source::[" << parser.GetEventName() << " at " << parser.GetSourceHost() << ":" << parser.GetSourcePort() << "]" << std::endl;
						}

						if (source == NULL) {
							return false;
						}

						sources.push_back(source);

						source->Start();
					} else {
						// error message
						std::ostringstream o;

						o << "HTTP/1.0 403 OK" << "\r\n";
						o << "Cache-Control: no-cache" << "\r\n";
						o << "Pragma: no-cache" << "\r\n";
						o << "Content-Type: text/html" << "\r\n";
						o << "Connection: close" << "\r\n";
						o << "\r\n";
						o << "<html><body>" << "\r\n";
						o << "Mlive<br><hr><br>" << "\r\n";
						o << "Invalid source / Request error" << "\r\n";
						o << "</body><html>" << "\r\n";
						o << "\r\n" << std::flush;

						socket->Send((const char *)o.str().c_str(), o.str().size());

						return false;
					}
				}	

				if (out_rate >= atoi(Configuration::GetInstance()->GetProperty("max-output-rate").c_str())) {
					return false;
				}

				return source->AddClient(socket, parser);
			}
		} else {
			// main page message
			std::ostringstream o;

			o << "HTTP/1.0 200 OK" << "\r\n";
			o << "Cache-Control: no-cache" << "\r\n";
			o << "Pragma: no-cache" << "\r\n";
			o << "Content-Type: text/html" << "\r\n";
			o << "Connection: close" << "\r\n";
			o << "\r\n";
			o << "<html><body>" << "\r\n";
			o << "Mlive<br><hr><br>" << "\r\n";
			o << "version 0.01 alpha" << "\r\n";
			o << "</body><html>" << "\r\n";
			o << "\r\n" << std::flush;

			socket->Send((const char *)o.str().c_str(), o.str().size());

			return false;
		}
	} catch (...) {
	}

	return false;
}

void Server::RemoveSources()
{
  _mutex.lock();

  for (std::vector<Source *>::iterator i=sources.begin(); i!=sources.end(); i++) {
    Source *c = (*i);

    if (c->IsClosed() == true) {
      std::cout << "Remove source::[source=" << c->GetSourceName() << "] [address=0x" << std::hex << (unsigned long)(c) << "]" << std::dec << std::endl;

      sources.erase(i);

      c->Stop();
      delete c;

      break;
    }

  }

  _mutex.unlock();
}

}
