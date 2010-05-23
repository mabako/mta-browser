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

#include <list>
#include <string>

namespace browse
{
	using namespace std;
	
	class Player
	{
	friend class Server;
	private:
		Player( const string _name, const unsigned short _ping )
		{
			name = _name;
			ping = _ping;
		}
		
		string			name;
		unsigned short	ping;
	public:
		~Player( ) { };
		
		const string			GetName( )	{ return name; }
		const unsigned short	GetPing( )	{ return ping; }
	};
	
	class Server
	{
	friend class ServerListItem;
	private:
		Server( const string queryResponse, const string ip );
		
		const bool		ReadString( string &read );
		Player*			ReadPlayer( );
		
		string				ip;
		unsigned short		port;
		string				name;
		string				gametype;
		string				mapname;
		
		bool				passworded;
		unsigned char		numplayers;
		unsigned char		maxplayers;
		
		list < Player* >	players;
		
		unsigned int		pos;
		string				buffer;
		bool				valid;
	public:
		~Server( );
		
		const bool				IsValid( )					{ return valid; }
		const string			GetIP( )					{ return ip; }
		const unsigned short	GetPort( )					{ return port; }
		const string			GetName( )					{ return name; }
		const string			GetGameType( )				{ return gametype; }
		const string			GetMapName( )				{ return mapname; }
		const bool				HasPassword( )				{ return passworded; }
		const unsigned char		GetNumPlayers( )			{ return numplayers; }
		const unsigned char		GetMaxPlayers( )			{ return maxplayers; }
		const list< Player* >	GetPlayers( )				{ return players; }
		
		const bool				Matches( string filter );
	};
}
