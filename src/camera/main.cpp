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
CameraStatus* camStatus;
pthread_mutex_t mutex_lock;

//CameraStatus* camStatus;

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
		
//		is_openCVCamInit = false;
		std::cout << "Get[DBUS] : OpenCVInit\n";
		//Recording Thread Can be preempted by OpenCV 
		// Prioirity of Rec < Priority of OpenCV
		//check if the recording is running
		if(camStatus->getIsRecRunning() || camStatus->getIsRecInitialized())
		{
			//Impementation need	
		}
		openCV_camProp->printSetValue();
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
		camStatus->setIsOpenCVInitialized(true);
    //is_openCVCamInit = true;
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "OpenCVStart"))
	{	
		std::cout << "Get[DBUS] : OpenCVStart\n";
	  //Checking that CamProp is initialized 	
		if(camStatus->getIsOpenCVInitialized()){
		  openCV_cam->setEos(true);
		  //Running Thread for Recording Thread
			thr_id[INDEX_OF_OPENCV_THR] = pthread_create(&OPELCamThread[0], NULL,  openCVCameraSupportThr, (void*)0);
			if(thr_id < 0)
			{
				fprintf(stderr, "Create Thread Failed\n");
				return DBUS_HANDLER_RESULT_HANDLED;
			}
			camStatus->setIsOpenCVRunning(true);
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
 		openCV_cam->setEos(false);
		if(!openCV_cam->stop())
		{
			fprintf(stderr, "DEVICE STOP FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		camStatus->setIsOpenCVRunning(false);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "OpenCVClose"))
	{
		std::cout << "Get[DBUS] : OpenCVClose\n";
		//Stop the Thread for Recording Thread
		openCV_cam->setEos(false);
		if(!openCV_cam->stop())
		{
			fprintf(stderr, "DEVICE STOP FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		if(-1 == pthread_detach(OPELCamThread[0]))
		{
			fprintf(stderr, "Detatching the p_thread Failed\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		if(!openCV_cam->close_device())
		{
			fprintf(stderr, "DEVICE CLOSE FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		camStatus->setIsOpenCVRunning(false);
		camStatus->setIsOpenCVInitialized(false);
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
	pthread_mutex_t mutex_lock;
	openCV_camProp = new CameraProperty(!isRec);
	rec_camProp = new CameraProperty(isRec);

	rec_cam = new OpenCVSupport();
	openCV_cam = new OpenCVSupport();

	openCV_cam->setCameraProperty(openCV_camProp);
	rec_cam->setCameraProperty(rec_camProp);
  pthread_mutex_init(&mutex_lock, NULL);
	camStatus = CameraStatus::getInstance();
	camStatus->getThrMutex(mutex_lock);
	openCV_cam->setThrMutex(mutex_lock);

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
