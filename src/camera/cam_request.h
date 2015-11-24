#ifndef _CAM_REQUEST_H_
#define _CAM_REQUEST_H_
#include <stdio.h>
#include <unistd.h>



enum kinds {openCV=0, REC, setProperty};
enum operations {init=0, start, stop, close};

typedef struct CameraRequest
{
 	pid_t pid;
//char* name;
	enum kinds kind;
	enum operations operation;
	int width;
	int height;

}CameraRequest;

#endif /*_CAM_REQUEST_H_*/
