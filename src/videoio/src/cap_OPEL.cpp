/*OPEL Camera Library */
#include "precomp.hpp"


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <asm/types.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
//#include <semaphore.h>
#include <dbus/dbus.h>

#include <linux/videodev2.h>

#include <libv4l1.h>
#include <libv4l2.h> 

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define SEM_KEY_FOR_PROPERTY 49411
#define SEM_KEY_FOR_STATUS 49441
#define SHM_KEY_FOR_STATUS 4944
#define SHM_KEY_FOR_PROPERTY 4941
#define SHM_KEY_FOR_BUFFER 5315
#define OPENCV_DEFAULT_WIDTH 640
#define OPENCV_DEFAULT_HEIGHT 480

#define OPENCV_DEFAULT_BUF_SIZE 921600
#define OPENCV_DEFAULT_BUF_INDEX 4

const char* dev_name = "/dev/video0";
static struct sembuf mutex_post = {0, -1, SEM_UNDO};
static struct sembuf mutex_wait = {0, 1, SEM_UNDO};

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int* array;
};
typedef struct status
{
	bool isCCDRunning;
	bool isOpenCVRunning;
	bool isOpenCVInitialized;
	bool isRecRunning;
	bool isRecInitialized;
}status;

typedef struct property
{
	int width;
	int height;
	int buffer_size;
	int buffer_num;
	int n_buffer;
	bool isPropertyChanged;
	bool allowRunning;
}property;

static void sendDbusMsg(DBusConnection* conn, const char* msg)
{
	DBusMessage* message;
	message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", msg);
	dbus_connection_send(conn, message, NULL);
	dbus_message_unref(message);
}
class CvCaptureCAM_OPEL_CPP : CvCapture
{
				public:
					CvCaptureCAM_OPEL_CPP();
					virtual ~CvCaptureCAM_OPEL_CPP();
					
					virtual bool open( int index );
					virtual bool close();
					virtual double getProperty(int) const;
					virtual bool setProperty(int, double);
					virtual bool grabFrame();
					virtual IplImage* retrieveFrame(int);
					
				protected: 
					IplImage frame;
					int fd;
					int shmid;
					void* shmPtr;
					int shmid_for_property;
					void* shmPtr_for_property;
					int shmid_for_status;
					void* shmPtr_for_status;
					unsigned buffer_size;
					unsigned buffer_index;
					bool init_SharedMemorySpace(void);
					bool init_SharedMemorySpaceForProperty(void);
					bool uinit_SharedMemorySpaceForProperty(void);
					bool uinit_SharedMemorySpace(void);
				  bool init_Semaphore(void);
					bool init_DBus(void);
					bool readFrame(void);
					int width, height;
					int mode;
					property* prop;
					status* st;
					void mainLoop(void);
					int semid;
				
				
					DBusConnection* connection;
					DBusError error;
					
};
bool CvCaptureCAM_OPEL_CPP::init_Semaphore(void)
{

	union semun sem_union;
	semid = semget((key_t)SEM_KEY_FOR_STATUS, 0, 0666 | IPC_CREAT);
	if(-1 == semid)
	{
		fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::init_Semaphore] : Semaphore Initialization Error\n");
		return false;
	}
	sem_union.val = 1;
	if(-1 == semctl(semid, 0, SETVAL, sem_union))
	{
		fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::init_Semaphore] : Semaphore Semctl Error\n");
		return false;
	}
	return true;
}
bool CvCaptureCAM_OPEL_CPP::init_DBus(void)
{
	dbus_error_init(&error);
	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if(!connection)
	{
		fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::init_DBus] : dbus_bus_get Error\n"); 
		dbus_error_free(&error);
		return false;
	}
	return true;
}
static inline int channels_for_mode(int mode)
{
				switch(mode){
				case CV_CAP_MODE_GRAY:
								return 1;
				case CV_CAP_MODE_YUYV:
								return 2;
				default:
								return 3;
				}
}
bool CvCaptureCAM_OPEL_CPP::readFrame(void)
{
				/* set timestamp in capture strcut to be timestamp of most recent frame */
			//	printf("ReadFrame Call\n");
				return true;

}
void CvCaptureCAM_OPEL_CPP::mainLoop(void)
{
				unsigned int count;
				count = 1;
				while(count-- > 0)
				{
								for(;;){
												fd_set fds;
												struct timeval tv;
												int r;
												FD_ZERO(&fds);
												FD_SET(fd, &fds);
												/*Time Out Value*/
												tv.tv_sec = 10; 
												tv.tv_usec = 0; 
												r = select(fd+1, &fds, NULL, NULL, &tv);
												if(-1 == r)
												{
																if(EINTR == errno)
																		continue;
																fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::mainLoop] : Select Failed\n");
												}
												if(0 == r)
												{
																fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::mainLoop] : select timeout\n");
																break;
												}
												
												if(readFrame())
																break;

								}

				}
}
CvCaptureCAM_OPEL_CPP::CvCaptureCAM_OPEL_CPP()
{
				this->width = OPENCV_DEFAULT_WIDTH;
				this->height = OPENCV_DEFAULT_HEIGHT;
				this->buffer_size = OPENCV_DEFAULT_BUF_SIZE;
				this->buffer_index = OPENCV_DEFAULT_BUF_INDEX;
				this->mode = CV_CAP_MODE_RGB;
}

