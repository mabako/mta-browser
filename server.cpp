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

#include <sstream>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "server.h"
#include <iostream>

namespace browse
{
	using namespace std;
	
	/* Case Insensitive string.find */
	bool contains( const string string1, const string string2 )
	{
		string str1( string1 );
		string str2( string2 );
		transform( str1.begin(), str1.end(), str1.begin(), ::tolower );	
		transform( str2.begin(), str2.end(), str2.begin(), ::tolower );
		return str1.find( str2 ) != string::npos;
	}
	
	Server::Server( const string queryResponse, const string _ip )
	{
		buffer = queryResponse;
		valid = false;
		
		if( queryResponse.length( ) < 15 )
			return;
		
		/* MTA's custom ASE protocol header */
		if( strncmp( queryResponse.c_str( ), "EYE1", 4 ) != 0 )
			return;
		pos = 4; /* We read the EYE1 part */
		
		/* Save the IP */
		ip = _ip;
		
		/* Read the actual Query information */
		string temp;
		
		/* Game: this should be our game version string. */
		if( !ReadString( temp ) )
			return;
#ifdef ASE_GAME
		if( temp != ASE_GAME )
			return;
#endif
		
		/* Port */
		if( !ReadString( temp ) )
			return;
		port = atoi( temp.c_str( ) );
		
		/* Server Name */
		if( !ReadString( name ) )
			return;
		
		/* Game Type */
		if( !ReadString( gametype ) )
			return;
		if( gametype == "MTA:SA" )
			gametype = "";
		
		/* Map Name */
		if( !ReadString( mapname ) )
			return;
		if( mapname == "None" )
			mapname = "";
		
		/* Version */
		if( !ReadString( temp ) )
			return;
#ifdef ASE_VERSION
		if( temp != ASE_VERSION )
			return;
#endif
		
		/* Is this server passworded? */
		if( !ReadString( temp ) )
			return;
		passworded = atoi( temp.c_str( ) ) == 1;
		
		/* Number of players */
		if( !ReadString( temp ) )
			return;
		numplayers = atoi( temp.c_str( ) );
		
		/* Max. number of players */
		if( !ReadString( temp ) )
			return;
		maxplayers = atoi( temp.c_str( ) );
		
		/* Rules - I mean, who cares? */
		do
		{
			/* Key */
			if( !ReadString( temp ) )
				return;
			
			/* If the Key is empty, we're at the end of rules */
			if( temp.length( ) == 0 )
				break;
			
			/* Value */
			if( !ReadString( temp ) )
				return;
		}
		while( true ); /* Break me, c'mon */
		
		/* Read the player list */
		for( unsigned char ppos; ppos < numplayers; ppos ++ )
		{
			Player* player = ReadPlayer( );
			if( player )
				players.push_back( player );
			else
				return;
		}
		
		/* And finally it is valid */
		valid = true;
	}
	
	/* Reads an ASE string */
	const bool Server::ReadString( string &read )
	{
		if ( pos <= buffer.length( ) )
		{
			unsigned char len = buffer[pos];
			if ( pos + len <= buffer.length( ) )
			{
				stringstream temp;
				for( unsigned int i = pos + 1; i < ( pos + len ); i ++ )
				{
					char c = buffer[i];
					if( c >= 32 && c <= 127 )
						temp << c;
				}
				read = temp.str( );
				pos += len;
				return true;
			}
			pos++;
		}
		return false;
	}
	
	Player* Server::ReadPlayer( )
	{
		if( pos >= buffer.length( ) )
			return NULL;
		
		string name;
		unsigned short ping = 0;
		
		string temp;
		unsigned char flags = buffer[ pos ++ ];
		
		/* Name */
		if( flags & 0x01 )
			if( !ReadString( name ) )
				return NULL; 
		
		/* Team Name */
		if( flags & 0x02 )
			if( !ReadString( temp ) )
				return NULL; 
		
		/* Skin */
		if( flags & 0x04 )
			if( !ReadString( temp ) )
				return NULL; 
		
		/* Score */
		if( flags & 0x08 )
			if( !ReadString( temp ) )
				return NULL; 
		
		/* Ping */
		if( flags & 0x10 )
		{
			if( !ReadString( temp ) )
				return NULL;
			ping = (unsigned short) atoi( temp.c_str( ) );
		}
		
		/* Time */
		if( flags & 0x20 )
			if( !ReadString( temp ) )
				return NULL;
		
		return new Player( name, ping );
	}
	
	Server::~Server( )
	{
		/* Clear the old server lists */
		for( list < Player* >::iterator iter = players.begin( ); iter != players.end( ); ++ iter )
		{
			delete *iter;
		}
		players.clear( );
	}
	
	/* Checks if the server matches the given filter */
	const bool Server::Matches( string filter )
	{
		/* We're not even valid/fully initalized, skip this */
		if( !IsValid( ) )
			return false;
		
		/* Always matches no filter */
		if( filter.length( ) == 0 )
			return true;
		
		/* It's part of the server name */
		if( contains( name, filter ) )
			return true;
		
		/* It's a part of the IP */
		if( contains( ip, filter ) )
			return true;
		
		/* It's a part of the game type */
		if( contains( gametype, filter ) )
			return true;
		
		/* It's a part of the map name */
		if( contains( mapname, filter ) )
			return true;
		
		/* Check if it's contained in any player's name */
		for( list < Player* >::iterator iter = players.begin( ); iter != players.end( ); ++ iter )
			if( contains( (*iter)->GetName( ), filter ) )
				return true;
		
		return false;
	}
}
