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

/* Networking stuff */
#ifndef WIN32
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#include "config.h"
#include "serverlistitem.h"

namespace browse
{
	using namespace std;
	
	ServerListItem::ServerListItem( const string _ip, const unsigned short port )
	{
		ip = _ip;
		
		sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		/* We need non-blocking sockets */
#ifdef _WIN32
		unsigned long flag = 1;
		ioctlsocket( sock, FIONBIO, &flag );
#else
		int flags = fcntl(sock, F_GETFL);
		flags |= O_NONBLOCK;
		fcntl(sock, F_SETFL, flags);
#endif
		
		addressSize = sizeof( address );
		memset( &address, 0, sizeof( address ) );
		address.sin_family = AF_INET;
		address.sin_port = htons(port);
		address.sin_addr.s_addr = inet_addr( ip.c_str( ) );
		if( sendto( sock, "s", 1, 0, ( sockaddr* ) &address, addressSize ) < 0 )
		{
		/*
			cout << "Error" << endl;
		*/
		}
	}
	
	ServerListItem::~ServerListItem( )
	{
		if( sock > 0 )
#ifdef WIN32
			closesocket( sock );
#else
			close( sock );
#endif
	}
	
	Server* ServerListItem::Pulse( )
	{
		char buffer[ SERVER_LIST_QUERY_BUFFER ] = { 0 };
		if( recvfrom( sock, buffer, sizeof( buffer ), 0, ( sockaddr* ) &address, &addressSize ) > 0 )
		{
			/* Since we have some data, close our socket. */
#ifdef WIN32
			closesocket( sock );
#else
			close( sock );
#endif
			sock = 0;
			
			/* Return the details */
			return new Server( buffer, ip );
		}
		return NULL;
	}
}
