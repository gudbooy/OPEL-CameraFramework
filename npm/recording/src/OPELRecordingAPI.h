#ifndef _OPEL_RECORDING_H_
#define _OPEL_RECORDING_H_
extern "C"{
	#include <semaphore.h>
	#include <stdio.h>
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
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <sys/sem.h>
	#include <getopt.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <sys/stat.h>
}
#include <v8.h>
#include <node.h>
#include <uv.h>
#include <nan.h>

#define REC_SHM_KEY 9447
#define ERR_INDEX 0
#define DATA_INDEX 1
#define REC_WIDTH 1920
#define REC_HEIGHT 1080
#define REC_BUFFER_SIZE 4147200
#define REC_BUFFER_INDEX 4
#define SEM_FOR_PAYLOAD_SIZE 9948
char SEM_NAME[] = "vik";
union semun
{
	int val;
	struct semid_ds* buf;
	unsigned short int *array;
};
static bool eos;
class RecordingWorker : public Nan::AsyncWorker
{
	public:

		RecordingWorker(Nan::Callback* callback, const char* file_path, int count) : Nan::AsyncWorker(callback), file_path(file_path), count(count), fd(0), width(0), height(0), buffer_size(0), buffer_index(0), fout(NULL), shmPtr(NULL) {}

		~RecordingWorker() {}

		void Execute()
		{
				pid_t pid;
				while((count-- > 0) && eos)
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
							if(EINTR == errno){
								return ;
							}
						}
						if(0 == r)
						{
							break;
						}
					if(writeFrame())
							break;
					}
				
				}
				fflush(fout);
				closeFileCap();
				//FFmpeg Muxing Start
		}
		
		bool writeFrame()
		{
				
				unsigned sz;
				unsigned offset;
				unsigned offset_length;
				unsigned offset_check;
			
				offset = (buffer_index-1)*buffer_size;
				offset_length = buffer_index*buffer_size;
				offset_check = offset_length+sizeof(int);
				
				void* _buffer = (void*)shmPtr+offset;
				void* _length = (void*)shmPtr+offset_length;
				void* _check = (void*)shmPtr+offset_check;
				
				int* check = (int*)_check;
				void* buffer = _buffer; 
				int* length = (int*)_length;

					
				sem_wait(mutex);
				if(*check)
				{
						fprintf(stderr, "length : %d\n", *length);
					sz = fwrite((char*)buffer, sizeof(char), *length, fout);	
					if(sz != *length)
					{
							fprintf(stderr, "ERROR Occurred\n");
					}
				}
				else{
					printf("skip!!!\n");
					if(count == 0)
					{
						this->count = 1;
					}
				/*	else
					{
						this->count++;
					}*/
				}
				sem_post(mutex);
				usleep(100);
				return true;
		}
		void HandleOKCallback()
		{
			Nan::HandleScope scope;
			v8::Local<v8::Value>  argv[]  = {
				Nan::Null(), Nan::New<v8::Number> (1) 
			};
			callback->Call(2, argv);
		}	
		void HandleErrorCallback()
		{
			Nan::HandleScope scope;
			v8::Local<v8::Value> argv[] = {
				Nan::Null(), Nan::New<v8::Number>(1)
			};
			callback->Call(2, argv);
		}
		void setFd(int fd) { this->fd = fd; } 
		void setWidth(int width) { this->width = width; }
		void setHeight(int height) { this->height = height; }
		void setBufferSize(int buffer_size) { this->buffer_size = buffer_size; }
		void setBufferIndex(int buffer_index) { this->buffer_index = buffer_index; } 
	  void setShmPtr(void* shmPtr) { this->shmPtr = shmPtr;
		}	
		bool initSEM(void)
		{
			mutex = sem_open(SEM_NAME, 0, 0666, 0); 
			if(mutex == SEM_FAILED)
			{
				sem_unlink(SEM_NAME);
				return false;
			}
			return true;

		}
		bool initSHM(void)
		{
			shmid = shmget((key_t)REC_SHM_KEY, 0, 0);
			if(shmid == -1)
				return false;
			shmPtr = shmat(shmid, (void*)0, 0666|IPC_CREAT);
			if(shmPtr == (void*)-1){
				return false;
			}	
			return true;
		}
		bool openFileCap(void)
		{	
			fout = fopen(file_path, "w+");	
			fprintf(stderr, "file_path!!!!!! : %s\n", file_path);
			if(!fout){
				return false;
			}
			return true;
		}
		void closeFileCap(void)
		{
			if(fout)
				fclose(fout);
		}
	private:
			
			const char* file_path;
			int count;
			int fd;
			int width, height; 
			int buffer_size;
			int buffer_index;
			FILE* fout;
			int semid;
			int shmid; 
			void* shmPtr;
			sem_t *mutex;
};

class OPELRecording : public Nan::ObjectWrap{

	public:
		static NAN_MODULE_INIT(Init);
  	void sendDbusMsg(const char* msg); 
		bool sendDbusMsgCnt(const char* msg, int count);
		bool initDbus();
		//void init(const Nan::FunctionCallbackInfo<v8::value>& info);
		bool openDevice();

		bool initSharedMemorySpace();
		bool uInitSharedMemorySpace();
		int getFd() { return this->fd; } 
		int getWidth() { return this->width; }
		int getHeight() { return this->height; }
		int getBufferSize() { return this->buffer_size; }
		int getBufferIndex() { return this->buffer_index; }
		void* getShmPtr() { return this->shmPtr; }

	private:
		explicit OPELRecording();
		~OPELRecording();
	  
		static NAN_METHOD(New);
	
		static NAN_METHOD(recInit);
		static NAN_METHOD(recStart);
		static NAN_METHOD(recStop);
		static NAN_METHOD(recClose);	
		//	static Nan::Persistent<v8::Function> constructor;	
		static inline Nan::Persistent<v8::Function>& constructor()
		{
			static Nan::Persistent<v8::Function> my_constructor;
			return my_constructor;
		}
		int width; 
		int height;
		int buffer_size;
		int buffer_index;
		DBusConnection* conn;
		DBusError err;
		void* shmPtr;		
		int shmid;
		int fd;
};
//NODE_MODULE(OPELRecording, OPELRecording::Init)

#endif /* _OPEL_RECORDING_H_ */

