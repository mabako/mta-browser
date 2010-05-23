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

#include <string.h>
#include <stdlib.h>
#include "window.h"

namespace browse
{
	Window::Window( )
	{
		/* Create the window */
		pgWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		
		/* adjust some properties */
		gtk_window_set_title( GTK_WINDOW( pgWindow ), APPLICATION_TITLE );
		gtk_window_set_default_size( GTK_WINDOW( pgWindow ), 1000, 480 );
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
		gtk_clist_set_sort_column( GTK_CLIST( pgServerList ), 1 ); /* FIXME: You should be able to select other columns to sort by */
		gtk_clist_set_compare_func( GTK_CLIST( pgServerList ), &ComparePlayers );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 0, 250 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 1, 60 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 2, 130 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 3, 130 );
		gtk_clist_set_column_width( GTK_CLIST( pgServerList ), 4, 150 );
		gtk_signal_connect( GTK_OBJECT( pgServerList ), "select-row", GTK_SIGNAL_FUNC( &SelectServer ), this );
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
		gtk_timeout_add( 50, &Pulse, this );
		
		/* Closing handler */
		g_signal_connect_swapped( G_OBJECT( pgWindow ), "destroy", G_CALLBACK( &Destroy ), this );
		
		/* Create our server list class and let it fetch all servers. */
		pServerList = new ServerList( );
		if( pServerList )
			pServerList->Refresh( );
	}
	
	/* Should we even do this? GTK can handle deletion of its widgets? */
	Window::~Window( )
	{
		if( pServerList )
			delete pServerList;
	}
	
	/* Called to let us handle our internal stuff. */
	gint Window::Pulse( gpointer data )
	{
		Window* pgWindow = static_cast < Window* > ( data );
		assert( pgWindow );
		
		GtkWidget* pgServerList = pgWindow->GetServerListWidget( );
		assert( pgServerList );
		
		ServerList* pServerList = pgWindow->GetServerList( );
		assert( pServerList );
		
		/* Check if new servers are available */
		list < Server* > newServers = pServerList->Pulse( );
		if( newServers.size( ) > 0 )
		{
			/* Just freeze the list for the time being to stop it from flashing */
			gtk_clist_freeze( GTK_CLIST( pgServerList ) );
			
			/* Add each item */
			for( list < Server* >::iterator iter = newServers.begin( ); iter != newServers.end( ); ++ iter )
			{
				gchar* content[5];
				content[0] = ( gchar* ) (*iter)->GetName( ).c_str( );
				
				GString* temp = g_string_new( NULL );
				g_string_sprintf( temp, "%d/%d", ( unsigned int ) (*iter)->GetNumPlayers( ), ( unsigned int ) (*iter)->GetMaxPlayers( ) );
				content[1] = temp->str;
				
				content[2] = ( gchar* ) (*iter)->GetGameType( ).c_str( );
				content[3] = ( gchar* ) (*iter)->GetMapName( ).c_str( );
				
				GString* temp2 = g_string_new( NULL );
				g_string_sprintf( temp2, "%s:%d", (*iter)->GetIP( ).c_str( ), (*iter)->GetPort( ) );
				content[4] = temp2->str;
				
				gint row = gtk_clist_append( GTK_CLIST( pgServerList ), content );
				gtk_clist_set_row_data( GTK_CLIST( pgServerList ), row, *iter );
			}
			
			/* Unfreeze the list again to have it updated */
			gtk_clist_thaw( GTK_CLIST( pgServerList ) );
			
			/* Sort me */
			gtk_clist_sort( GTK_CLIST( pgServerList ) );
		}
		
		/* We want to continue execution of our function. */
		return true;
	}
	
	/* Called whenever we click 'Refresh' */
	void Window::Refresh( GtkWidget* pWidget, gpointer data )
	{
		Window* pgWindow = static_cast < Window* > ( data );
		assert( pgWindow );
		
		/* Clear the old list */
		GtkWidget* pgServerList = pgWindow->GetServerListWidget( );
		assert( pgServerList );
		gtk_clist_clear( GTK_CLIST( pgServerList ) );

		
		/* Refresh the List */
		ServerList* pServerList = pgWindow->GetServerList( );
		assert( pServerList );
		pServerList->Refresh( );
	}
	
	/* Called whenever we click 'Close' or the X. Obviously we want to exit */
	void Window::Destroy( GtkWidget* pWidget, gpointer data )
	{
		Window* pgWindow = static_cast < Window* > ( data );
		assert( pgWindow );
		delete pgWindow;
		
		/* Let GTK handle itself. */
		gtk_main_quit( );
	}
	
	/* Someone selected a server */
	void Window::SelectServer( GtkWidget* pWdidget, gint row, gint column, GdkEventButton* event, gpointer data )
	{
		Window* pgWindow = static_cast < Window* > ( data );
		assert( pgWindow );
		
		/* Clear the old player list */
		GtkWidget* pgPlayerList = pgWindow->GetPlayerListWidget( );
		assert( pgPlayerList );
		gtk_clist_clear( GTK_CLIST( pgPlayerList ) );
		
		/* Get the List */
		GtkWidget* pgServerList = pgWindow->GetServerListWidget( );
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
				
				gint row = gtk_clist_append( GTK_CLIST( pgPlayerList ), content );
			}
			
			/* Unfreeze the list again to have it updated */
			gtk_clist_thaw( GTK_CLIST( pgPlayerList ) );
			
			/* Sort me */
			gtk_clist_sort( GTK_CLIST( pgPlayerList ) );
		}
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
			players2 = 0;
		
		/* strtok + atoi segfaults here */
		for( unsigned char pos = 0; pos < strlen( text1 ); ++ pos )
			if( text1[pos] >= '0' and text1[pos] <= '9' )
				players1 = players1 * 10 + text1[pos] - '0';
			else if( text1[pos] == '/' )
				break;
		
		for( unsigned char pos = 0; pos < strlen( text2 ); ++ pos )
			if( text2[pos] >= '0' and text2[pos] <= '9' )
				players2 = players2 * 10 + text2[pos] - '0';
			else if( text2[pos] == '/' )
				break;
		
		if( players1 > players2 )
			return -1;
		else if( players1 == players2 )
			return 0;
		else
			return 1;
	}
	
	/* Returns the Server List */
	ServerList* Window::GetServerList( )
	{
		return pServerList;
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
}