CvCaptureCAM_OPEL_CPP::~CvCaptureCAM_OPEL_CPP()
{
	//sem_close(this->mutex);
	//sem_unlink("openCVProp");
}

bool CvCaptureCAM_OPEL_CPP::init_SharedMemorySpaceForProperty()
{
/*	shmid_for_property = shmget((key_t)SHM_KEY_FOR_PROPERTY, 0, 0);
	if(shmid_for_property == -1)
	{
		fprintf(stderr,"[CvCaptureCAM_OPEL_CPP::init_SharedMemorySpaceForProperty] : Shmget Error\n");
		return false;
	}
 */
	shmid_for_status = shmget((key_t)SHM_KEY_FOR_STATUS, 0, 0);
	if(shmid_for_status == -1)
	{
		fprintf(stderr,"[CvCaptureCAM_OPEL_CPP::init_SharedMemorySpaceForProperty] : Shmget Error\n");
		return false;
	}
/*	
	shmPtr_for_property = shmat(shmid_for_property, (void*)0, 0666|IPC_CREAT);
	if(shmPtr_for_property == (void*)-1)
	{
	  fprintf(stderr,"[CvCaptureCAM_OPEL_CPP::init_SharedMemorySpaceForProperty] : Shmat Error\n");
		return false;
	}
 */
	shmPtr_for_status = shmat(shmid_for_status, (void*)0, 0666|IPC_CREAT);
	if(shmPtr_for_status == (void*)-1)
	{
	  fprintf(stderr,"[CvCaptureCAM_OPEL_CPP::init_SharedMemorySpaceForProperty] : Shmat Error\n");
		return false;
	}
	prop = (property*)shmPtr_for_property;

	return true;
}

