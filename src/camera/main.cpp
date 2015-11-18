#include <iostream>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "cam_core.h"
#include "cam_status.h"

#define errExit(msg) do {perror(msg); exit(EXIT_FAILURE);}while(0)
#define NUM_OF_THREADS 2

#define INDEX_OF_OPENCV_THR 0
#define INDEX_OF_REC_THR 1
static CameraProperty* openCV_camProp;
static CameraProperty* rec_camProp;
static OpenCVSupport* rec_cam;
static OpenCVSupport* openCV_cam;

static bool is_openCVCamInit;

static pthread_t OPELCamThread[NUM_OF_THREADS]; 
int thr_id[2] = {0,};


void* openCVCameraSupportThr(void* args)
{
		if(!openCV_cam->start())
		{
			fprintf(stderr, "START FAIL\n");
			return NULL;
		}
		return (void*)1;
}

static DBusHandlerResult dbus_filter(DBusConnection *conn, DBusMessage *message, void *user_data)
{
	DBusMessageIter args;

	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "OpenCVInit"))
	{
		is_openCVCamInit = false;
		std::cout << "Get[DBUS] : OpenCVInit\n";
		rec_camProp->printSetValue();
		if(!openCV_cam->open())
		{
			fprintf(stderr, "DEVICE OPEN FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		if(!openCV_cam->init_device())
		{
			fprintf(stderr, "DEVICE INIT FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		is_openCVCamInit = true;
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "OpenCVStart"))
	{	
		std::cout << "Get[DBUS] : OpenCVStart\n";
	  //Checking that CamProp is initialized 	
	  if(is_openCVCamInit){
		  //Running Thread for Recording Thread
			thr_id[INDEX_OF_OPENCV_THR] = pthread_create(&OPELCamThread[0], NULL,  openCVCameraSupportThr, (void*)0);
		}
		else
		{
			fprintf(stderr, "Camera Property Not Initialized Yet\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "OpenCVStop"))
	{
		std::cout << "Get[DBUS] : OpenCVStop\n";
		//Stop the Thread for Recording Thread
	
		return DBUS_HANDLER_RESULT_HANDLED;
	}
/*
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "openCVInit"))
	{	
		std::cout << "Get Start Service Get\n";
		openCV_camProp->printSetValue();
		//Stop the Thread for Recording Thread


		return DBUS_HANDLER_RESULT_HANDLED;
	}
	
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "openCVStart"))
	{	
		std::cout << "Get Start Service Get\n";
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "openCVStop"))
	{	
		std::cout << "Get Start Service Get\n";
		return DBUS_HANDLER_RESULT_HANDLED;
	}
*/




	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int main()
{
	

	bool isRec = true;		
	openCV_camProp = new CameraProperty(!isRec);
	rec_camProp = new CameraProperty(isRec);

	rec_cam = new OpenCVSupport();
	openCV_cam = new OpenCVSupport();

	openCV_cam->setCameraProperty(openCV_camProp);
	rec_cam->setCameraProperty(rec_camProp);

	DBusConnection *conn;
	DBusError err;
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, false);
	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
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


				return 0;
}
