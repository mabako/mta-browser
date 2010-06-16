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

/* Version of this app */
#define VERSION                     "1.5"

/* Name of the Window */
#define APPLICATION_TITLE			"MTA Server Browser " VERSION

/* Timeout for the complete server list to be parsed. */
#define SERVER_LIST_MASTER_TIMEOUT	3000

/* Buffer size for a query that was sent */
#define SERVER_LIST_QUERY_BUFFER	4096

/* ATTENTION: The following should be equal to the game configuration. */
/* Address to query for the Server List. */
#define SERVER_LIST_URL				"http://1mgg.com/affil/mta-1.0"

/* ASE Game - This specifies which game we actually want to see. Unset this to show any. */
#define ASE_GAME					"mta"

/* ASE Version - The Game's version. Unset this to show any. */
#define ASE_VERSION                 "1.0"

/* Max. Queries to send per pulse */
#define MAX_QUERIES_PER_PULSE		5

/* Check if _WIN32 is defined (Visual Studio) */
#if defined(_WIN32) && !defined(WIN32)
	#define WIN32
#endif

#if defined(_DEBUG) && !defined(DEBUG)
	#define DEBUG
#endif
