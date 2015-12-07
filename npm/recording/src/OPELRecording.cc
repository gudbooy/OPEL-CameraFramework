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

const char* dev_name = "/dev/video0";

bool RecordingWorker::readFrame()
{

}

void RecordingWorker::Execute()
{
	while(count-- > 0)
	{
		for(;;)
		{
			fd_set fds;
			struct timeval tv;
			int r; 
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			tv.tv_sec = 10;
			tv.tv_usec = 0;
			r = select(fd+1, &fds, NULL, NULL, &tv);
			if(-1 == r)
			{
				if(EINTR == errno)
					continue;
			}
			if(0 == r)
			{
				break;
			}
			if(readFrame())
				break;
		}
	}
}

OPELRecording::OPELRecording()
{
	this->width = REC_WIDTH;
	this->height = REC_HEIGHT;
	this->buffer_size = REC_BUFFER_SIZE;
	this->buffer_index = REC_BUFFER_INDEX;
}
void OPELRecording::sendDbusMsg(const char* msg)
{
	DBusMessage* message;
	message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", msg);
	dbus_connection_send(conn, message, NULL);
	dbus_message_unref(message);
}

bool OPELRecording::initDbus()
{
  dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if(!conn)
	{
		dbus_error_free(&err);
		return false;
	}
	return true;
}

bool OPELRecording::openDevice()
{
	this->fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if(-1 == fd)
		return false;
	return true;
}

bool OPELRecording::initSharedMemorySpace()
{
	shmid = shmget((key_t)REC_SHM_KEY, 0, 0);
	if(shmid == -1)
		return false;
	shmPtr = shmat(shmid, (void*)0, 0666|IPC_CREAT);
	if(shmPtr == (void*)-1)
		return false;
	return true;
}

NAN_METHOD(OPELRecording::New)
{
	//Nan::NanScope();
	if(info.IsConstructCall())
	{
		OPELRecording *recObj = new OPELRecording();
		recObj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}
	else
	{
		v8::Local<v8::Function> cons = Nan::New(constructor());
		info.GetReturnValue().Set(cons->NewInstance(0, 0));
	}
}
NAN_MODULE_INIT(OPELRecording::Init)
{
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("OPELRecording").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	SetPrototypeMethod(tpl, "init", recInit);
	SetPrototypeMethod(tpl, "start", recStart);
	SetPrototypeMethod(tpl, "stop", recStop);
	SetPrototypeMethod(tpl, "close", recClose);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OPELRecording").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NODE_MODULE(OPELRecording, OPELRecording::Init)
