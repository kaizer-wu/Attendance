#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "../include/cam.h"
#include "../include/sig.h"

#define	REQBUFS_COUNT	4

extern struct jpg_buf_t *jpg;
extern pthread_mutex_t cam_mutex;

struct cam_buf {
	void *start;
	size_t length;
};

static struct v4l2_requestbuffers reqbufs;
static struct cam_buf bufs[REQBUFS_COUNT];   
static int Exitflag;
static int Stopflag;


int camera_init(char *devpath, unsigned int *width, unsigned int *height, unsigned int *size);
int camera_start(int fd);
int camera_dqbuf(int fd, void **buf, unsigned int *size, unsigned int *index);
int camera_eqbuf(int fd, unsigned int index);
int camera_stop(int fd);
int camera_exit(int fd);
void camSigHandler(int sigid);

void camSigHandler(int sigid)
{
	switch(sigid)
	{
		case SIGBGIN:
			Stopflag=0;
			break;
		case SIGSUCC:
			Stopflag=1;
			break;
		case SIGFAIL:
			Stopflag=1;
			break;
		case SIGEROR:
			Stopflag=1;
			break;
		case SIGEXIT:
			Stopflag=1;
			Exitflag=1;
			break;
	}
}

int camera_init(char *devpath, unsigned int *width, unsigned int *height, unsigned int *size)
{
	int i;
	int fd = -1;;
	int ret;
	struct v4l2_buffer vbuf;
	struct v4l2_format format;
	struct v4l2_capability capability;
	Exitflag = 0;
	if((fd = open(devpath, O_RDWR)) == -1){
        perror("camera_init open");
		return -1;		
	}
	/*ioctl 查看支持的驱动*/
	ret = ioctl(fd, VIDIOC_QUERYCAP, &capability);
	if (ret == -1) {
		perror("camera->init ");
		return -1;
	}
	/*判断设备是否支持视频采集*/
	if(!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "camera->init: device can not support V4L2_CAP_VIDEO_CAPTURE\n");
		close(fd);
		return -1;
	}
	/*判断设备是否支持视频流采集*/
	if(!(capability.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "camera->init: device can not support V4L2_CAP_STREAMING\n");
		close(fd);
		return -1;
	}
#ifdef VIDEO_FORMAT_MJPEG
	/*设置捕获的视频格式MJPEG*/
	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	format.fmt.pix.width = *width;
	format.fmt.pix.height = *height;
	format.fmt.pix.field = V4L2_FIELD_ANY;
	ret = ioctl(fd, VIDIOC_S_FMT, &format);
	if(ret == -1)
		perror("camera init  8888");
	else {
		fprintf(stdout, "camera->init: picture format is mjpeg\n");
	}
#else
	/*设置捕获的视频格式YUYV*/
	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	format.fmt.pix.width = *width;
	format.fmt.pix.height = *height;
	format.fmt.pix.field = V4L2_FIELD_ANY;
	ret = ioctl(fd, VIDIOC_S_FMT, &format);
	if(ret == -1)
		perror("camera init  8888");
	else {
		fprintf(stdout, "camera->init: picture format is yuyv\n");
	}

#endif
	ret = ioctl(fd, VIDIOC_G_FMT, &format);
	if (ret == -1) {
		perror("camera init");
		return -1;
	}
	/*向驱动申请缓存*/
	memset(&reqbufs, 0, sizeof(struct v4l2_requestbuffers));
	reqbufs.count	= REQBUFS_COUNT;
	reqbufs.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbufs.memory	= V4L2_MEMORY_MMAP;
	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbufs);			
	if (ret == -1) {	
		perror("camera init");
		close(fd);
		return -1;
	}
	/*循环映射并入队*/
	for (i = 0; i < reqbufs.count; i++){
		/*真正获取缓存的地址大小*/
		memset(&vbuf, 0, sizeof(struct v4l2_buffer));
		vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vbuf.memory = V4L2_MEMORY_MMAP;
		vbuf.index = i;
		ret = ioctl(fd, VIDIOC_QUERYBUF, &vbuf);
		if (ret == -1) {
			perror("camera init");
			close(fd);
			return -1;
		}
		/*映射缓存到用户空间,通过mmap讲内核的缓存地址映射到用户空间,并切和文件描述符fd相关联*/
		bufs[i].length = vbuf.length;
		bufs[i].start = mmap(NULL, vbuf.length, PROT_READ | PROT_WRITE, 
		                     MAP_SHARED, fd, vbuf.m.offset);
		if (bufs[i].start == MAP_FAILED) {
			perror("camera init");
			close(fd);
			return -1;
		}
		/*每次映射都会入队,放入缓冲队列*/
		vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vbuf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_QBUF, &vbuf);
		if (ret == -1) {
			perror("camera init");
			close(fd);
			return -1;
		}
	}
	/*返回真正设置成功的宽.高.大小*/
	*width = format.fmt.pix.width;
	*height = format.fmt.pix.height;
	*size = bufs[0].length;

	signal(SIGBGIN,camSigHandler);
	signal(SIGSUCC,camSigHandler);
	signal(SIGFAIL,camSigHandler);
	signal(SIGEROR,camSigHandler);
	signal(SIGEXIT,camSigHandler);

	return fd;
}


