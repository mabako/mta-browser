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

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "window.hpp"

namespace browse
{
	Window::Window( )
	{
		/* Create the window */
		pgWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		
		/* adjust some properties */
		gtk_window_set_title( GTK_WINDOW( pgWindow ), APPLICATION_TITLE );
#ifdef WIN32
		gtk_window_set_default_size( GTK_WINDOW( pgWindow ), 1000, 540 );
#else
	 	/* Good on Linux, ugly on Windows */
		gtk_window_set_default_size( GTK_WINDOW( pgWindow ), 1000, 480 );
#endif
		gtk_window_set_policy( GTK_WINDOW( pgWindow), true, true, false );
		gtk_container_set_border_width( GTK_CONTAINER( pgWindow ), 5 );
		
		/* Create a layout */
		GtkWidget* pLayout = gtk_table_new( 20, 5, TRUE );
		gtk_table_set_col_spacings( GTK_TABLE( pLayout ), 3 );
		gtk_table_set_row_spacings( GTK_TABLE( pLayout ), 3 );
		gtk_container_add( GTK_CONTAINER( pgWindow ), pLayout );
		
		/* Server List */
		gchar* serverListTitles[5] = { ( gchar* ) "Name", ( gchar* ) "Players", ( gchar* ) "Gametype", ( gchar* ) "Map", ( gchar* ) "IP/Port" };
		pgServerList = gtk_clist_new_with_titles( 5, serverListTitles );
		gtk_clist_set_sort_column( GTK_CLIST( pgServerList ), 1 );
		gtk_clist_set_compare_func( GTK_CLIST( pgServerList ), &ComparePlayers );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 0, 250 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 1, 60 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 2, 130 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 3, 130 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 4, 150 );
		gtk_signal_connect( GTK_OBJECT( pgServerList ), "select-row", GTK_SIGNAL_FUNC( &SelectServer ), this );
		gtk_signal_connect( GTK_OBJECT( pgServerList ), "unselect-row", GTK_SIGNAL_FUNC( &SelectNoServer ), this );
		gtk_signal_connect( GTK_OBJECT( pgServerList ), "click-column", GTK_SIGNAL_FUNC( &SetSortColumn ), this );
		GtkObject* hadj = gtk_adjustment_new( 0.0, 0.0, 1.0, 0.01, 0.1, 0.1 );
		GtkObject* vadj = gtk_adjustment_new( 0.0, 0.0, 1.0, 0.01, 0.1, 0.1 );
		GtkWidget* pgServerScroll = gtk_scrolled_window_new( GTK_ADJUSTMENT( hadj ), GTK_ADJUSTMENT( vadj ) );
		gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pgServerScroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
		gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( pgServerScroll ), pgServerList );
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pgServerScroll, 0, 4, 0, 19 );
		
		/* Player List */
		gchar* playerListTitles[1] = { ( gchar* ) "Name" };
		pgPlayerList = gtk_clist_new_with_titles( 1, playerListTitles );
		GtkObject* hadj2 = gtk_adjustment_new( 0.0, 0.0, 1.0, 0.01, 0.1, 0.1 );
		GtkObject* vadj2 = gtk_adjustment_new( 0.0, 0.0, 1.0, 0.01, 0.1, 0.1 );
		GtkWidget* pgPlayerScroll = gtk_scrolled_window_new( GTK_ADJUSTMENT( hadj2 ), GTK_ADJUSTMENT( vadj2 ) );
		gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pgPlayerScroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
		gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( pgPlayerScroll ), pgPlayerList );
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pgPlayerScroll, 4, 5, 0, 19 );
		
		/* Filter */
		GtkWidget* pgFilter = gtk_entry_new( );
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pgFilter, 0, 1, 19, 20 );
		gtk_signal_connect( GTK_OBJECT( pgFilter ), "changed", GTK_SIGNAL_FUNC( &ChangeFilter ), this );
		
		pgStatus = gtk_label_new( "Querying Game-Monitor.com" );
#ifdef WIN32
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pgStatus, 1, 2, 19, 20 );
		
		pgButtonConnect = gtk_button_new_with_label( "Connect" );
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pgButtonConnect, 2, 3, 19, 20 );
		gtk_signal_connect( GTK_OBJECT( pgButtonConnect ), "clicked", GTK_SIGNAL_FUNC( &Connect ), this );
