#include "cam_cap_check.h"
#include "cam_core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/ioctl.h>

static void checkDeviceCapabilites(void)
{
		Camera* cam = Camera::getInstance();
		int fd = cam->get_fd;
		if(xioctl(fd, VIDIOC_QUERYCAP, &cap))
		{
						fprintf(stderr, "%s querycap error!!!");
						exit(0);
		}
		capabilities = cap.capabilities;
		if(capabilities & V4L2_CAP_DEVICE_CAPS)
						capabilities = cap.device_caps;

		if(V4L2_CAP_VIDEO_CAPTURE & capabilities)
						printf("Supports the single-planar API\n");
		if(V4L2_CAP_VIDEO_CAPTURE_MPLANE & capabilities)
						printf("Supports the multi-planar API\n");
		if(V4L2_CAP_VIDEO_OUTPUT & capabilities)
						printf("Supports the single-planar Ouput\n");
		if(V4L2_CAP_VIDEO_CAPTURE_OUTPUT_MPLANE & capabilities)
						printf("Supports the multi-planar Output\n");
		if(V4L2_CAP_VIDEO_M2M & capabilities)
						printf("Supports the M2M interface\n");
		if(V4L2_CAP_VIDEO_M2M_MPLANE & capabilities)
						printf("Supports the M2M multi-planar\n");
		if(V4L2_CAP_VIDEO_OVERLAY & capabilities)
						printf("Supports the video overlay interface\n");
		if(V4L2_CAP_ASYNCIO & capabilities)
						printf("Supports the AsyncI/O\n");
		
}












