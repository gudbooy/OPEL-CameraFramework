#ifndef _CAM_STATUS_H_
#define _CAM_STATUS_H_
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>


#define OPENCV_PROP_SHM 1588



class CameraStatus
{
	public:
//		~CameraStatus();
   void getThrMutex(pthread_mutex_t& mutex);
   static CameraStatus* getInstance(void);
	 bool getIsOpenCVRunning() { return this->isOpenCVRunning; }
	 void setIsOpenCVRunning(bool isRunning) { this->isOpenCVRunning = isRunning; }
	 bool getIsOpenCVInitialized() { return this->isOpenCVInitialized; }
	 void setIsOpenCVInitialized(bool isInitialized) { this->isOpenCVInitialized = isInitialized; }
	
	 bool getIsRecRunning() { return this->isRecRunning; }
	 void setIsRecRunning(bool isRunning) { this->isRecRunning = isRunning; };
	 bool getIsRecInitialized() { return this->isRecInitialized; }
	 void setIsRecInitialized(bool isInitialized) { this->isRecInitialized 	= isInitialized; }

	private:
	  static CameraStatus* camStats;
		CameraStatus();
		bool isCCDRunning;
		bool isOpenCVRunning;
		bool isOpenCVInitialized;
		bool isRecRunning;
		bool isRecInitialized;
		pthread_mutex_t mutex;

};

#endif   /*_CAM_STATUS_H_*/
