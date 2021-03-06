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
#include <iostream>
#include <gtk/gtk.h>
#include "window.hpp"

using namespace std;

#ifdef WIN32
int
#ifdef _MSC_VER
	/* Visual Studio wants us to use a stdcall, let's go for it */
	__stdcall
#endif
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	/* Call GTK's initalization */
	gtk_init( NULL, NULL );
	new browse::Window( );
	gtk_main( );
	
	return 0;
}
#else
int main( int argc, char *argv[] )
{
	if( argc == 2 && ( !strcmp( argv[1], "-v" ) || !strcmp( argv[1], "--version" ) ) )
	{
		/* Small Version text */
		cout << "browse - A GTK+ based MTA:SA Server Browser - Version " << VERSION << endl;
	}
	else
	{
		/* Call GTK's initalization */
		gtk_init( &argc, &argv );
	
		new browse::Window( );
	
		gtk_main( );
	}
	return 0;
}
#endif