bool CvCaptureCAM_OPEL_CPP::init_SharedMemorySpace()
{
				shmid = shmget((key_t)SHM_KEY_FOR_BUFFER, 0, 0);
				if(shmid == -1)
				{
								fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::init_SharedMemorySpace] : Shmget Error\n");
								return false;
				}
				shmPtr = shmat(shmid, (void*)0, 0666|IPC_CREAT);
				if(shmPtr == (void*)-1)
				{
							fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::init_SharedMemorySpace] : Shmat Error\n");
							return false;
				}
			//	fprintf(stderr, "Shared Memory Space Init\n");
				return true;
			
}
bool CvCaptureCAM_OPEL_CPP::uinit_SharedMemorySpace()
{
			
				return true;
}
static void open_device(int* fd)
{
				*(fd) = open(dev_name, O_RDWR | O_NONBLOCK, 0);
				if(-1 == (*fd))
				{
								fprintf(stderr, "[OPEN_DEVICE] : /dev/video0 Cannot Open\n");
				}
			
}
bool CvCaptureCAM_OPEL_CPP::open(int index)
{	
				bool allowIsRunning;
				open_device(&fd);
				
				if(this->fd == -1)
						return false;
  			
				if(!(this->init_SharedMemorySpaceForProperty()))
						return false;
       	
				if(!init_DBus())
						return false;
				
				if(!init_Semaphore())
						return false;

				printf("******* CAMERA PROPERTY ********\n");
				printf("width = %d\n", this->width);
				printf("height = %d\n", this->height);
				printf("buffer_size = %d\n", this->buffer_size);
				fflush(stdout);
						


				sendDbusMsg(this->connection, "OpenCVInit");
				/*
				semop(semid, &mutex_wait, 1);
				allowIsRunning = prop->allowRunning;
				semop(semid, &mutex_post, 1);
				
				fprintf(stderr, "START\n");
				
				while(!allowIsRunning)
				{
					semop(semid, &mutex_wait, 1);
					printf("polling\n");
					allowIsRunning = prop->allowRunning;
					semop(semid, &mutex_post, 1);
				}
				*/
		/*		semop(semid, &mutex_wait, 1);
		  	this->width = prop->width;
				this->height = prop->height;
				this->buffer_size = prop->buffer_size;
				this->buffer_index = prop->buffer_num;
				this->mode = CV_CAP_MODE_RGB;

				printf("******* CAMERA PROPERTY ********\n");
				printf("width = %d\n", prop->width);
				printf("height = %d\n", prop->height);
				printf("buffer_size = %d\n", prop->buffer_size);
				fflush(stdout);
				semop(semid, &mutex_post, 1);*/
		
//				sendDbusMsg(this->connection, "OpenCVInit");
				//check if CCD is already running ? and check whether recording is running or openCV running
				//
				//if so, read property and start Getting Image from CCD, if not read property then send openCVInit and openCVSend Signal via DBus to CCD 
				//
				

				cvInitImageHeader(&frame, cvSize(this->width, this->height), IPL_DEPTH_8U, channels_for_mode(mode), IPL_ORIGIN_TL, 4);
				frame.imageData = (char *)cvAlloc(frame.imageSize); 
				if(!frame.imageData)
				{
					fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::open] : cvAlloc Error\n");
					return false;
				}
				if(!init_SharedMemorySpace())
								return false;
				
				sendDbusMsg(this->connection, "OpenCVStart");

			  return true;
}
bool CvCaptureCAM_OPEL_CPP::close()
{
	// check if already running
	
	sendDbusMsg(this->connection, "OpenCVStop");			
	return true;			
}
/* get property Not Supported in OPEL OpenCV lib */
double CvCaptureCAM_OPEL_CPP::getProperty(int) const
{

				return false;
}
/* set Property Not Supported in OPEL OpenCV lib */
bool CvCaptureCAM_OPEL_CPP::setProperty(int, double)
{
				fprintf(stderr, "[CvCaptureCAM_OPEL_CPP::setProperty] : Cannot configure the setProperty\n");
				return false;
}
bool CvCaptureCAM_OPEL_CPP::grabFrame()
{
			//	fprintf(stderr, "GrabFrame\n");	
				mainLoop();				
				return true;
}
IplImage* CvCaptureCAM_OPEL_CPP::retrieveFrame(int)
{
		  	unsigned offset;
				offset = (buffer_index-1)*buffer_size;
			//	printf("buffer_size : %d\n", buffer_size);
		  //	printf("frame_image_size : %d\n", frame.imageSize);
				if(shmPtr)
				{
								memcpy((char*)frame.imageData, (char*)shmPtr+offset, buffer_size);
				}
				return &frame;
}
CvCapture* cvCreateCameraCapture_V4L(int index)
{
				CvCaptureCAM_OPEL_CPP* capture = new CvCaptureCAM_OPEL_CPP;
				if( capture->open(index))
							return (CvCapture*)capture;
				
				delete capture;
				return 0;
}














