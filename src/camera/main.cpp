#include <iostream>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <stdio.h>

#include "cam_core.h"

#define errExit(msg) do {perror(msg); exit(EXIT_FAILURE);}while(0)

static DBusHandlerResult dbus_filter(DBusConnection *conn, DBusMessage *message, void *user_data)
{
				DBusMessageIter args;
				
				if(dbus_message_is_signal(message, "org.opel.camera.daemon", "init"))
				{
						std::cout << "Get Initialization Service Get\n";
						return DBUS_HANDLER_RESULT_HANDLED;
				}


				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
int main()
{
				CameraProperty* camProp = CameraProperty::getInstance();
//				camProp->~CameraProperty();
				
				Camera* cam = Camera::getInstance();
				if(!cam->open_device()){
								errExit("open_device Error!!!\n"); }
				std::cout << "Open Device Success\n";
				if(!cam->init_device()){
								errExit("init_device Error!!!\n"); }
				std::cout << "Init Device Success\n";
				if(!cam->start_capturing()){
								errExit("start_capturing Error!!!\n");}
				std::cout << "Capturing Done\n";
				if(!cam->stop_capturing()){
								errExit("stop_capturing Error!!!\n");} 
				if(!cam->uninit_device()){
								errExit("uninit_device Error!!!\n");}
				if(!cam->close_device()){
								errExit("close_device Error!!!\n");
				}

				
	/*
				
				DBusConnection *conn;
				DBusError err;
				GMainLoop *loop;
				loop = g_main_loop_new(NULL, false);
				dbus_error_init(&err);
				if(dbus_error_is_set(&err))
				{
								printf("Error Connecting to the Daemon Bus : %s", err.message);
								dbus_error_free(&err);
								return 1;
				}
				dbus_bus_add_match(conn, "type='signal',interface='org.opel.camera.daemon'", NULL);
				dbus_connection_add_filter(conn, dbus_filter, loop, NULL);
				dbus_connection_setup_with_g_main(conn, NULL);
				g_main_loop_run(loop);
*/

				return 0;
}
