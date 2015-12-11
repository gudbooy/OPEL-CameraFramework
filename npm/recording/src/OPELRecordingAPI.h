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
static bool eos;
union semun
{
	int val;
	struct semid_ds* buf;
	unsigned short int *array;
};
static struct sembuf status_post = {0, -1, SEM_UNDO};
static struct sembuf status_wait = {0, 1, SEM_UNDO};
class RecordingWorker : public Nan::AsyncWorker
{
	public:

		RecordingWorker(Nan::Callback* callback, const char* file_path, int count) : Nan::AsyncWorker(callback), file_path(file_path), count(count), fd(0), width(0), height(0), buffer_size(0), buffer_index(0), fout(NULL), shmPtr(NULL) {}

		~RecordingWorker() {}

		void Execute()
		{
		  //if(openFileCap())
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
								return ;
						}
						if(0 == r)
						{
							break;
						}
						if(readFrame())		
							break;
					}
				
				}
			fflush(fout);
				closeFileCap();
				printf("close\n");
		}
		
		bool readFrame()
		{
				int* length;
				int len;
				unsigned sz;
				unsigned offset;
				unsigned offset_size;
				offset = (buffer_index-1)*buffer_size;
				offset_size = buffer_index*buffer_size;
				//				FILE* fp = fopen(file_path, "ab");
				//semop(semid, &status_wait, 1);
				
				sem_wait(mutex);
				length = (int*)(shmPtr+offset_size);
				fprintf(stderr, "length[1] : %d\n", *length);
				sz = fwrite((char*)shmPtr+offset, sizeof(char), *length, fout);	
				if(sz != *length)
				{
					fprintf(stderr, "%u != %d\n", sz, *length);
				}
				sem_post(mutex);
				
				//semop(semid, &status_post, 1);
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
	  void setShmPtr(void* shmPtr) { this->shmPtr = shmPtr; }	
		bool initSEM(void)
		{
			mutex = sem_open(SEM_NAME, 0, 0666, 0); 
			if(mutex == SEM_FAILED)
			{
				sem_unlink(SEM_NAME);
				return false;
			}
			
	/*		union semun sem_union;
			semid = semget((key_t)SEM_FOR_PAYLOAD_SIZE, 0, 0);
			if(-1 == semid)
				return false;
			sem_union.val = 1;
			if(-1 == semctl(semid, 0, SETVAL, sem_union))
				return false;*/

			
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
			char cwdd[1024];
		fout = fopen(file_path, "w+");	
			//return true;	
			if(!fout){
				fprintf(stderr, "cwd:%s\n", getcwd(cwdd, 1024));
				fprintf(stderr, "error:%d,%s\n", errno, strerror(errno));
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
		bool initDbus();
		//void init(const Nan::FunctionCallbackInfo<v8::value>& info);
		bool openDevice();

		bool initSharedMemorySpace();
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

