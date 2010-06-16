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
#include <list>
#include "config.hpp"
#include "serverlistitem.hpp"

namespace browse
{
	using namespace std;
	
	class ServerList
	{
	public:
		 ServerList( );
		~ServerList( );
		
		void				Refresh( );
		list < Server* >	Pulse( );
		list < Server* >	Filter( string filter );
		const string		GetStatus( );
	private:
		void				Clear( );
		void				ParseList( string list );
		
		int							sock;
		list < Server* >			servers;
		list < ServerListItem* >	newServers;
		string						filter;
		
		unsigned short				totalServers;
		unsigned short				filteredServers;
	};
}
