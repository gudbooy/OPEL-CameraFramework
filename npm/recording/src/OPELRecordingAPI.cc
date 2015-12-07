#include <v8.h>
#include <node.h>
#include <nan.h>
#include <node_buffer.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <assert.h>
#include <dbus/dbus.h>
#include "OPELRecording.h"
#include <string>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

extern "C"{
	#include <stdio.h>
}

// addon.init(function(err, data){ data has width, height, buffer size});
//

NAN_METHOD(OPELRecording::recInit)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
		
	//if(!info[0]->IsString())
	//{
	 //	Nan::ThrowTypeError("Is Not String Format");
	//}
	if(!(recObj->openDevice()))
	{
		//error
		Nan::ThrowError("Open FileDesc /dev/video0 Failed\n");
		return;
	}
	if(!(recObj->initSharedMemorySpace()))
	{
		//error
		Nan::ThrowError("Initialize Shared Memory Space Failed\n");
		return;
	}
	recObj->sendDbusMsg("recInit");
}
NAN_METHOD(OPELRecording::recStart)
{	
	int count;
	const char* file_path;
	if(!info[0]->IsString())
	{
		Nan::ThrowTypeError("Put Recording File Path");
		return;
	}
	if(!info[1]->IsNumber())
	{
		Nan::ThrowTypeError("Put Recording Count");
		return;
	}
//std::string file_path = Nan::To<std::string>(info[0]).FromJust();
	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);
	file_path = path.c_str();
	
	count = Nan::To<int>(info[1]).FromJust();
	
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());


	recObj->sendDbusMsg("recStart");
	Nan::Callback *callback = new Nan::Callback();
	RecordingWorker* recWorker = new RecordingWorker(callback, file_path, count);	
	
	recWorker->setFd(recObj->getFd());
	recWorker->setWidth(recObj->getWidth());
	recWorker->setHeight(recObj->getHeight());
	recWorker->setBufferSize(recObj->getBufferSize());
	recWorker->setBufferIndex(recObj->getBufferIndex());
	
	Nan::AsyncQueueWorker(recWorker);
	

}



