mta:
	g++ -o browse server.cpp serverlistitem.cpp serverlist.cpp window.cpp browse.cpp -O2 -DNDEBUG `pkg-config --cflags --libs gtk+-2.0` `curl-config --libs`
	strip browse
