#ifndef _CAM_PROPERTY_H_
#define _CAM_PROPERTY_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <linux/videodev2.h>
#include <libv4l1.h>
#include <libv4l2.h>
//#include "cam_core.h"

#define DEFAULT_COUNT 100000
#define OPENCV_DEFAULT_PIXFORMAT 2 //RGB24
#define REC_DEFAULT_PIXFORMAT 4 // h.264
#define DEFAULT_WIDTH 640 
#define DEFAULT_HEIGHT 480


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
								struct v4l2_buffer* getBuffer(void) {return this->buf; }
								enum v4l2_buf_type getType(void) {return this->type;}
								int getWidth() { return this->width; }
								int getHeight() { return this->height; }
								unsigned int getPixelformat() { return this->pixelformat; }
								enum v4l2_field getField() { return this->field; }
							  unsigned int* getCount() { return this->count; }
								void setCount(unsigned long long count) { *(this->count) = count; }
								void printSetValue(void);
								//	struct stat getStat() { return this->st; }
							  //void setStat(struct stat st) { this->st = st; }
								
				private:
								int fd;
								static CameraProperty* camProp;
								CameraProperty();
								int deviceHandle;
								int bufferIndex;
								int width, height;
								unsigned int pixelformat;
								enum v4l2_field field;
								int mode;
								unsigned int* count;
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
