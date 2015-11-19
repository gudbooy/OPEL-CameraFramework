#include "cam_property.h"

//CameraProperty* CameraProperty::camProp = NULL;

//CameraProperty* CameraProperty::getInstance()
//{
//				if(this->camProp == (void*)0)
//								camProp = new CameraProperty();
//				return camProp;
//					return this;
//}
void CameraProperty::printSetValue(void)
{
				std::cout << "***** OPEL CAMERA PROPERTIES *****" <<std::endl;
				std::cout << "Width : " << this->width << " Height : " << this->height << std::endl;
				std::cout << "Pixelformat : " << this->pixelformat << std::endl;
				std::cout << "Number of Frame : " << *(count) << std::endl; 
				std::cout << "Field : " << this->field << std::endl;
				
}
void CameraProperty::initProperty(property** prop)
{
	//semaphore lock
  sem_wait(mutex);
	(*prop)->width = this->width;
	(*prop)->height = this->height;
	(*prop)->buffer_size = this->buffer_size;
	(*prop)->buffer_num = this->buffer_num;
	(*prop)->n_buffer = this->n_buffer;
  (*prop)->isPropertyChanged = true;
	sem_post(mutex);
	//semaphore unlock
}
bool CameraProperty::InitSharedPropertyToTarget(property* prop)
{
	//if already shared
	if(!(this->shmPtr_for_property)){
		this->shmid_for_property = shmget(shmkey_for_property, sizeof(property), 0666|IPC_CREAT);
		if(this->shmid_for_property == -1)
		{
			fprintf(stderr,"[InitSharedPropertyToTarget] : Shmget Error\n");
			return false;			
		}
		shmPtr_for_property = (void*)shmat(this->shmid_for_property, NULL, 0);
		if(shmPtr_for_property == (void*)-1)
		{
			fprintf(stderr, "[InitSharedPropertyToTarget] : Shmat Error\n");
			return false;
		}
		shmPtr_for_property = (void*)prop;
	}
	else
	{
		return false;
	}
	
}
bool CameraProperty::uInitSharedPropertyToTarget(property* prop)
{
  // uninit
	return true;
}
void CameraProperty::setBufferSize(int buffer_size)
{
	//semaphore lock
	sem_wait(mutex);
	this->buffer_size = buffer_size; 
	sem_post(mutex);
	//semaphore unlock
}
void CameraProperty::setBufferNum(int buffer_num)
{
	//semaphore lock
	sem_wait(mutex);
	this->buffer_num = buffer_num;
	sem_post(mutex);
	//semaphore unlock
}
CameraProperty::CameraProperty(bool isRec)
{
	if(!isRec)
	{
				fprintf(stderr, "Called Construct for OpenCV Support\n");
  			this->count = (unsigned int*)malloc(sizeof(unsigned int));
				this->cap = (struct v4l2_capability*)malloc(sizeof(struct v4l2_capability));
				this->inp = (struct v4l2_input*)malloc(sizeof(struct v4l2_input));
				this->fmt = (struct v4l2_format*)malloc(sizeof(struct v4l2_format));
				this->crop = (struct v4l2_crop*)malloc(sizeof(struct v4l2_crop));
				this->cropcap = (struct v4l2_cropcap*)malloc(sizeof(struct v4l2_cropcap));
		    this->req = (struct v4l2_requestbuffers*)malloc(sizeof(struct v4l2_requestbuffers)); 
				this->compr = (struct v4l2_jpegcompression*)malloc(sizeof(struct v4l2_jpegcompression));
				this->control = (struct v4l2_control*)malloc(sizeof(struct v4l2_control));
				this->buf = (struct v4l2_buffer*)malloc(sizeof(struct v4l2_buffer));
				this->queryctrl = (struct v4l2_queryctrl*)malloc(sizeof(struct v4l2_queryctrl));
				this->timestamp = (struct timeval*)malloc(sizeof(struct timeval));
				*(this->count) = DEFAULT_COUNT;
				this->field = V4L2_FIELD_ANY;
				this->width = DEFAULT_WIDTH;
				this->height = DEFAULT_HEIGHT;
				this->pixelformat = V4L2_PIX_FMT_RGB24;
				this->shmkey = OPENCV_SHM_KEY;
				this->shmkey_for_property = OPENCV_SHM_KEY_FOR_PROPERTY;
				this->shmPtr_for_property = NULL;
				this->mutex = sem_open("openCVProp", O_CREAT, 0666, 1);
				if(this->mutex == SEM_FAILED)
				{
					fprintf(stderr, "[CameraProperty::CameraProperty] : Sem_open Error\n");
					sem_unlink("openCVProp");
					this->mutex = NULL;
				}

	}
	else
	{
				fprintf(stderr, "Called Construct for Recording\n");
  			this->count = (unsigned int*)malloc(sizeof(unsigned int));
				this->cap = (struct v4l2_capability*)malloc(sizeof(struct v4l2_capability));
				this->inp = (struct v4l2_input*)malloc(sizeof(struct v4l2_input));
				this->fmt = (struct v4l2_format*)malloc(sizeof(struct v4l2_format));
				this->crop = (struct v4l2_crop*)malloc(sizeof(struct v4l2_crop));
				this->cropcap = (struct v4l2_cropcap*)malloc(sizeof(struct v4l2_cropcap));
		    this->req = (struct v4l2_requestbuffers*)malloc(sizeof(struct v4l2_requestbuffers)); 
				this->compr = (struct v4l2_jpegcompression*)malloc(sizeof(struct v4l2_jpegcompression));
				this->control = (struct v4l2_control*)malloc(sizeof(struct v4l2_control));
				this->buf = (struct v4l2_buffer*)malloc(sizeof(struct v4l2_buffer));
				this->queryctrl = (struct v4l2_queryctrl*)malloc(sizeof(struct v4l2_queryctrl));
				this->timestamp = (struct timeval*)malloc(sizeof(struct timeval));
				*(this->count) = DEFAULT_COUNT;
				this->field = V4L2_FIELD_ANY;
				this->width = REC_DEFAULT_WIDTH;
				this->height = REC_DEFAULT_HEIGHT;
				this->pixelformat = V4L2_PIX_FMT_H264;
				this->shmkey = REC_SHM_KEY;
				this->shmkey_for_property = REC_SHM_KEY_FOR_PROPERTY;
				this->shmPtr_for_property = NULL;
	}

}

