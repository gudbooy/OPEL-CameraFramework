#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include<string.h>
#include<poll.h>
#include<getopt.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include<sys/wait.h>
#include<semaphore.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <linux/videodev2.h>
#define CLEAR(x) memset(&(x), 0, sizeof(x))
char SEM_FOR_SELECT[] = "vvik";
//using namespace cv;
int buffer_len;
int buffer_num;
int fd;
static char* dev_name;
int count = 1000;
int n_buffer;
void* shmem;
sem_t* mx;
static int xioctl(int fh, int request, void* arg)
{
				int r;
				do{
								r = ioctl(fh, request, arg);
				}while(-1 == r && EINTR == errno);
				return r;
}
bool checkFPS(unsigned& cnt, unsigned& last, struct timeval &tv_last)
{

	if(cnt == 0)
		gettimeofday(&tv_last, NULL);
	else
	{
		struct timeval tv_cur, res;
		gettimeofday(&tv_cur, NULL);
		timersub(&tv_cur, &tv_last, &res);
		if(res.tv_sec){
			unsigned fps =(cnt - last);
			last = cnt;
			tv_last = tv_cur;
			fprintf(stderr, " %d fps\n", fps);
		}
	}
	cnt++;
}
static int getCameraProperty(void)
{
/*				fd = open(dev_name, O_RDWR | O_NONBLOCK,0);
				struct v4l2_buffer buf;
				CLEAR(buf); 
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index = 4;
				buffer_len = buf.length; 
				buffer_num = buf.index;
				if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
				{
								printf("error! getProperty");
								return 0;
				}*/
		buffer_len = 921600; 
		buffer_num = 4;
		return 1;
}
static void mainloop(void)
{
				struct pollfd pollfds[1];
				pollfds[0].fd = fd;
				pollfds[0].events = POLLIN;
				int retval;
				int n=0;
				unsigned cnt=0;
				unsigned last=0;
				struct timeval tv_last;
				fd_set fds;
				for(;;)
				{
					retval = poll(pollfds, 1, -1);
					if(pollfds[0].revents & POLLIN)
					{
						fprintf(stderr, "%d\n", ++n);
					}
					usleep(100);
				}

			//	FD_ZERO(&fds);
			//	FD_SET(fd, &fds);
		/*		while(count-- > 0)
				{
								for(;;){
									struct timeval tv;
									int r;
									FD_ZERO(&fds);
									FD_SET(fd, &fds);
									tv.tv_sec = 2;
									tv.tv_usec = 0;
									r = select(fd+1, &fds, NULL, NULL, &tv);
									if(-1 == r)
									{
													if(EINTR == errno)
																	continue;
											//		printf("select error\n");
											//		exit(1);
												//	errno_exit("select");
									}
									else if(0 == r)
									{
													fprintf(stderr, "select timeout\n");
										//			exit(EXIT_FAILURE);
									}
	//								printf("hihihihi");
									else{
									if(FD_ISSET(fd, &fds))
									{
									
									}	
									}
									fprintf(stderr, "%d\n", ++n);
									//checkFPS(cnt, last, tv_last);
								//	if(n_buffer < buffer_num) 								
								//	{
									   	
						//					fwrite(shmem+n_buffer*buffer_len, buffer_len, 1, stdout);
						//					n_buffer++;
						//			}
						//			else
						//			{
						//							n_buffer=0;
						//			}
								}
				}*/
}
int main()
{
	dev_name = "/dev/video0";
				//	void* shmem;
	int shmid;
	 n_buffer = 0;
//	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
/*	 if( -1 == mkfifo("/tmp/fifo", 0666))
	 {
		 fprintf(stderr, "Failed To make FIFO\n");
		 exit(1);
	 }*/
	 fd = open("/tmp/fifo", O_RDWR | O_NONBLOCK, 0);
	 if(-1 == fd)
	{
					printf("fd open error!!!\n");
					exit(1);
	}
//	 printf("hihihihi");
	mx = sem_open(SEM_FOR_SELECT, O_CREAT, 0666, 1);
	if(mx == SEM_FAILED)
	{
		fprintf(stderr, "SEMAHPORE_FAILE\n");
		sem_unlink(SEM_FOR_SELECT); 
		exit(1);
		//	return false;
	}
//	return true;
/*	shmid = shmget((key_t)1234, 0, 0);
	if(shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}
	shmem = shmat(shmid, (void*)0, 0666|IPC_CREAT);
	if(shmem == (void*)-1)
	{
		perror("shmat failed : ");
		exit(0);
	}*/
	if(!getCameraProperty())
	{

					printf("error");
					return 0;
	}
	mainloop();
//	cvNamedWindow("window", CV_WINDOW_AUTOSIZE);
	//cv::namedWindow("window", CV_WINDOW_AUTOSIZE);
/*		for(;;)
	{

				if(buffer_num > n_buffer)
				{
//								IplImage frame;
	//							memcpy((char*)frame.imageData, (char*)(shmem+n_buffer*buffer_len), buffer_len);
	  						//CvMat cvmat = cvMat(960, 1280, CV_8UC3, (void*)shmem+n_buffer*buffer_len);
								//IplImage *img;
								//img = cvDecodeImage(&cvmat, 1);
						//		cvShowImage("window", &frame);
								//cv::imshow("window", frame);
	//							printf("hohohoho\n");
		//						printf("%x\n", &shmem+n_buffer*buffer_len);
							fwrite(shmem+n_buffer*buffer_len, buffer_len, 1, stdout);
							fwrite(shmem+n_buffer*buffer_len, buffer_len, 1, stdout);
								n_buffer++;
				}
				else
				 	n_buffer = 0;

				//		printf("%s\n", (char*)shmem+n_buffer*buffer_len);
	}*/
	
				return 0;
}
