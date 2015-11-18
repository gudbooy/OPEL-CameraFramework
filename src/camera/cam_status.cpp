#include "cam_status.h"
CameraStatus* CameraStatus::camStats = NULL;



void CameraStatus::getThrMutex(pthread_mutex_t& mutex) 
{
	this->mutex = mutex;
}
CameraStatus::CameraStatus()
{

		this->isCCDRunning = false;
		this->isOpenCVRunning = false;
		this->isOpenCVInitialized = false;
		this->isRecRunning = false;
		this->isRecInitialized = false;

}
CameraStatus* CameraStatus::getInstance(void)
{
	if(camStats == (void*)0)
		camStats = new CameraStatus();
	return camStats;
}