CameraProperty::CameraProperty()
{

			
				this->count = (unsigned int*)malloc(sizeof(unsigned int));
				this->cap = (struct v4l2_capability*)malloc(sizeof(struct v4l2_capability));
				this->inp = (struct v4l2_input*)malloc(sizeof(struct v4l2_input));
				this->fmt = (struct v4l2_format*)malloc(sizeof(struct v4l2_format));
				this->crop = (struct v4l2_crop*)malloc(sizeof(struct v4l2_crop));
				this->cropcap = (struct v4l2_cropcap*)malloc(sizeof(struct v4l2_cropcap));
		    this->req = (struct v4l2_requestbuffers*)malloc(sizeof(struct v4l2_requestbuffers)); 
				this->compr = (struct v4l2_jpegcompression*)malloc(sizeof(struct v4l2_jpegcompression));
				this->control = (struct v4l2_control*)malloc(sizeof(struct v4l2_control));
				this->buf = (struct v4l2_buffer*)malloc(sizeof(struct v4l2_buffer));
				this->queryctrl = (struct v4l2_queryctrl*)malloc(sizeof(struct v4l2_queryctrl));
				this->timestamp = (struct timeval*)malloc(sizeof(struct timeval));
				*(this->count) = DEFAULT_COUNT;
				this->field = V4L2_FIELD_ANY;
				this->width = DEFAULT_WIDTH;
				this->height = DEFAULT_HEIGHT;
				this->pixelformat = V4L2_PIX_FMT_RGB24;


				//	this->st = (struct stat*) malloc(sizeof(struct stat));
}
CameraProperty::~CameraProperty()
{
				
				free(this->count);
				free(this->cap);
				free(this->inp);
				free(this->fmt);
				free(this->crop);
				free(this->cropcap);
				free(this->req);
				free(this->compr); 
				free(this->control); 
				free(this->buf); 
				free(this->queryctrl); 
				free(this->timestamp);
				//free(this->st);
				if(this->mutex != SEM_FAILED)
				{
					sem_close(this->mutex);
					sem_unlink("openCVProp");
				}
}
