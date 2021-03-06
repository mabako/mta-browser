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

#include "config.hpp"
#include "serverlist.hpp"

namespace browse
{
	class Window
	{
	public:
		 Window( );
		~Window( );
		
		static gint Pulse( gpointer data );
#ifdef WIN32
		static void Connect( GtkWidget* pWidget, gpointer data );
#endif
		static void Refresh( GtkWidget* pWidget, gpointer data );
		static void Destroy( GtkWidget* pWidget, gpointer data );
		static void SelectServer( GtkWidget* pWdidget, gint row, gint column, GdkEventButton* event, gpointer data );
		static void SelectNoServer( GtkWidget* pWidget, gint row, gint column, GdkEventButton* event, gpointer data );
		static void ChangeFilter( GtkEntry* pEntry, gpointer data );
		static void SetSortColumn( GtkCList* list, gint column, gpointer data );
		static gint ComparePlayers( GtkCList* list, gconstpointer ptr1, gconstpointer ptr2 );
		
		ServerList* GetServerList( );
		Server*     GetSelectedServer( );
		void        SetSelectedServer( Server* server );
		GtkWidget*  GetServerListWidget( );
		GtkWidget*  GetPlayerListWidget( );
		void		UpdateStatusLabel( string text );
	private:
		void        AddToServerList( Server* server );
		const bool	GetRefreshOnNextPulse( );
		void		SetRefreshOnNextPulse( bool b );
		
		GtkWidget*  pgWindow;
		GtkWidget*  pgServerList;
		GtkWidget*  pgPlayerList;
		GtkWidget*	pgStatus;
#ifdef WIN32
		GtkWidget*  pgButtonConnect;
#endif
		
		ServerList* pServerList;
		Server*     pSelectedServer;
		
		bool		refreshOnNextPulse;
	};
}
