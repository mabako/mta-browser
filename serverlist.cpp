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

/* Networking stuff */
#ifndef WIN32
	#include <sys/socket.h>
	#include <fcntl.h>
	#include <arpa/inet.h>
#endif

#ifdef DEBUG
	#include <iostream>
#endif

#include "serverlist.hpp"

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
					/* Check for HTTP Status code */
					long httpcode = 0;
					curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &httpcode );
					switch( httpcode )
					{
						case 0: /* Will be returned if no response code is recieved */
						case 200:
							result = buffer;
							break;
						case 401:
							error = "GM Error 401: Unauthorized";
							break;
						case 403:
							error = "GM Error 403: Forbidden";
							break;
						case 404:
							error = "GM Error 404: Not Found";
							break;
						case 500:
							error = "GM Error 500: Internal Server Error";
							break;
						default:
							stringstream ss;
							ss << "GM Error " << httpcode;
							error = ss.str( );
					}
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
		
		/* Create a Socket - one for all our needs */
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
		
		/* Initalize our status */
		totalServers = 0;
		filteredServers = 0;
	}
	
	ServerList::~ServerList( )
	{
		if( sock > 0 )
#ifdef WIN32
			closesocket( sock );
#else
			close( sock );
#endif
		
		Clear( );
#ifdef WIN32
		WSACleanup( );
#endif
	}
	
	/* Clears the old server list and instead fetches it again */
	const string ServerList::Refresh( )
	{
		Clear( );
		
		/* Delete the old socket since we do not want its messages anymore */
		if( sock > 0 )
#ifdef WIN32
			closesocket( sock );
#else
			close( sock );
#endif
		
		/* Create another socket */
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
		
		/* New HTTP request */
		HTTP* pHTTP = new HTTP( SERVER_LIST_URL, SERVER_LIST_MASTER_TIMEOUT );
		string returnValue = "";
		if( pHTTP )
		{
			if( pHTTP->WasSuccessful( ) )
			{
				returnValue = ParseList( pHTTP->GetResult( ) );
			}
			else
			{
				returnValue = pHTTP->GetError( );
			}
			delete pHTTP;
		}
		return returnValue;
	}
	
	/* Performs a single tick */
	list < Server* > ServerList::Pulse( )
	{
		list < Server* > newServersThisPulse;
		unsigned int sentQueriesThisPulse = 0;
		
		sockaddr_in address;
		socklen_t addressSize = sizeof( address );
		char buffer[ SERVER_LIST_QUERY_BUFFER ] = { 0 };
		while( recvfrom( sock, buffer, sizeof( buffer ), 0, ( sockaddr* ) &address, &addressSize ) > 0 )
		{
			Server* server = new Server( buffer, string( inet_ntoa( address.sin_addr ) ) );
			if( server->IsValid( ) )
			{
				/* add it to the list of existing servers */
				servers.push_back( server );
#ifdef DEBUG
				cout << "Server " << servers.size( ) << ": " << server->GetIP( ) << ":" << server->GetPort( ) << " - " << server->GetName( ) << endl;
#endif
				/* We only want it to appear on the GUI if it matches our filter */
				if( server->Matches( filter ) )
				{
					++ filteredServers;
					newServersThisPulse.push_back( server );
				}
			}
			else
			{
#ifdef DEBUG
				cout << "Dropping Server " << inet_ntoa( address.sin_addr ) << " (Invalid Server)" << endl;
#endif
				delete server;
			}
		}

		for( list < ServerListItem* >::iterator iter = newServers.begin( ); iter != newServers.end( ); )
		{
			if( !(*iter)->WasQuerySent( ) )
			{
				if( sentQueriesThisPulse < MAX_QUERIES_PER_PULSE )
				{
					if( (*iter)->SendQuery( ) )
					{
						++ sentQueriesThisPulse;
						newServers.erase( iter++ );
					}
					else
					{
						++ iter;
					}
				}
				else
				{
					/* Let's fast-forward to the end of the list as we don't want to mess with more stuff */
					break;
				}
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
		
		filteredServers = matchingServers.size( );
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
		
		/* Update our status */
		totalServers = 0;
		filteredServers = 0;
	}
	
	/* Parses the Server List and creates a new class for each server */
	const string ServerList::ParseList( string list )
	{
		if( list.length( ) < 2 )
			return "Failed to parse List";
		
		unsigned short count = ( list[0] << 8 ) + list[1];
		totalServers = count;
		
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
			newServers.push_back( new ServerListItem( ip.str( ), ( unsigned short )( ( list[pos+4] << 8 ) + list[pos+5] ), sock ) );
			
			pos += 6;
		}
		return "";
	}

	const string ServerList::GetStatus( )
	{
		stringstream ss;
		if( filter.length( ) == 0 )
			ss << "Found " << servers.size( ) << "/" << totalServers << " Servers";
		else if( filteredServers == 0 )
			return "No matching Servers found";
		else
			ss << "Found " << filteredServers << "/" << totalServers << " matching Servers";
		return ss.str( );
	}
}
