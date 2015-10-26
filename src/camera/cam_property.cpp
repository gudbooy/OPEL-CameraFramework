#include "cam_property.h"

CameraProperty* CameraProperty::camProp = NULL;

CameraProperty::~CameraProperty()
{
				if(camProp != NULL)
								delete camProp;
				camProp = NULL;
}

CameraProperty* CameraProperty::getInstance()
{
				if(camProp == (void*)0)
								camProp = new CameraProperty();
				return camProp;
}
