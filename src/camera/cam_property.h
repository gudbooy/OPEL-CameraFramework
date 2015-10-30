#ifndef _CAM_PROPERTY_H_
#define _CAM_PROPERTY_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <linux/videodev2.h>
#include <libv4l1.h>
#include <libv4l2.h>
//#include "cam_core.h"

class CameraProperty
{
				public: 
								static CameraProperty* getInstance();
								~CameraProperty();
								void setfd(int fd) { this->fd = fd; }		
							  int getfd(void) { return this->fd; } 
								struct v4l2_requestbuffers* getRequestbuffers(void) { return this->req; } 	
								void setN_buffers(unsigned int n_buffer) {this->n_buffer = n_buffer; }
						    unsigned int getN_buffers(void) {return this->n_buffer; }	
								struct v4l2_capability* getCapability(void) { return this->cap; }
								struct v4l2_cropcap* getCropcap(void) { return this->cropcap; } 
								struct v4l2_crop* getCrop(void) { return this->crop; }
								struct v4l2_format* getFormat(void) {return this->fmt; }
								int getWidth() { return this->width; }
								int getHeight() { return this->height; }
								int getPixelformat() { return this->pixelformat; }
								enum v4l2_field getField() { return this->field; }
								//	struct stat getStat() { return this->st; }
							  //void setStat(struct stat st) { this->st = st; }
								
				private:
								int fd;
								static CameraProperty* camProp;
								CameraProperty();
								int deviceHandle;
								int bufferIndex;
								int width, height;
								int pixelformat;
								enum v4l2_field field;
								int mode;
								unsigned long long count;
								unsigned int n_buffer;

								struct v4l2_capability* cap;
								struct v4l2_input* inp;
								struct v4l2_format* fmt;
								struct v4l2_crop* crop;
								struct v4l2_cropcap* cropcap;
								struct v4l2_requestbuffers* req;
								struct v4l2_jpegcompression* compr;
								struct v4l2_control* control;
								struct v4l2_buffer* buf;
								enum v4l2_buf_type type;
								struct v4l2_queryctrl* queryctrl;
								struct timeval* timestamp;
//								struct stat* st;
};


#endif /*_CAM_PROPERTY_H_*/
