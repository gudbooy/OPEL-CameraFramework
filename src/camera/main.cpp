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
				if(dbus_message_is_signal(message, "org.opel.camera.daemon", "start"))
				{	
						std::cout << "Get Start Service Get\n";
						return DBUS_HANDLER_RESULT_HANDLED;
				}


				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
int main()
{
					OpenCVSupport* cam = new OpenCVSupport();
					CameraProperty* camProp = cam->getCameraProperty();
					camProp->printSetValue();
					if(!cam->open())
					{
									fprintf(stderr, "DEVICE OPEN FAIL\n");
									exit(-1);
					}
					if(!cam->init_device())
					{
									fprintf(stderr, "INIT DEVICE FAIL\n");
									exit(-1);
					}
					if(!cam->start())
					{
									fprintf(stderr, "START FAIL\n");
									exit(-1);
					}
					if(!cam->stop())
					{
									fprintf(stderr, "STOP FAIL\n");
									exit(-1);
					}
					if(!cam->close_device())
					{
									fprintf(stderr, "CLOSE DEVICE FAIL\n");
									exit(-1);
					}


					
					
					cam->deleteCameraProperty();
					delete cam;	
						
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
