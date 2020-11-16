# wps
wps using gdbus

This code does wps using gdbus api's .

PreRequiste is wpa_supplicant and gdbus are required

compile the code 

gcc -o dbus_wps.out dbus_wps.c $(pkg-config --cflags --libs glib-2.0 gio-2.0)

run 

sudo ./dbus_wps.out
