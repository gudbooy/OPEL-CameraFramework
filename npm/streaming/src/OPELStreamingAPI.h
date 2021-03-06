#ifndef _OPEL_STREAMING_H_
#define _OPEL_STREAMING_H_
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
	#include <sys/socket.h>
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
#define INFINITE_NUM 10000000
#define PORT_NUM 9488

//const char lengthPassword[] = "Esevan Length Encryption Algorithm";

char SEM_NAME[] = "vik";
static bool eos;
class StreamingWorker : public Nan::AsyncWorker
{
	public:

		StreamingWorker(Nan::Callback* callback, int count) : Nan::AsyncWorker(callback), count(count), fd(0), width(0), height(0), buffer_size(0), buffer_index(0), fout(NULL), shmPtr(NULL) {}

		~StreamingWorker() {}

		void Execute()
		{
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
					if(writeFrame()){
							break;
					}	
					}
				}
			//PES 
			this->closeConnection();
			//sendDbusMsg(Done);
			sendDbusMsgCnt("recStart", 0); //termination
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

				bool res = false;

					
				sem_wait(mutex);
				if(*check)
				{
						fprintf(stderr, "length : %d\n", *length);
						//Network Write - Target Buffer : (char*)buffer, Length : *length
						//PES
						int curr, bytes, total, i;
						unsigned int len;
						unsigned int offset;
						

						bytes = 0;
						curr = 0;
						total = *length;
						len = htonl(*length);

						unsigned char buff[1460] = {0,};

						memcpy(buff, &len, sizeof(len));

						while(curr < total){
							offset = htonl(curr);
							memcpy(buff+4, &offset, sizeof(offset));
							for(i=0; i<1452; i++){
								buff[i+8] = *((char*)buffer+(curr+i));
							}
							bytes = write(client_desc, buff, 1460);
							if(bytes < 0){
								fprintf(stderr, "Writing error\n");
								close(client_desc);
								eos = false;
								break;
							}
							else if(bytes == 0){
								fprintf(stderr, "Disconnected \n");
								close(client_desc);
								eos = false;
								break;
							}

							curr += bytes-8;							
						}

						if(curr >= total){
							fprintf(stderr, "Frame has successfully sent\n");
						//	sem_post(mutex);
							res = true;
						//		break;
						}
						else{
							fprintf(stderr, "Something woring happens, check the bugs\n");
							eos = false;
						//		break;
						}
				}
				else{
					printf("skip!!!\n");
/*					if(count == 0)
					{
						this->count = 1;
					}*/
				}
				sem_post(mutex);
				usleep(100000); //10FPS 
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
		bool initConnection(void)
		{	
			//PES
			//Network Connection Initialization code
			fprintf(stderr, "InitConnection\n");
			socket_desc = socket(AF_INET, SOCK_STREAM, 0);

			if(socket_desc == -1){
				fprintf(stderr, "Could not create socket\n");
				return false;
			}

			server.sin_family = AF_INET;
			server.sin_addr.s_addr = INADDR_ANY;
			server.sin_port = htons(PORT_NUM);

			if ( bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0){
				fprintf(stderr, "Bind failed\n");
				return false;
			}

			listen(socket_desc, 3);

			fprintf(stderr, "Wating for incomming connection...");
			struct sockaddr_in cli_addr;
			int c = sizeof(struct sockaddr_in);
			client_desc = accept(socket_desc, (struct sockaddr *)&cli_addr, (socklen_t *)&c);
			if (client_desc < 0){
				fprintf(stderr, "failed\n");
				return false;
			}
			else{
				fprintf(stderr, "OK \n");
			}

			return true;
		}
		bool sendDbusMsgCnt(const char* msg, int count)
		{
			DBusMessageIter args;
			dbus_int32_t cnt = (dbus_int32_t)count;
			DBusMessage* message;
			message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", msg);
			dbus_message_iter_init_append(message, &args);
			if(!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &cnt))
			{
				fprintf(stderr, "[OPELRecording::sendDbusMsgCnt] : dbus append args error\n");
				return false;
			}
			dbus_connection_send(conn, message, NULL);
			dbus_message_unref(message);
			return true;
		}
		void setDbusConnection(DBusConnection* conn, DBusError err)
		{
			this->conn = conn;
			this->err = err;
		}
		bool closeConnection(void)
		{
			//PES
			//Network Connection  Close code
			fprintf(stderr, "CloseConnection\n");
			close(client_desc);
			close(socket_desc);

			return true;
		}

	private:
			
			int count;
			int fd;
			int width, height; 
			int buffer_size;
			int buffer_index;
			FILE* fout;
			int semid;
			int shmid; 
			void* shmPtr;
			bool isInfinite;
			sem_t *mutex;

			int socket_desc;
			int client_desc;
			struct sockaddr_in server;
			DBusConnection* conn;
			DBusError err;
};

class OPELStreaming : public Nan::ObjectWrap{

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
		DBusConnection* getConn() { return this->conn; }
		DBusError getErr() { return this->err; }
	private:
		explicit OPELStreaming();
		~OPELStreaming();
	  
		static NAN_METHOD(New);
	
		static NAN_METHOD(streamInit);
		static NAN_METHOD(streamStart);
		static NAN_METHOD(streamStop);
//		static NAN_METHOD(recClose);	
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

#endif /* _OPEL_STREAMING_H_ */

