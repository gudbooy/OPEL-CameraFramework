#ifndef OPEL_CAMERA_CONFIGURE_H_
#define OPEL_CAMERA_CONFIGURE_H_

extern "C"{
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <types.h>
}

#include <v8.h>
#include <node.h>
#include <uv.h>
#include <nan.h>
typedef struct v4l2_ctrl_range{
	__u32 ctrl_id;
	__s32 initial_value;
	__s32 current_value;
	__s32 minimum;
	__s32 maxmum;
	__s32 default_value;
}v4l2_ctrl_range;

class OPELCameraConfigure : public Nan::ObjectWrap{
	public:
		static NAN_MODULE_INIT(Init);
		bool openDevice();
		int v4l2_get_ctrl_min();
	private:
		explicit OPELCameraConfigure();
		~OPELCameraConfigure();


		static NAN_METHOD(New);

		static NAN_METHOD(ConfigureInit);
		//Get Parameters
		static NAN_METHOD(getBrightness);
		static NAN_METHOD(getContrast);
		static NAN_METHOD(getSaturation);
		static NAN_METHOD(getHue);
		static NAN_METHOD(getGain);
		static NAN_METHOD(getExposure);

		//Set Parameters
		static NAN_METHOD(setBrightness);
		static NAN_METHOD(setContrast);
		static NAN_METHOD(setSaturation);
		static NAN_METHOD(setHue);
		static NAN_METHOD(setGain);
		static NAN_METHOD(setExposure);

		static inline Nan::Persistent<v8::Function>& constructor()
		{
			static Nan::Persistent<v8::Function> my_constructor;
			return my_constructor;
		}
		struct v4l2_control control;
		struct v4l2_queryctrl queryctrl;
		v4l2_ctrl_range** v4l2_ctrl_ranges;
		int v4l2_ctrl_count;
		int fd; // V4L2 Camera Device
};

#endif
