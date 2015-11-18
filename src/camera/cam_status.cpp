#include "cam_status.h"
CameraStatus* CameraStatus::camStats = NULL;

CameraStatus::CameraStatus()
{




}
CameraStatus* CameraStatus::getInstance(void)
{
	if(this->camStats == (void*)0)
		this->camStats = new CameraStatus();
	return this->camStats;
}

