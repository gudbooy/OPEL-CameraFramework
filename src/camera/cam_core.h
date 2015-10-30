#ifndef _CAM_CORE_H_
#define _CAM_CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#include <asm/types.h>

#include <linux/videodev2.h>

#include <libv4l1.h>
#include <libv4l2.h>


#include <sys/ipc.h>
#include <sys/shm.h>
/*User-Defined Header*/
#include "cam_property.h"


#define SHM_KEY 1234

#define V4L2_DEFAULT_WIDTH 640
#define V4L2_DEFAULT_HEIGHT 480


#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define MAX_V4L2_BUFFERS 10

#define OptLast 256


static int xioctl(int fh, int request, void* arg);
static void errno_exit(const char *s);

static void streaming_set_cap(int fd);





const static char* deviceName = "/dev/video0";



typedef struct buffer{
	void *start;
	size_t length;
}buffer;

struct ctl_parameters{
	bool terminate_decoding;
	char options[OptLast];
	char fd_name[80];
	bool filename_active;
	double freq;
	uint32_t wait_limit;
	uint8_t tuner_index;
//	struct v4l2_hw_freq_seek freq_seek;
};

static struct ctl_parameters params;

typedef struct Cam_V4L2
{
	char* deviceName;
	int deviceHandle;
	int bufferIndex; 
	int width, heigth;
	int mode; 
	
	void* shm_buffer;
	
	struct v4l2_capability cap;
	struct v4l2_input inp;
	struct v4l2_format form;
	struct v4l2_crop crop;
	struct v4l2_cropcap cropcap;
	struct v4l2_requestbuffers req;
	struct v4l2_jpegcompression compr;
	struct v4l2_control control;
	enum v4l2_buf_type type;
	struct v4l2_queryctrl queryctrl;

	struct timeval timestamp;
}Cam_V4L2;
/*class CamProperty
{
				public:
					CamProperty();
					CamProperty(int, int, int, enum v4l2_field);
					~CamProperty();
					void setWidth(int width){ this->width = width; }
					void setHeight(int height) {this->height = height; }
					void setPixelformat(int pixelformat) {this->pixelformat = pixelformat;}
					void setFieldformat(enum v4l2_field field) {this->field = field;}
					int getWidth(void) {return this->width;}
					int getHeight(void) {return this->height;}
					int getPixelformat(void) {return this->pixelformat;}
					enum v4l2_field getField(void) {return this->field;}
				private:
					int width;
					int height;
					int pixelformat;
					enum v4l2_field field;
};*/
static bool libv4l2_open(CameraProperty* camProp);
static bool libv4l2_init(CameraProperty* camProp);
static bool init_SharedMemorySpace(int req_count, int buffer_size, int shmid, void* shmptr);
static bool uinit_SharedMemorySpace(int shmid);
static bool mainLoop(CameraProperty* camProp);
//static bool libv4l2_userPointer(unsigned int buffer_size, CameraProperty* camProp, void* buffers);
class OPELCamera
{
				public:
								void setCameraProperty(CameraProperty* camProp);
								CameraProperty* getCameraProperty() const;
								OPELCamera();
								OPELCamera(CameraProperty* camProp);
								bool open();
			          virtual bool init_device() = 0;
								virtual bool start() = 0;
				protected:
								virtual bool init_userPointer(unsigned int) = 0;
								CameraProperty* camProp;			
																		
};


class Record : public OPELCamera
{
				public:
									

				private:
					 char* output_path;
};
class OpenCVSupport : public OPELCamera
{
	public:
		virtual	bool init_device();
	 	virtual bool start();
		//virtual bool init_userPointer(unsigned int);
	  
			
	private:
	 	  virtual bool init_userPointer(unsigned int);
			buffer* buffers;
			int shmid;
			void* shmPtr; 
			struct shmid_ds shm_info;
};
class Camera
{
public: 
	int get_fd(void){ return this->fd; } 	
	bool open_device(void);
	bool uninit_device();
	bool init_device();
	int start_capturing();
	int stop_capturing();
	bool close_device();
	static Camera* getInstance();	
	static Camera* getInstance(char* deviceName);	
	static void* getSharedBufferPtr(void);
//	static int setInstance(CamProperty cp);
	~Camera();	
private:
	/* singleton */
	static Camera* cam;
	
	Camera();
	Camera(char* deviceName);
//	~Camera();
	
	
	int shmid;
	void* shmPtr;
	struct shmid_ds shm_info;
	buffer* buffers;
	unsigned int n_buffers;

	/* File Descriptor of Camera /dev/video(x) */
	int fd;
	bool init_SharedMemoryRegion(int, int);
	bool uninit_SharedMemoryRegion();
	bool init_userp(unsigned int);
	bool read_frame(void);
	bool mainloop();
	// 	char* deviceName ="/dev/video0";

	int deviceHandle;
	int bufferIndex;
	int width, height;
	int pixelformat;
	enum v4l2_field field;
	int mode; 
	unsigned long long count;
	unsigned int n_buffer;	
	
	/* V4L2 API */
	struct v4l2_capability cap;
	struct v4l2_input inp;
	struct v4l2_format fmt;
	struct v4l2_crop crop;
	struct v4l2_cropcap cropcap;
	struct v4l2_requestbuffers req;
	struct v4l2_jpegcompression compr;
	struct v4l2_control control;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;
	struct v4l2_queryctrl queryctrl;
	struct timeval timestamp; 
	void init_mmap();

protected:
//	friend double getInternalProperty(int) const; 
	friend double setInternalProperty(int, double);

};









#endif /* CAM_CORE_H_ */
