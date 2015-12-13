#include "cam_core.h"
#include <stdio.h>
static bool init_SharedMemorySpace(int req_count, int buffer_size, int shmid, void** shmptr);
static bool uinit_SharedMemorySpace(int shmid);
char SEM_NAME[] = "vik";

Camera* Camera::cam = NULL;

bool OpenCVSupport::init_Semaphore()
{
	mx = sem_open(SEM_NAME, O_CREAT, 0666, 1);
	if(mx == SEM_FAILED)
	{

		fprintf(stderr, "[INIT_SEMAPHORE] : FAILED\n");
		sem_unlink(SEM_NAME);
		return false;
	}
	
	/*	union semun sem_union;
	semid = semget(semKey, 1, 0666 | IPC_CREAT);
	if(-1 == semid)
	{
		fprintf(stderr, "[INIT_SEMAPHORE] : FAILED\n");
		return false;
	}
	sem_union.val = 1;
	if(-1 == semctl(semid, 0, SETVAL, sem_union))
	{
		fprintf(stderr, "[init_Semaphore] : Semaphore Initialization Failed\n");
		return false;
	}*/
	return true;
}

bool OpenCVSupport::uinit_Semaphore()
{
	sem_close(mx);
	sem_unlink(SEM_NAME);
	return true;
}


OPELCamera::OPELCamera()
{
				//		this->camProp = CameraProperty::getInstance();
				//	unsigned int* cnt = camProp->getCount();
				//	printf("count : %d\n ", *cnt);
}

void OPELCamera::setCameraProperty(CameraProperty* camProp)
{
				this->camProp = camProp;
}
void OPELCamera::deleteCameraProperty()
{
				delete this->camProp;
}
				
