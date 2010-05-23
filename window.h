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
#include <assert.h>
#include <gtk/gtk.h>

#include "config.h"
#include "serverlist.h"

namespace browse
{
	class Window
	{
	public:
		 Window( );
		~Window( );
		
		static gint Pulse( gpointer data );
		static void Refresh( GtkWidget* pWidget, gpointer data );
		static void Destroy( GtkWidget* pWidget, gpointer data );
		static void SelectServer( GtkWidget* pWdidget, gint row, gint column, GdkEventButton* event, gpointer data );
		static gint ComparePlayers( GtkCList* list, gconstpointer ptr1, gconstpointer ptr2 );
		
		ServerList* GetServerList( );
		GtkWidget*  GetServerListWidget( );
		GtkWidget*  GetPlayerListWidget( );
	private:
		GtkWidget*  pgWindow;
		GtkWidget*  pgServerList;
		GtkWidget*  pgPlayerList;
		
		ServerList* pServerList;
	};
}