int camera_start(int fd)
{
	int ret;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/*ioctl控制摄像头开始采集*/
	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if (ret == -1) {
		perror("camera->start");
		return -1;
	}
	fprintf(stdout, "camera->start: start capture\n");

	return 0;
}

int camera_dqbuf(int fd, void **buf, unsigned int *size, unsigned int *index)
{
	int ret;
	fd_set fds;
	struct timeval timeout;
	struct v4l2_buffer vbuf;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
		if (ret == -1) {
			perror("camera->dbytesusedqbuf");
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else if (ret == 0) {
			fprintf(stderr, "camera->dqbuf: dequeue buffer timeout\n");
			return -1;
		} else {
			vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vbuf.memory = V4L2_MEMORY_MMAP;
			ret = ioctl(fd, VIDIOC_DQBUF, &vbuf);
			if (ret == -1) {
				perror("camera->dqbuf");
				return -1;
			}
			*buf = bufs[vbuf.index].start;
			*size = vbuf.bytesused;
			*index = vbuf.index;

			return 0;
		}
	}
}

int camera_eqbuf(int fd, unsigned int index)
{
	int ret;
	struct v4l2_buffer vbuf;

	vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbuf.memory = V4L2_MEMORY_MMAP;
	vbuf.index = index;
	ret = ioctl(fd, VIDIOC_QBUF, &vbuf);
	if (ret == -1) {
		perror("camera->eqbuf");
		return -1;
	}

	return 0;
}

int camera_stop(int fd)
{
	int ret;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
	if (ret == -1) {
		perror("camera->stop");
		return -1;
	}
	fprintf(stdout, "camera->stop: stop capture\n");

	return 0;
}

int camera_exit(int fd)
{
	int i;
	int ret;
	struct v4l2_buffer vbuf;
	vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbuf.memory = V4L2_MEMORY_MMAP;
	for (i = 0; i < reqbufs.count; i++) {
		ret = ioctl(fd, VIDIOC_DQBUF, &vbuf);
		if (ret == -1)
			break;
	}
	for (i = 0; i < reqbufs.count; i++)
		munmap(bufs[i].start, bufs[i].length);
	fprintf(stdout, "camera->exit: camera exit\n");
	return close(fd);
}

int camera_on(void)
{
	int ret;
	int i;
	int fd;
	unsigned int width;
	unsigned int height;
	unsigned int size;
	unsigned int index;
	char *yuv;
	char *rgb;

	/*设置分辨率*/
	width = W;
	height = H;
	
	/*初始化摄像头*/
	fd = camera_init(CAMERA_USB, &width, &height, &size);
	if (fd == -1)
	{
		return fd;
	}
	
	while(!Exitflag)
	{
		if(Stopflag)
		{
			jpg->jpg_size=0;
			sleep(1);
			continue;
		}
		/*摄像头开启*/
		ret = camera_start(fd);
		if (ret == -1)
		{
			return fd;
		}
	#ifndef VIDEO_FORMAT_MJPEG 
		convert_rgb_to_jpg_init();
		rgb = malloc(width*height*3);
		if (rgb == NULL) {
			perror("malloc");
			return -1;
		}
	#endif
		/*前几张图片可能有问题，需要采集几张图片丢弃，确保收到的是稳定后的图片*/
		for (i = 0; i < 8; i++) {
			ret = camera_dqbuf(fd, (void **)&yuv, &size, &index);
			if (ret == -1)
				exit(EXIT_FAILURE);

			ret = camera_eqbuf(fd, index);
			if (ret == -1)
				exit(EXIT_FAILURE);
		}

		fprintf(stdout, "init camera success\n");
		
		/* 循环采集图片 */
		while (!Stopflag) {
			/*从队列中取出图片缓冲*/
			ret = camera_dqbuf(fd, (void **)&yuv, &size, &index);
			if (ret == -1)
			{
				return ret;
			}

	#ifdef VIDEO_FORMAT_MJPEG
				memcpy(jpg->jpg_buf, yuv, size);
				jpg->jpg_size = size;
	#else
				convert_yuv_to_rgb(yuv, rgb, width, height, 24)	;
				/* 将rgb转换为jpg */
				jpg->jpg_size = convert_rgb_to_jpg_work(rgb, jpg->jpg_buf, width, height, 24, 80);
			}
	#endif

			/*把缓冲重新入队*/
			ret = camera_eqbuf(fd, index);
			if (ret == -1)
				return -1;
			
		}


		ret = camera_stop(fd);
		if (ret == -1)
		{
			return -1;
		}
	}
	

	ret = camera_exit(fd);
	if (ret == -1)
	{
		return -1;
	}
	return 0;
}
