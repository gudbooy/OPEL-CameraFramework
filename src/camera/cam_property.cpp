#include "cam_property.h"

CameraProperty* CameraProperty::camProp = NULL;

CameraProperty* CameraProperty::getInstance()
{
				if(camProp == (void*)0)
								camProp = new CameraProperty();
				return camProp;
}
void CameraProperty::printSetValue(void)
{
				std::cout << "***** OPEL CAMERA PROPERTIES *****" <<std::endl;
				std::cout << "Width : " << this->width << " Height : " << this->height << std::endl;
				std::cout << "Pixelformat : " << this->pixelformat << std::endl;
				std::cout << "Number of Frame : " << *(count) << std::endl; 
				std::cout << "Field : " << this->field << std::endl;
				
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
}
