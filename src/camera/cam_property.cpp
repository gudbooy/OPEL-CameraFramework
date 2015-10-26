#include "cam_core.h"

CamProperty::CamProperty()
{
				this->width = 640;
				this->height = 480;
				this->pixelformat = V4L2_PIX_FMT_YUYV;
				this->field = V4L2_FIELD_INTERLACED;
}
CamProperty::CamProperty(int width, int height, int pixelformat, enum v4l2_field field)
{
				this->width = width;
				this->height = height;
				this->pixelformat = pixelformat;
				this->field = field;
}
CamProperty::~CamProperty()
{
	//nothing
}