CameraProperty* OPELCamera::getCameraProperty() const
{
				return this->camProp;
}
bool OPELCamera::open()
{
				return libv4l2_open(this->camProp) ? true : false;
}
bool OpenCVSupport::getEos(void)
{
	return this->eos;
}
void OpenCVSupport::setEos(bool eos)
{
	pthread_mutex_lock(&mutex);
	this->eos = eos;
	pthread_mutex_unlock(&mutex);	
}
bool OpenCVSupport::stop()
{
				int fd = this->camProp->getfd();
				enum v4l2_buf_type  type;	
				type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
				{
						fprintf(stderr, "VIDIOC_STREAMOFF");
						return false;
				}
				return true;
}
bool OpenCVSupport::close_device()
{
				unsigned int i;
				int fd = camProp->getfd();
				if(!uinit_SharedMemorySpace(this->shmid, &shmPtr))
								return false;
				if(!uinit_Semaphore())
								return false;
				if(-1 == close(fd))
				{
						fprintf(stderr, "[OpenCVSupport::close_device] : close(fd) failed\n");
						return false;
				}
				free(this->buffers);
				return true;
}
static bool uinit_SharedMemorySpace(int shmid, void** shmPtr)
{
				if(-1 == shmdt(*shmPtr))
				{
								fprintf(stderr, "Detach Shared Memory Space Failed\n");
								return false;
				}

				/*	if(-1 == shmctl(shmid, IPC_RMID, 0))
				{
								fprintf(stderr, "UnInit Shared Memory Space Failed\n");
								return false;
				}*/
				return true;
}
bool OpenCVSupport::init_userPointer(unsigned int buffer_size)
{
				
				unsigned int i; 
				int fd = camProp->getfd();
				struct v4l2_requestbuffers* req = camProp->getRequestbuffers();
				CLEAR(*(req));
				req->count = 4;
				camProp->setN_buffers(req->count);
				req->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				req->memory = V4L2_MEMORY_USERPTR;
				
				if(-1 == xioctl(fd, VIDIOC_REQBUFS, req))
				{
								if(EINVAL == errno)
								{
												fprintf(stderr, "%s does not support " " user pointer i/o\n", deviceName);
												return false;
								}
								else
								{
											  fprintf(stderr, "VIDIOC_REQBUFS");
												return false;
								}
				}

				buffers = (buffer*)calloc(4, sizeof(*buffers)); 
				if(!buffers){
					fprintf(stderr, "Out of Memory\n");
					return false;
				}
				camProp->setBufferSize(buffer_size);
				camProp->setBufferNum(req->count);
				if(!init_SharedMemorySpace(req->count, buffer_size, shmid, &shmPtr, camProp->getShmKey()))
				{
								fprintf(stderr, "Shared Memory Space Initialization Failed\n");
								return false;
				}
				for(i=0; i< req->count; i++)
				{
						//		printf("buffer_size : %d\n", buffer_size);
						//		(*buffers[i].length) = buffer_size;
								/* MINI */
								buffers[i].start = shmPtr+i*buffer_size;
								if(!buffers[i].start)
								{
												fprintf(stderr, "Link to SharedMemorySpace failed\n");
												return false;
								}
				}
				int offset = req->count*buffer_size;
				for(i=0; i< req->count; i++)
				{
					buffers[i].length = (int*)(shmPtr+offset)+(i*sizeof(int));
					if(!buffers[i].length)
					{
						fprintf(stderr, "Link to SharedMemorySpace failed\n"); 
						return false;
					}
					*buffers[i].length = buffer_size;
				}
				if(!init_Semaphore())
					return false;

				return true;
}
static void processImg(void* p, int* size)
{

	unsigned sz; 
	FILE *fp = fopen("/home/pi/camData/cam.mjpg", "ab");
	sz = fwrite((char*)p, sizeof(char), *size, fp);
//	printf("sz : %d, length : %d", sz, *size);
	if(sz != *size)
	{
		fprintf(stderr, "failed!!!!!\n");
	}
	fflush(fp);
	fclose(fp);
}
static bool readFrame(CameraProperty* camProp, buffer* buffers, unsigned& cnt, unsigned &last, struct timeval &tv_last, int semid, void* shmPtr, sem_t* mx)
{
		char ch = '<';
		struct v4l2_buffer* buf = camProp->getBuffer();
		int fd = camProp->getfd();
		int n_buffers = camProp->getN_buffers();
		int size = camProp->getBufferSize();
		int offset = size*(n_buffers-1);
		int offset_size = size*n_buffers;
		unsigned int i;
		unsigned int* count = camProp->getCount();
		void* ptr = shmPtr+offset;
		void* _check =(void*)shmPtr+offset_size+sizeof(int);
		void* _length = (void*)shmPtr+offset_size;
		int* length  = (int*)_length;
		int* check = (int*)_check;
		CLEAR(*buf);
		buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf->memory = V4L2_MEMORY_USERPTR;
		if(-1 == xioctl(fd, VIDIOC_DQBUF, buf))
		{
						switch(errno){
							case EAGAIN:
											return false;
											break;
							case EIO: 
											/*Ignore*/
							default: 
											fprintf(stderr, "VIDIOC_DQBUF\n");
											return false;
						}
		}
		for(i=0; i <n_buffers; ++i)
					if(buf->m.userptr == (unsigned long)buffers[i].start && buf->length >= (*buffers[i].length))
						break;
			
				assert(i < n_buffers); 
				
				sem_wait(mx);
					*check = 0;
					printf("check : %d\n", *check);				
				sem_post(mx);

				//semop(semid, &status_wait, 1);
				//*buffers[0].length = buf->bytesused;
					sem_wait(mx);
					*length = buf->bytesused;
					printf("length : %d\n", *length);
//					memset((char*)ptr, 0x0, size);
					memcpy((char*)ptr, (char*)buf->m.userptr, *length);		
					*check = 1;
					printf("check : %d\n", (int)(*check));
					sem_post(mx);			
	
//				processImg((void*)ptr, length);

				if(-1 == xioctl(fd, VIDIOC_QBUF, buf))
				{
								fprintf(stderr, "VIDIOC_QBUF\n"); 
								return false;

				}
		//		printf("palying\n");
	
//		fprintf(stderr, "%c", ch);
//		fflush(stderr);

		if(cnt == 0)
		{
						gettimeofday(&tv_last, NULL);
		}else
		{
						struct timeval tv_cur, res;
						gettimeofday(&tv_cur, NULL);
						timersub(&tv_cur, &tv_last, &res);
						if(res.tv_sec){
								unsigned fps = (100*(cnt - last)) / (res.tv_sec * 100 + res.tv_usec / 10000);
								last = cnt; 
								tv_last = tv_cur;
		//						fprintf(stderr, " %d fps\n", fps);
						}
		}
		cnt++;

		return true;
}
bool OpenCVSupport::mainLoop(CameraProperty* camProp, buffer* buffers)
{
			pthread_mutex_lock(&mutex);
		  bool volatile_eos = this->eos;
			pthread_mutex_unlock(&mutex);
			unsigned last = 0;
			struct timeval tv_last;
			unsigned int* count = camProp->getCount();
			int fd = camProp->getfd();
//			printf("Request Count is : %d", *count);
			unsigned int cnt=0;
			while(((*count)-- > 0) && volatile_eos)
			{
	//						printf("count : %d\n", (*count));
							for(;;)
							{
											fd_set fds;
											struct timeval tv;
											int r;
											FD_ZERO(&fds);
											FD_SET(fd, &fds);
											tv.tv_sec = 2;
											tv.tv_usec = 0;
											r = select(fd+1, &fds, NULL, NULL, &tv);
											if(-1 == r){
															if(EINTR == errno)
																			continue;
															fprintf(stderr, "Select Error\n");
															return false;
											}
											if(0 == r){
															fprintf(stderr, "Select Timeout\n");
															return false;
											}
											if(FD_ISSET(fd, &fds))
											{
														
											}

											if(readFrame(camProp, buffers, cnt, last, tv_last, this->semid, this->shmPtr, this->mx))
														break;	
							}
							pthread_mutex_lock(&mutex);
							volatile_eos = this->eos;
					  	pthread_mutex_unlock(&mutex);

					//		pthread_mutex_lock(&mutex);
					//		if(eos)

			}
		  
			return true;
}
bool OpenCVSupport::start()
{
				unsigned int i;
				int fd = camProp->getfd();
				unsigned int n_buffer = camProp->getN_buffers();
				struct v4l2_buffer* buf = camProp->getBuffer();
				enum v4l2_buf_type type = camProp->getType();	
				for(i=0; i< n_buffer; i++)
				{
								CLEAR(*buf);
								buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
								buf->memory = V4L2_MEMORY_USERPTR;
								buf->index = i;
								buf->m.userptr = (unsigned long)buffers[i].start;
								buf->length = *buffers[i].length;
//								sem_wait(this->mx);										
						*buffers[1].length = 0;
	//							sem_post(this->mx);
								if(-1 == xioctl(fd, VIDIOC_QBUF, buf))
								{
												fprintf(stderr, "VIDIOC_QBUF\n");
												return false;
								}
				}
				
				type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(-1 == xioctl(fd, VIDIOC_STREAMON, &type))
				{
								fprintf(stderr, "VIDIOC_STREAMON\n");
								return false;
				}
			  if(!mainLoop(this->camProp, this->buffers))
				{
								fprintf(stderr, "Main_Loop\n");
								return false;
				}

				return true;
}
bool OpenCVSupport::init_device()
{
				unsigned int min;
				int fd = camProp->getfd();
				struct v4l2_capability* cap = camProp->getCapability();
				struct v4l2_cropcap* cropcap = camProp->getCropcap();
				struct v4l2_crop* crop = camProp->getCrop();
				struct v4l2_format* fmt = camProp->getFormat();
				if(-1 == xioctl(fd, VIDIOC_QUERYCAP, cap))
				{
								if(EINVAL == errno)
								{
												fprintf(stderr, "%s is no V4L2 device \n", deviceName);
												return false;
								}
								else
								{
												errno_exit("VIDIOC_QUERYCAP");
												return false;
								}
				}
				if(!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE))
				{
								fprintf(stderr, "%s is no video capture device\n", deviceName);
								return false;
				}
				if(!(cap->capabilities & V4L2_CAP_STREAMING))
				{
								fprintf(stderr, "%s is no video capture device\n", deviceName);
								return false;
				}
				CLEAR(*cropcap);
				cropcap->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(0 == xioctl(fd, VIDIOC_CROPCAP, cropcap))
				{
								crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
								crop->c = cropcap->defrect;
								if(-1 == xioctl(fd, VIDIOC_S_CROP, crop))
								{
												switch(errno)
												{
																case EINVAL:
																				fprintf(stderr, "Cropping Not Supported\n");
																				break;
																default:
																				fprintf(stderr, "Errors Ignored\n");
																				break;
												}
								}
				}
				else
				{
								/* Do Nothing */
				}
				CLEAR(*fmt);
				fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				fmt->fmt.pix.width = camProp->getWidth();
				fmt->fmt.pix.height = camProp->getHeight();
				fmt->fmt.pix.pixelformat = camProp->getPixelformat();
				fmt->fmt.pix.field = camProp->getField();
				if(-1 == xioctl(fd, VIDIOC_S_FMT, fmt))
				{
								fprintf(stderr, "VIDIOC_S_FMT Error\n");
								return false;
				}

				min = fmt->fmt.pix.width * 2;
				if(fmt->fmt.pix.bytesperline < min)
					fmt->fmt.pix.bytesperline = min;
				min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
				if(fmt->fmt.pix.sizeimage < min)
					fmt->fmt.pix.sizeimage = min;

				if(!init_userPointer(fmt->fmt.pix.sizeimage))
				{
								fprintf(stderr, "initialize user pointer failed\n");
								return false;
				}
				return true;

				//	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
}
static bool libv4l2_open(CameraProperty* camProp)
{
				int fd;
				struct stat st; 
				if(-1 == stat(deviceName,&st))
				{
						fprintf(stderr, "Cannot identify '%s' : %d, %s\n", deviceName, errno, strerror(errno));
						return false;
				}
				if(!S_ISCHR(st.st_mode))
				{
						fprintf(stderr, "%s is no device\n", deviceName);
						return false;
				}
				fd = open(deviceName, O_RDWR | O_NONBLOCK, 0);
				if(-1 == fd)
				{
								fprintf(stderr, "Cannot open '%s' : %d, %s\n", deviceName, errno, strerror(errno));
								return false;
				}
	//			camProp->setStat(st);
				camProp->setfd(fd);
				return true;
}
static bool libv4l2_init(CameraProperty* camProp)
{
				return true;
}