#else
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pgStatus, 1, 3, 19, 20 );
#endif
		/* Refresh Button */
		GtkWidget* pButtonRefresh = gtk_button_new_with_label( "Refresh" );
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pButtonRefresh, 3, 4, 19, 20 );
		gtk_signal_connect( GTK_OBJECT( pButtonRefresh ), "clicked", GTK_SIGNAL_FUNC( &Refresh ), this );
		
		/* Close Button */
		GtkWidget* pButtonClose = gtk_button_new_with_label( "Close" );
		gtk_table_attach_defaults( GTK_TABLE( pLayout ), pButtonClose, 4, 5, 19, 20 );
		gtk_signal_connect( GTK_OBJECT( pButtonClose ), "clicked", GTK_SIGNAL_FUNC( &Destroy ), this );
		
		/* Show our windows */
		gtk_widget_show_all( pgWindow );
		
		/* we want to be updated! */
		gtk_timeout_add( FETCH_NEW_SERVERS_INTERVAL, &Pulse, this );
		
		/* Closing handler */
		g_signal_connect_swapped( G_OBJECT( pgWindow ), "destroy", G_CALLBACK( &Destroy ), this );
		
		/* Create our server list class and let it fetch all servers. */
		pServerList = new ServerList( );
		SetRefreshOnNextPulse( true );
		
		SetSelectedServer( NULL );
	}
	
	/* Should we even do this? GTK can handle deletion of its widgets? */
	Window::~Window( )
	{
		if( pServerList )
			delete pServerList;
	}
	
	/* Adds a single server to the list */
	void Window::AddToServerList( Server* server )
	{
		gchar* content[5];
		content[0] = ( gchar* ) server->GetName( ).c_str( );
		
		GString* temp = g_string_new( NULL );
		g_string_sprintf( temp, "%d/%d", ( unsigned int ) server->GetNumPlayers( ), ( unsigned int ) server->GetMaxPlayers( ) );
		content[1] = temp->str;
		
		content[2] = ( gchar* ) server->GetGameType( ).c_str( );
		content[3] = ( gchar* ) server->GetMapName( ).c_str( );
		
		GString* temp2 = g_string_new( NULL );
		g_string_sprintf( temp2, "%s:%d", server->GetIP( ).c_str( ), server->GetPort( ) );
		content[4] = temp2->str;
		
		gint row = gtk_clist_append( GTK_CLIST( pgServerList ), content );
		gtk_clist_set_row_data( GTK_CLIST( pgServerList ), row, server );
	}
	
	/* Returns true if the list should be refreshed on the next pulse */
	const bool Window::GetRefreshOnNextPulse( )
	{
		return refreshOnNextPulse;
	}
	
	/* Sets whetever the list should be refreshed on next pulse (i.e. creating the window, clicking 'Refresh') */
	void Window::SetRefreshOnNextPulse( bool b )
	{
		refreshOnNextPulse = b;
	}
	
	/* Called to let us handle our internal stuff. */
	gint Window::Pulse( gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		GtkWidget* pgServerList = pWindow->GetServerListWidget( );
		assert( pgServerList );
		
		ServerList* pServerList = pWindow->GetServerList( );
		assert( pServerList );
		
		/* If we were asked to refresh the list, do so */
		if( pWindow->GetRefreshOnNextPulse( ) )
		{
			pWindow->SetRefreshOnNextPulse( false );
			const string error = pServerList->Refresh( );
			if( error.length( ) > 0 )
			{
				pWindow->UpdateStatusLabel( error );
				return true;
			}
		}
		
		/* Check if new servers are available */
		list < Server* > newServers = pServerList->Pulse( );
		if( newServers.size( ) > 0 )
		{
			/* Just freeze the list for the time being to stop it from flashing */
			gtk_clist_freeze( GTK_CLIST( pgServerList ) );
			
			/* Add each item */
			for( list < Server* >::iterator iter = newServers.begin( ); iter != newServers.end( ); ++ iter )
				pWindow->AddToServerList( *iter );
			
			/* Sort me */
			gtk_clist_sort( GTK_CLIST( pgServerList ) );
			
			/* Unfreeze the list again to have it updated */
			gtk_clist_thaw( GTK_CLIST( pgServerList ) );

			/* Update our status */
			pWindow->UpdateStatusLabel( pServerList->GetStatus( ) );
		}
		
		/* We want to continue execution of our function. */
		return true;
	}
	
	/* Called whenever we click 'Refresh' */
	void Window::Refresh( GtkWidget* pWidget, gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		/* Set the new status */
		pWindow->UpdateStatusLabel( "Querying Game-Monitor.com" );
		
		/* Clear the old list */
		GtkWidget* pgServerList = pWindow->GetServerListWidget( );
		assert( pgServerList );
		gtk_clist_clear( GTK_CLIST( pgServerList ) );
		
		/* Clear the old player list */
		GtkWidget* pgPlayerList = pWindow->GetPlayerListWidget( );
		assert( pgPlayerList );
		gtk_clist_clear( GTK_CLIST( pgPlayerList ) );
		
		/* Refresh the List */
		pWindow->SetRefreshOnNextPulse( true );
		
		/* We don't have no server no more */
		pWindow->SetSelectedServer( NULL );
	}
	
