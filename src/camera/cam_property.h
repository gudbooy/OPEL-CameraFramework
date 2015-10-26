#include "cam_core.h"
class CameraProperty
{
				public: 
								static CameraProperty* getInstance();
								~CameraProperty();
				private:
								static CameraProperty* camProp;
								CameraProperty() {};
								int deviceHandle;
								int bufferIndex;
								int width, height;
								int pixelformat;
								enum v4l2_field field;
								int mode;
								unsigned long long count;
								unsigned int n_buffer;

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
};
/*
CameraProperty* CameraProperty::getInstance()
{
				if(cameraProperty == (void*)0)
						cameraProperty = new CameraProperty();								
				return cameraProperty;
}
*/

