#ifndef _CAM_STATUS_H_
#define _CAM_STATUS_H_
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <fcntl.h>
#define OPENCV_PROP_SHM 1588


class CameraStatus
{
	public:
		~CameraStatus();
   void getThrMutex(pthread_mutex_t& mutex);
   static CameraStatus* getInstance(void);
	 bool getIsOpenCVRunning() { return this->isOpenCVRunning; }
	 void setIsOpenCVRunning(bool isRunning) 
	 { 
		 sem_wait(this->statusMutex);
		 this->isOpenCVRunning = isRunning; 
	 	 sem_post(this->statusMutex);
	 }
	 bool getIsOpenCVInitialized() { return this->isOpenCVInitialized; }
	 void setIsOpenCVInitialized(bool isInitialized) 
	 { 
		 sem_wait(this->statusMutex);
		 this->isOpenCVInitialized = isInitialized; 
	 	 sem_post(this->statusMutex);
	 }
	
	 bool getIsRecRunning() { return this->isRecRunning; }
	 void setIsRecRunning(bool isRunning) 
	 {
		 sem_wait(this->statusMutex);
		 this->isRecRunning = isRunning; 
	 	 sem_post(this->statusMutex);
	 };
	 bool getIsRecInitialized() { return this->isRecInitialized; }
	 void setIsRecInitialized(bool isInitialized) 
	 { 
		 sem_wait(this->statusMutex);
		 this->isRecInitialized 	= isInitialized; 
	 	 sem_post(this->statusMutex);
	 }
	
	private:
	  static CameraStatus* camStats;
		CameraStatus();
		bool isCCDRunning;
		bool isOpenCVRunning;
		bool isOpenCVInitialized;
		bool isRecRunning;
		bool isRecInitialized;
		pthread_mutex_t mutex;
		sem_t* statusMutex;    
//		const char* mutex_path = "camStatusMutex";
};

#endif   /*_CAM_STATUS_H_*/