#ifdef WIN32
	/* Called whenever we click 'Connect' */
	void Window::Connect( GtkWidget* pWidget, gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		Server* pServer = pWindow->GetSelectedServer( );
		if( pServer )
		{
			GString* temp = g_string_new( NULL );
			g_string_sprintf( temp, "mtasa://%s:%d", pServer->GetIP( ).c_str( ), pServer->GetPort( ) );
			ShellExecute( NULL, "open", temp->str, NULL, NULL, SW_SHOWNORMAL );
		}
	}
#endif
	
	/* Called whenever we click 'Close' or the X. Obviously we want to exit */
	void Window::Destroy( GtkWidget* pWidget, gpointer data )
	{
#if 0
		/* This application has exceptions when exitting, probably caused by this code */
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		delete pWindow;
#endif
		
		/* Let GTK handle itself. */
		gtk_main_quit( );
	}
	
	/* Someone selected a server */
	void Window::SelectServer( GtkWidget* pWdidget, gint row, gint column, GdkEventButton* event, gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		/* Clear the old player list */
		GtkWidget* pgPlayerList = pWindow->GetPlayerListWidget( );
		assert( pgPlayerList );
		gtk_clist_clear( GTK_CLIST( pgPlayerList ) );
		
		/* Get the List */
		GtkWidget* pgServerList = pWindow->GetServerListWidget( );
		assert( pgServerList );
		
		/* Get our Row data (Pointer to the Server result) */
		Server* server = static_cast < Server* > ( gtk_clist_get_row_data( GTK_CLIST( pgServerList ), row ) );
		assert( server );
		
		/* If the server has no players, we're done */
		list < Player* > players = server->GetPlayers( );
		if( players.size( ) > 0 )
		{
			/* Just freeze the list for the time being to stop it from flashing */
			gtk_clist_freeze( GTK_CLIST( pgPlayerList ) );
			
			/* Add each item */
			for( list < Player* >::iterator iter = players.begin( ); iter != players.end( ); ++ iter )
			{
				gchar* content[1];
				content[0] = ( gchar* ) (*iter)->GetName( ).c_str( );
				
				gtk_clist_append( GTK_CLIST( pgPlayerList ), content );
			}
			
			/* Sort me */
			gtk_clist_sort( GTK_CLIST( pgPlayerList ) );
			
			/* Unfreeze the list again to have it updated */
			gtk_clist_thaw( GTK_CLIST( pgPlayerList ) );
		}
		
		/* Save for later use */
		pWindow->SetSelectedServer( server );
	}
	
	/* Someone selected no server, tsk. */
	void Window::SelectNoServer( GtkWidget* pWdidget, gint row, gint column, GdkEventButton* event, gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		/* Clear the old player list */
		GtkWidget* pgPlayerList = pWindow->GetPlayerListWidget( );
		assert( pgPlayerList );
		gtk_clist_clear( GTK_CLIST( pgPlayerList ) );
	}
	
	/* We changed the filter */
	void Window::ChangeFilter( GtkEntry* pEntry, gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		/* Clear the old list */
		GtkWidget* pgServerList = pWindow->GetServerListWidget( );
		assert( pgServerList );
		gtk_clist_clear( GTK_CLIST( pgServerList ) );
		
		/* Clear the old player list */
		GtkWidget* pgPlayerList = pWindow->GetPlayerListWidget( );
		assert( pgPlayerList );
		gtk_clist_clear( GTK_CLIST( pgPlayerList ) );
		
		/* Refresh the List */
		ServerList* pServerList = pWindow->GetServerList( );
		assert( pServerList );
		list < Server* > newServers = pServerList->Filter( string( ( char* ) gtk_entry_get_text( pEntry ) ) );
		if( newServers.size( ) > 0 )
		{
			/* Just freeze the list for the time being to stop it from flashing */
			gtk_clist_freeze( GTK_CLIST( pgServerList ) );
			
			/* Add each item */
			for( list < Server* >::iterator iter = newServers.begin( ); iter != newServers.end( ); ++ iter )
				pWindow->AddToServerList( *iter );
			
			/* Sort me */
			gtk_clist_sort( GTK_CLIST( pgServerList ) );
			
			/* Unfreeze the list again to have it updated */
			gtk_clist_thaw( GTK_CLIST( pgServerList ) );
		}
		
		/* Save for later use */
		pWindow->SetSelectedServer( NULL );

		/* Update our status */
		pWindow->UpdateStatusLabel( pServerList->GetStatus( ) );
	}
	
	void Window::SetSortColumn( GtkCList* list, gint column, gpointer data )
	{
		Window* pWindow = static_cast < Window* > ( data );
		assert( pWindow );
		
		GtkWidget* pgServerList = pWindow->GetServerListWidget( );
		assert( pgServerList );
		
		if( column == list->sort_column )
		{
			/* Selected same column again, change ascending/descending */
			gtk_clist_set_sort_type( GTK_CLIST( pgServerList ), ( GtkSortType ) ( 1 - list->sort_type ) );
		}
		else
		{
			/* We selected another column */
			gtk_clist_set_sort_column( GTK_CLIST( pgServerList ), column );
			if( column == 1 )
				gtk_clist_set_compare_func( GTK_CLIST( pgServerList ), &ComparePlayers );
			else
				gtk_clist_set_compare_func( GTK_CLIST( pgServerList ), NULL );
		}
		
		gtk_clist_sort( GTK_CLIST( pgServerList ) );
	}
	
	/* Let's compare us! We need a custom definition since we want this to be by player count. */
	gint Window::ComparePlayers( GtkCList* list, gconstpointer ptr1, gconstpointer ptr2 )
	{
		GtkCListRow* row1 = ( GtkCListRow* ) ptr1;
		GtkCListRow* row2 = ( GtkCListRow* ) ptr2;
		
		char* text1 = GTK_CELL_TEXT( row1->cell[list->sort_column] )->text;
		char* text2 = GTK_CELL_TEXT( row2->cell[list->sort_column] )->text;
		
		if( !text2 )
			return -( text1 != NULL );
		if( !text1 )
			return 1;
		
		unsigned char
			players1 = 0,
			maxplayers1 = 0,
			players2 = 0,
			maxplayers2 = 0;
		
		/* strtok + atoi segfaults here */
		for( unsigned char pos = 0; pos < strlen( text1 ); ++ pos )
			if( text1[pos] >= '0' && text1[pos] <= '9' )
				players1 = players1 * 10 + text1[pos] - '0';
			else if( text1[pos] == '/' )
			{
				for( unsigned char pos2 = pos + 1; pos2 < strlen( text1 ); ++ pos2 )
					if( text1[pos2] >= '0' && text1[pos2] <= '9' )
						maxplayers1 = maxplayers1 * 10 + text1[pos2] - '0';
				break;
			}
		
		for( unsigned char pos = 0; pos < strlen( text2 ); ++ pos )
			if( text2[pos] >= '0' && text2[pos] <= '9' )
				players2 = players2 * 10 + text2[pos] - '0';
			else if( text2[pos] == '/' )
			{
				for( unsigned char pos2 = pos + 1; pos2 < strlen( text2 ); ++ pos2 )
					if( text2[pos2] >= '0' && text2[pos2] <= '9' )
						maxplayers2 = maxplayers2 * 10 + text2[pos2] - '0';
				break;
			}
		
		if( players1 > players2 )
			return -1;
		else if( players1 == players2 )
		{
			if( maxplayers1 > maxplayers2 )
				return -1;
			else if( maxplayers1 == maxplayers2 )
				return 0;
			else
				return 1;
		}
		else
			return 1;
	}
	
	/* Returns the Server List */
	ServerList* Window::GetServerList( )
	{
		return pServerList;
	}
	
	/* Returns the Server List */
	Server* Window::GetSelectedServer( )
	{
		return pSelectedServer;
	}
	
	/* Returns the Server List */
	void Window::SetSelectedServer( Server* server )
	{
		pSelectedServer = server;
#ifdef WIN32
		gtk_widget_set_sensitive( GTK_WIDGET( pgButtonConnect ), server != NULL );
#endif
	}
	
	/* Returns the Server List Widget */
	GtkWidget* Window::GetServerListWidget( )
	{
		return pgServerList;
	}
	
	/* Returns the Player List Widget */
	GtkWidget* Window::GetPlayerListWidget( )
	{
		return pgPlayerList;
	}
	
	/* Updates the Status Label to display a certain text */
	void Window::UpdateStatusLabel( string text )
	{
		assert( pgStatus );
		gtk_label_set_text( GTK_LABEL( pgStatus ), ( gchar* ) text.c_str( ) );
	}
}
