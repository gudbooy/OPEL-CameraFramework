#ifndef _CAM_STATUS_H_
#define _CAM_STATUS_H_
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
class CameraStatus
{
	public:
//		~CameraStatus();
   CameraStatus* getInstance(void);
	 bool getIsCCDRunning() {
	 
	 }
	




	private:
	  static CameraStatus* camStats;
		CameraStatus();
		bool isCCDRunning;
		bool isOpenCVRunning;
		bool isRecRunning;

};

#endif   /*_CAM_STATUS_H_*/
