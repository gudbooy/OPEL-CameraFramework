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
#include "cam_property.h"
#define errExit(msg) do {perror(msg); exit(EXIT_FAILURE);}while(0)
#define NUM_OF_THREADS 2

#define INDEX_OF_OPENCV_THR 0
#define INDEX_OF_REC_THR 1
static CameraProperty* openCV_camProp;
static CameraProperty* rec_camProp;
static OpenCVSupport* rec_cam;
static OpenCVSupport* openCV_cam;

//static property* recordingProperty;
static property* openCVProperty;

CameraStatus* camStatus;
pthread_mutex_t mutex_lock;
bool first;

//CameraStatus* camStatus;

static bool is_openCVCamInit;

static pthread_t OPELCamThread[NUM_OF_THREADS]; 
int thr_id[2] = {0,};

void* recCameraSupportThr(void* args)
{
	if(!rec_cam->start())
	{
		fprintf(stderr, "START FIALED\n");
		return NULL;
	}
	return (void*)1;
}
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
			fprintf(stderr, "Recording Process is Running\n");
			return DBUS_HANDLER_RESULT_HANDLED;
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
			thr_id[INDEX_OF_OPENCV_THR] = pthread_create(&OPELCamThread[INDEX_OF_OPENCV_THR], NULL,  openCVCameraSupportThr, (void*)0);
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
		if(-1 == pthread_detach(OPELCamThread[0]))
		{
			fprintf(stderr, "[DBUS_OPENCVSTOP] : pthread_detach Error\n");
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
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "recInit"))
	{	
		std::cout << "Get[DBUS] : recInit\n";
		rec_camProp->printSetValue();
    //if OpenCV Service is running then stop the service
		if(camStatus->getIsOpenCVRunning() || camStatus->getIsOpenCVInitialized())
		{
				if(!openCV_cam->stop())
				{
						fprintf(stderr, "STOP FAILED\n");
				}
				if(!openCV_cam->close_device())
				{
					fprintf(stderr, "CLOSE DEVICE FAILED\n");
				}
				//stop and uninit openCV
				camStatus->getIsOpenCVRunning() ? camStatus->setIsOpenCVRunning(false) : camStatus->setIsOpenCVInitialized(false);
		}
		
		if(!rec_cam->open())
		{
			fprintf(stderr, "DEVICE INIT FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		if(!rec_cam->init_device())
		{
			fprintf(stderr, "DEVICE INIT FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}	
		camStatus->setIsRecInitialized(true);
		//Stop the Thread for Recording Thread
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "recStart"))
	{	
		std::cout << "Get[DBUS] : recStart\n";
	  rec_cam->setEos(true);
		if(camStatus->getIsRecInitialized()){
		thr_id[INDEX_OF_REC_THR] = pthread_create(&OPELCamThread[INDEX_OF_REC_THR], NULL, recCameraSupportThr, (void*)0);
		if(thr_id[INDEX_OF_REC_THR] < 0)
		{
			fprintf(stderr, "CREATE THREAD FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		camStatus->setIsRecRunning(true);
		}
		else
		{
			fprintf(stderr, "CAMERA PROPERTY NOT INITIALIZED YET\n");
			return DBUS_HANDLER_RESULT_HANDLED;	
		}
	  return DBUS_HANDLER_RESULT_HANDLED;	
	}

	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "recStop"))
	{	
		std::cout << "Get[DBUS] : recStop\n";	 
		rec_cam->setEos(false);
		if(!rec_cam->stop())
		{
			fprintf(stderr, "DEVICE STOP FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		camStatus->setIsRecRunning(false);

		return DBUS_HANDLER_RESULT_HANDLED;
	}
	if(dbus_message_is_signal(message, "org.opel.camera.daemon", "recClose"))
	{	
		std::cout << "Get[DBUS] : recClose\n";
		rec_cam->setEos(false);
		if(!rec_cam->stop())
		{
			fprintf(stderr, "DEVICE STOP FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		if(-1 == pthread_detach(OPELCamThread[INDEX_OF_REC_THR]))
		{
			fprintf(stderr, "Detatching the p_thread failed\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		if(!rec_cam->close_device())
		{
			fprintf(stderr, "DEVICE CLOSE FAILED\n");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		camStatus->setIsRecRunning(false);
		camStatus->setIsRecInitialized(false);
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int main()
{
	bool isRec = true;		
	pthread_mutex_t mutex_lock;
	openCV_camProp = new CameraProperty(!isRec);
	rec_camProp = new CameraProperty(isRec);


//	openCV_camProp->initProperty(&openCVProperty);	
	openCV_camProp->InitSharedPropertyToTarget(openCVProperty);	
//	openCV_camProp->initProperty(openCVProperty);

	rec_cam = new OpenCVSupport();
	openCV_cam = new OpenCVSupport();

	openCV_cam->setCameraProperty(openCV_camProp);
	rec_cam->setCameraProperty(rec_camProp);
  pthread_mutex_init(&mutex_lock, NULL);
	
	camStatus = CameraStatus::getInstance();

	//	openCVProperty->camStatus = camStatus;
	 	
	camStatus->getThrMutex(mutex_lock);
	openCV_cam->setThrMutex(mutex_lock);
	
	first = true;
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
