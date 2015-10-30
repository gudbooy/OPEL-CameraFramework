#include "cam_property.h"

CameraProperty* CameraProperty::camProp = NULL;

CameraProperty* CameraProperty::getInstance()
{
				if(camProp == (void*)0)
								camProp = new CameraProperty();
				return camProp;
}

CameraProperty::CameraProperty()
{
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
			//	this->st = (struct stat*) malloc(sizeof(struct stat));
}
CameraProperty::~CameraProperty()
{
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
