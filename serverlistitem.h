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

#ifdef WIN32
#include <winsock.h>
typedef int socklen_t;
#else
#include <netinet/in.h> /* sockaddr_in, socklen_t */
#endif
#include <string>

#include "server.h"

namespace browse
{
	using namespace std;
	
	class ServerListItem
	{
	friend class ServerList;
	public:
		 ServerListItem( const string ip, const unsigned short port, const int socket );
		~ServerListItem( );
		
	private:
		const bool	WasQuerySent( );
		const bool	SendQuery( );
		Server*		Receive( );
		
		/* Socket stuff */
		int sock;
		sockaddr_in address;
		socklen_t addressSize;
		
		/* Server Information */
		string ip;
		bool sent;
	};
}