static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
	int r;
	do {
		r = ioctl(fh, request, arg);
	}while(-1 == r && EINTR == errno);
}
Camera* Camera::getInstance(char* deviceName)
{
	if(cam == NULL)
		cam = new Camera(deviceName);
	return cam;
}
/* instance */
Camera* Camera::getInstance()
{
	if(cam == NULL)
		cam = new Camera();
	return cam;
}
/* constructor */
Camera::Camera(char* deviceName)
{
//				this->deviceName = deviceName;
				this->width = 640;
				this->height = 480;
				this->pixelformat = V4L2_PIX_FMT_YUYV;
				this->field = V4L2_FIELD_INTERLACED;
				this->count = 10000;
// Do implement
}
Camera::Camera()
{
				this->width = 640;
				this->height = 480;
				this->pixelformat = V4L2_PIX_FMT_YUYV;
				this->field = V4L2_FIELD_INTERLACED;
				this->count = 10000;
}
/* destructor */
Camera::~Camera()
{
				if(!shmPtr)
				{
					if(!this->uninit_SharedMemoryRegion())
					{
								fprintf(stderr, "uninit shared memory space error\n");
								exit(1);
					}
				}				
				if(!buffers)
				{
								free(buffers);
				}
}
bool Camera::open_device(void)
{
//	cam->deviceName = deviceName;
	struct stat st; 
	if(-1 == stat(deviceName, &st))
	{
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", deviceName, errno, strerror(errno));
		return false;
	}	
	if(!S_ISCHR(st.st_mode))
	
	{
		fprintf(stderr, "%s is no device\n", deviceName);
		return false;
	}
	fd = open(deviceName, O_RDWR | O_NONBLOCK, 0); 
	if(-1 == fd)
	{
		fprintf(stderr, "Cannot open '%s': %d, %s\n", deviceName, errno, strerror(errno));
		return false;
	}
	return true;
}
bool Camera::init_SharedMemoryRegion(int req_count, int buffer_size)
{
				return false;
}

