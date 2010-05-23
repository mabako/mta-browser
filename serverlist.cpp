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
#include <sstream>
#include <curl/curl.h>

#include "serverlist.h"

namespace browse
{
	using namespace std;
	
	/* Small Helper for HTTP Requests */
	class HTTP
	{
	public:
		/* Does the request */
		HTTP( const string url, unsigned int timeout = 0 )
		{
			/* Initalize CURL */
			CURL* curl = curl_easy_init( );
			if( curl )
			{
				curl_easy_setopt( curl, CURLOPT_URL, SERVER_LIST_URL );
				curl_easy_setopt( curl, CURLOPT_HEADER, 0 );
				if( timeout > 0 )
				{
					curl_easy_setopt( curl, CURLOPT_TIMEOUT_MS, timeout );
				}
				
				/* Fetch our data */
				string buffer;
				curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteResult );
				curl_easy_setopt( curl, CURLOPT_WRITEDATA, &buffer );
				CURLcode curlresult = curl_easy_perform( curl );
				
				/* Cleanup */
				curl_easy_cleanup( curl );
				
				if( curlresult == CURLE_OK )
				{
					result = buffer;
					return;
				}
				
				error = result + " " + curl_easy_strerror( curlresult );
				return;
			}
			error = "Could not initalize CURL";
		}
		 
		/* Not much left to destroy */
		~HTTP( )
		{
		
		}
		
		/* Returns true if the request was successful */
		bool WasSuccessful( )
		{
			return error.length( ) == 0;
		}
		
		/* Returns the response to the HTTP request if it was successful. */
		const string GetResult( )
		{
			return result;
		}
		
		/* Returns the CURL error message if any */
		const string GetError( )
		{
			return error;
		}
		
	private:
		string error;
		string result;
		
		/* Writer for whatever is returned by CURL */
		static int WriteResult( char* data, size_t size, size_t something, string* buffer )
		{
			int result = 0;
			if( buffer )
			{
				buffer->append( data, size * something );
				result = size * something;
			}
			return result;
		}
	};
	
	/* Server List Class */
	
	ServerList::ServerList( )
	{
#ifdef WIN32
	   /* Shoot me against a wall, I'm a WinSock */
	   WSADATA wsa;
	   WSAStartup(MAKEWORD(2,0),&wsa);
#endif
	}
	
	ServerList::~ServerList( )
	{
		Clear( );
#ifdef WIN32
	   	WSACleanup( );
#endif
	}
	
	/* Clears the old server list and instead fetches it again */
	void ServerList::Refresh( )
	{
		Clear( );
		
		/* New HTTP request */
		HTTP* pHTTP = new HTTP( SERVER_LIST_URL, SERVER_LIST_MASTER_TIMEOUT );
		if( pHTTP )
		{
			if( pHTTP->WasSuccessful( ) )
			{
				ParseList( pHTTP->GetResult( ) );
			}
			/*
			else
			{
				cout << "Error fetching Server List: " << pHTTP->GetError( ) << endl;
			}
			*/
			delete pHTTP;
		}
	}
	
	/* Performs a single tick */
	list < Server* > ServerList::Pulse( )
	{
		list < Server* > newServersThisPulse;
		
		for( list < ServerListItem* >::iterator iter = newServers.begin( ); iter != newServers.end( ); )
		{
			Server* server = (*iter)->Pulse( );
			if( server )
			{
				/* destroy the list item */
				newServers.erase( iter++ );
				
				if( server->IsValid( ) )
				{
					/* add it to the list of actually existing servers */
					servers.push_back( server );
					
					/* We only want it to appear on the GUI if it matches our filter */
					if( server->Matches( filter ) )
					{
						newServersThisPulse.push_back( server );
					}
				}
			}
			else
			{
				++ iter;
			}
		}
		return newServersThisPulse;
	}
	
	/* Returns all servers matching the filter */
	list < Server* > ServerList::Filter( string _filter )
	{
		filter = _filter;
		list < Server* > matchingServers;
		
		for( list < Server* >::iterator iter = servers.begin( ); iter != servers.end( ); ++ iter )
			if( (*iter)->Matches( filter ) )
				matchingServers.push_back( *iter );
		
		return matchingServers;
	}
	
	/* Clears the whole server list */
	void ServerList::Clear( )
	{
		/* Clear the old server lists */
		for( list < ServerListItem* >::iterator iter = newServers.begin( ); iter != newServers.end( ); ++ iter )
		{
			delete *iter;
		}
		newServers.clear( );
		
		for( list < Server* >::iterator iter = servers.begin( ); iter != servers.end( ); ++ iter )
		{
			delete *iter;
		}
		servers.clear( );
	}
	
	/* Parses the Server List and creates a new class for each server */
	void ServerList::ParseList( string list )
	{
		if( list.length( ) < 2 )
			return;
		
		unsigned short count = ( list[0] << 8 ) + list[1];
		/*
		cout << "Server List: " << count << " Servers" << endl;
		*/
		
		unsigned int length = list.length( );
		unsigned int pos = 2;
		
		while( pos < length - 6 && count -- )
		{
			/* Fetch IP/Host parts */
			unsigned char
				a = list[pos],
				b = list[pos+1],
				c = list[pos+2],
				d = list[pos+3];
			stringstream ip;
			ip << (unsigned int) a << "." << (unsigned int) b << "." << (unsigned int) c << "." << (unsigned int) d;
			
			/* add to our server list */
			newServers.push_back( new ServerListItem( ip.str( ), ( unsigned short )( ( list[pos+4] << 8 ) + list[pos+5] ) ) );
			
			pos += 6;
		}
	}
}
