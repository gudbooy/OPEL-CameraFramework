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
		this->statusMutex = sem_open("camStatus", O_CREAT, 0666, 1);
		if(this->statusMutex == SEM_FAILED)
		{
			fprintf(stderr, "[CameraStatus::CameraStatus]: Sem_open Error\n");
			sem_unlink("camStatus");	
			this->statusMutex = NULL;
		}	
}
CameraStatus::~CameraStatus()
{
	sem_close(this->statusMutex);
	sem_unlink("camStatus"); 
}
CameraStatus* CameraStatus::getInstance(void)
{
	if(camStats == (void*)0)
		camStats = new CameraStatus();
	return camStats;
}