static bool init_SharedMemorySpace(int req_count, int buffer_size, int shmid, void** shmPtr, key_t shmkey)
{
				printf("buffer_len = %d, buffer_num = %d\n", buffer_size, req_count);
				
				shmid = shmget((key_t)shmkey, (buffer_size*req_count)+(sizeof(int)*req_count), 0666|IPC_CREAT);
				printf("shmkey : %d\n", shmkey);
				if(shmid == -1)
				{
								perror("shmget failed : ");
								return false;
				}
				*shmPtr = shmat(shmid, NULL, 0);
				
				if(*shmPtr == (void*)-1)
				{
								perror("shmget failed : ");
								return false;
				}
				printf("Shared Memory Region Initialization Success\n");
				return true;
}

bool Camera::uninit_SharedMemoryRegion()
{
			return false;
}
static bool uninit_SharedMemorySpace(int shmid)
{
				if(-1 == shmctl(shmid, IPC_RMID, 0))
				{
								fprintf(stderr, "Failed to remove shared data region\n");
								return false;
				}
				return true;
}
bool Camera::init_userp(unsigned int buffer_size)
{
				unsigned int i;
				CLEAR(req);
				req.count = 4;
				
				n_buffers = req.count;
				req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				req.memory = V4L2_MEMORY_USERPTR;
				

				if(-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
				{
								if(EINVAL == errno)
								{
												fprintf(stderr, "%s does not support " " user pointer i/o\n", deviceName);
												return false;
								}else
								{
												printf("VIDIOC_REQBUFS ERROR");
												return false;
								}
				}	
			//	buffers = (buffer*)calloc(4, sizeof(*buffers));
				if(!buffers){
						fprintf(stderr, "Out of Memroy\n");
						return false;
				}
				if(!init_SharedMemoryRegion(req.count, buffer_size))
				{
								printf("Shared Memory Region Init Failed\n");
								return false;
				}
				for(i = 0; i < n_buffers; ++i)
				{
								//buffers[i].length = buffer_size;
							//	buffers[i].start = shmPtr+i*buffer_size;
								
				}
				int offset = buffer_size*n_buffers;
				for(i=0; i< n_buffers; ++i)
				{
						
				}
		return true;
}
bool Camera::init_device()
{
	unsigned int min;
	if(-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))	
	{
		if(EINVAL == errno)
		{
			fprintf(stderr, "%s is no V4L2 device \n", deviceName);
			return false;
		//	exit(EXIT_FAILURE);
		}
		else
		{
			errno_exit("VIDIOC_QUERYCAP");
		}
	}	
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf(stderr, "%s is no video capture device\n", deviceName);
		return false;
		//	exit(EXIT_FAILURE);
	}

	if(!(cap.capabilities & V4L2_CAP_STREAMING))
	{
					fprintf(stderr, "%s does not support streaming i/o\n", deviceName);
					return false;
	}
	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;
		
		if(-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch(errno)
			{
				case EINVAL:
						fprintf(stderr, "Cropping Not Supported\n");
								break;
				default:
						fprintf(stderr, "Errors Ignored.\n");
								break;
						}
		 		}
	} else {
					/* Errors Ingonre. */
	}

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/* Setting Default Value */
	fmt.fmt.pix.width = this->width;
	fmt.fmt.pix.height = this->height;
	fmt.fmt.pix.pixelformat = this->pixelformat; 
	fmt.fmt.pix.field = this->field;
	if(-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
	{
					fprintf(stderr, "VIDIOC_S_FMT ERROR\n");
					return false;
	}

	min = fmt.fmt.pix.width * 2;
	if(fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if(fmt.fmt.pix.sizeimage < min)
			fmt.fmt.pix.sizeimage = min;

	if(!init_userp(fmt.fmt.pix.sizeimage))
	{
					fprintf(stderr, "initialize user pointer failed\n");
					return false;
	}

	printf("initialize device success\n");
				
	return true;
}
void process_image(const void *p, int* size)
{
		//		fwrite(p, size, 1, stdout);
		//		fflush(stderr);
		//		fprintf(stderr, "."); 
		//		fflush(stdout);
}
bool Camera::read_frame(void)
{
		struct v4l2_buffer buf;
	 	unsigned int i;
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		if(-1 ==xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch(errno) {
							case EAGAIN:
											return 0;
							case EIO:
											/* Could ignor EIO */
							default:
											fprintf(stderr, "VIDIOC_DQBUF\n");
											return false;
							
			}
		}
		for(i=0; i<n_buffers; ++i)
		{
					//	if(buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
										break;
						assert(i < n_buffers);
						//process_image(buffers[0].start, buffers[0].length);

										if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
										{
														fprintf(stderr, "VIDIOC_QBUF");
														return false;									
										}
					break;
		}

		return true;
} 
bool Camera::mainloop()
{
		while(count-- > 0)
		{
						for(;;){
							fd_set fds;
							struct timeval tv; 
							int r;
							FD_ZERO(&fds);
							FD_SET(fd, &fds);
							tv.tv_sec = 2;
							tv.tv_usec = 0;
							r = select(fd+1, &fds, NULL, NULL, &tv);
							if(-1 == r){
									if(EINTR == errno)
													continue;
									fprintf(stderr, "select error");
									return false;
							}
							if(0 == r){
								fprintf(stderr, "select timeout\n");
								return false;
							}
							if(read_frame())
											break;
						}
		}
		return true;

}
int Camera::start_capturing()
{
	unsigned int i;
	for(i=0; i<n_buffers; ++i)
	{
					CLEAR(buf);
					buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					buf.memory = V4L2_MEMORY_USERPTR;
					buf.index = i;
					buf.m.userptr = (unsigned long)buffers[i].start;
					//buf.length = buffers[i].length;
					if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
					{
									fprintf(stderr, "VIDIOC_QBUF\n");
									return false;
					}
	}
	
	type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == xioctl(fd, VIDIOC_STREAMON, &type))
	{				fprintf(stderr, "VIDIOC_STREAMON\n");
					return false;
	}
	if(!mainloop())
	{
					fprintf(stderr, "MAIN LOOP Failed\n");
					return false;
	}
	return true;
}

int Camera::stop_capturing()
{
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
	{
					fprintf(stderr, "VIDIOC_STREAMOFF");
					return 0;
	}	
	return 1;
}
bool Camera::uninit_device()
{
				unsigned int i;
				if(!uninit_SharedMemoryRegion())
								return false;
				free(buffers);
				return true;
}

bool Camera::close_device()
{
				if(-1 == close(fd)){
							fprintf(stderr, "Close failed\n");
							return false;
				}
				fd = -1;
	return true;
}


