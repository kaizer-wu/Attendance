#ifndef __CAM_H__
#define	__CAM_H__

#define JPG_MAX_SIZE	(1024 * 1024 - sizeof (unsigned int))
#define SIZE 16

#define CAMERA_USB "/dev/video0"
#define W 640
#define H 480
#define IP "192.168.137.1"
#define PORT 8888

//#define VIDEO_FORMAT_MJPEG

struct jpg_buf_t {
	char jpg_buf[JPG_MAX_SIZE];
	unsigned int jpg_size;
};


int camera_on();
#endif
