/*
GTK+ based MTA:SA Server Browser
Copyright (c) 2010 mabako

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <string.h>

#include "config.hpp"

/* Networking stuff */
#ifndef WIN32
	#include <arpa/inet.h>
#endif

#ifdef DEBUG
	#include <iostream>
#endif

#include "serverlistitem.hpp"

namespace browse
{
	using namespace std;
	
	ServerListItem::ServerListItem( const string _ip, const unsigned short port, const int socket )
	{
#ifdef DEBUG
		cout << "Server to query: " << _ip << ":" << ( unsigned int ) port << endl;
#endif
		
		sock = socket;
		ip = _ip;
		
		sent = false;
		
		/* Since we do set an address, we'll get messages only from this ip/port (below) */
		addressSize = sizeof( address );
		memset( &address, 0, sizeof( address ) );
		address.sin_family = AF_INET;
		address.sin_port = htons(port);
		address.sin_addr.s_addr = inet_addr( ip.c_str( ) );
	}
	
	ServerListItem::~ServerListItem( )
	{
	}
	
	const bool ServerListItem::WasQuerySent( )
	{
		return sent;
	}
	
	const bool ServerListItem::SendQuery( )
	{
		if( !WasQuerySent( ) )
		{
			if( sock == 0 )
			{
				return false;
			}
			else if( sendto( sock, "s", 1, 0, ( sockaddr* ) &address, addressSize ) < 0 )
			{
#ifdef DEBUG
				cout << "Error for " << ip << endl;
#endif
				return false;
			}
			else
			{
				sent = true;
			}
			return true;
		}
		return false;
	}
}
