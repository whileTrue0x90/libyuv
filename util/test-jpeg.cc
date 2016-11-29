/**
 * test-jpeg.c
 *
 * Copyright (c) 2011-2014 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation
 *
 * 2014, Samsung Electronics Co., Ltd.
 * ./test-perf -f test1.jpg -oout.raw -w1080 -h720
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/eventfd.h>

#include <linux/fb.h>
#include <linux/videodev2.h>

#include <sys/mman.h>

#include "libyuv.h"

#define MTK_JPEG_NOT_SUPP // added by minghsiu
//#define MTK_JPEG_EMPTY_EVENT // added by minghsiu
//#define BENCHMARK_NOMEMCPY // added by minghsiu

#define VIDEO_DEV_NAME	"/dev/jpeg-dec"

#define perror_exit(cond, func)\
	if (cond) {\
		fprintf(stderr, "%s:%d: ", __func__, __LINE__);\
		perror(func);\
		exit(EXIT_FAILURE);\
	}

#define perror_ret(cond, func)\
	if (cond) {\
		fprintf(stderr, "%s:%d: ", __func__, __LINE__);\
		perror(func);\
		return ret;\
	}

#define memzero(x)\
	memset(&(x), 0, sizeof (x));


//#define PROCESS_DEBUG
#ifdef PROCESS_DEBUG
#define debug(msg, ...)\
	fprintf(stderr, "%s: \n" msg, __func__, ##__VA_ARGS__);
#else
#define debug(msg, ...)
#endif

#define ENCODE 0
#define DECODE 1

enum pix_format {
	FMT_JPEG,
        FMT_RGB565,
        FMT_RGB565X,
        FMT_RGB32,
        FMT_BGR32,
        FMT_YUYV,
        FMT_YVYU,
        FMT_UYVY,
        FMT_VYUY,
        FMT_NV24,
        FMT_NV42,
        FMT_NV16,
        FMT_NV61,
        FMT_NV12,
        FMT_NV21,
        FMT_YUV420,
        FMT_GREY,
};

struct jpegfile {
  uint8* p_input_file;
  uint8* p_output_buffer;
  char input_filename[100];
  char output_filename[100];
  int input_file_sz;
  int input_fd;
  int output_buffer_sz;
};

struct jpegfile jpegfiles[10];
static int num_files = 0;

static int vid_fd, event_fd;
static uint8 *p_src_buf = NULL, *p_dst_buf = NULL;
static int src_buf_size, dst_buf_size;
static int capture_buffer_sz = 0;
int input_stream_on = 0, output_stream_on = 0;
int width, height;

/* Command-line params */
int mode = DECODE;
const char *input_filename;
const char *output_filename;
int fourcc = FMT_YUV420;
int software_decode = 0;
int userptr_decode = 0;
int frame = 1;

static void get_format_name_by_fourcc(unsigned int fourcc, char *fmt_name)
{
  switch (fourcc) {
    case V4L2_PIX_FMT_JPEG:
      strcpy(fmt_name, "V4L2_PIX_FMT_JPEG");
      break;
    case V4L2_PIX_FMT_YUV420:
      strcpy(fmt_name, "V4L2_PIX_FMT_YUV420");
      break;
    default:
      strcpy(fmt_name, "UNKNOWN");
      break;
  }
}

#ifndef MTK_JPEG_NOT_SUPP
static void get_subsampling_by_id(int subs, char *subs_name)
{
	switch (subs) {
	case V4L2_JPEG_CHROMA_SUBSAMPLING_444:
		strcpy(subs_name, "JPEG 4:4:4");
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_422:
		strcpy(subs_name, "JPEG 4:2:2");
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_420:
		strcpy(subs_name, "JPEG 4:2:0");
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_411:
		strcpy(subs_name, "JPEG 4:1:1");
		break;
	case V4L2_JPEG_CHROMA_SUBSAMPLING_GRAY:
		strcpy(subs_name, "GRAY SCALE JPEG");
		break;
	default:
		sprintf(subs_name, "Unknown JPEG subsampling: %d", subs);
		break;
	}
}
#endif

void init_input_files(void)
{
	struct stat stat;
  int i;
  for (i = 0; i < num_files; i++) {
    jpegfiles[i].input_fd = open(jpegfiles[i].input_filename, O_RDONLY);
    perror_exit(jpegfiles[i].input_fd < 0, "open");
    fstat(jpegfiles[i].input_fd, &stat);
    jpegfiles[i].input_file_sz = stat.st_size;
    printf("input_file size: %d\n", jpegfiles[i].input_file_sz);

    jpegfiles[i].p_input_file = (uint8 *)mmap(
        NULL, jpegfiles[i].input_file_sz, PROT_READ , MAP_SHARED, jpegfiles[i].input_fd, 0);
  }
}


void print_usage (void)
{
	fprintf (stderr, "Usage:\n"
		 "-f[INPUT FILE NAME]\n"
		 "-w[INPUT IMAGE WIDTH]\n"
		 "-h[INPUT IMAGE HEIGHT]\n");
}

static int parse_args(int argc, char *argv[])
{
	int c;

	opterr = 0;
	while ((c = getopt (argc, argv, "usf:w:h:n:")) != -1) {
		debug("c: %c, optarg: %s\n", c, optarg);
		switch (c) {
    case 'u':
      userptr_decode = 1;
      break;
    case 's':
      software_decode = 1;
      break;
		case 'f':
      char *p;
      for (p = strtok(optarg, ","); p; p = strtok(NULL, ",")) {
        strcpy(jpegfiles[num_files].input_filename, p);
        strncpy(jpegfiles[num_files].output_filename, p, strlen(p) - 4);
        strcat(jpegfiles[num_files].output_filename, ".yuv");
        num_files++;
      }
			break;
    case 'w':
      width = atoi(optarg);
      break;
    case 'h':
      height = atoi(optarg);
      break;
		case 'n':
			frame = atoi(optarg);
			break;
		case '?':
			print_usage ();
			return -1;
		default:
			fprintf (stderr,
				 "Unknown option character `\\x%x'.\n",
				 optopt);
			return -1;
		}
	}

	return 0;
}

int
timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

int showtime()
{
  static int start = 0;
  static struct timeval prev, now, diff;
  if (start == 0) {
    start = 1;
    gettimeofday(&prev, NULL);
    return 0;
  } else {
    gettimeofday(&now, NULL);
    timeval_subtract(&diff, &now, &prev);
    memcpy(&prev, &now, sizeof(struct timeval));
    return diff.tv_sec * 1000000 + diff.tv_usec;
  }
}

int mypoll(int device_fd) {
  struct pollfd pollfds[2];
  nfds_t nfds;
#ifdef MTK_JPEG_EMPTY_EVENT
  short inout;
  bool poll_again = false;
#endif

  pollfds[0].fd = event_fd;
  pollfds[0].events = POLLIN | POLLERR;
  nfds = 1;

  pollfds[nfds].fd = device_fd;
  pollfds[nfds].events = POLLIN | POLLOUT | POLLERR | POLLPRI;
  nfds++;
#ifdef MTK_JPEG_EMPTY_EVENT
  do {
    if (poll(pollfds, nfds, -1) == -1) {
      fprintf(stderr, "poll() failed\n");
      return 0;
    }
    debug("poll revents: %X\n", pollfds[nfds-1].revents);

    inout = pollfds[nfds-1].revents & (POLLIN | POLLOUT);
    if (!inout || inout == (POLLIN | POLLOUT) || poll_again)
      break;
    poll_again = true;
    pollfds[nfds-1].events &= ~(POLLIN | POLLOUT) | ~inout; // poll another direction
  } while(poll_again);
#else
  if (poll(pollfds, nfds, -1) == -1) {
    fprintf(stderr, "poll() failed\n");
    return 0;
  }
#endif
  debug("poll revents: %X\n", pollfds[nfds-1].revents);
  return (pollfds[nfds-1].revents & POLLPRI);
}

void hardware_init_dev()
{
	struct v4l2_capability cap;
	char video_node_name[20];
  int ret;

	strcpy(video_node_name, VIDEO_DEV_NAME);
  printf("Device node: %s\n", video_node_name);
	vid_fd = open(video_node_name, O_RDWR | O_NONBLOCK | O_CLOEXEC, 0);
	perror_exit(vid_fd < 0, "open");
  event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd == -1) {
    fprintf(stderr, "Open event fd fail\n");
    exit(EXIT_FAILURE);
  }

	ret = ioctl(vid_fd, VIDIOC_QUERYCAP, &cap);
	perror_exit(ret != 0, "ioctl");

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "Device does not support capture\n");
		exit(EXIT_FAILURE);
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
		fprintf(stderr, "Device does not support output\n");
		exit(EXIT_FAILURE);
	}

  {
    struct v4l2_event_subscription sub;
    memset(&sub, 0, sizeof(sub));
    sub.type = V4L2_EVENT_SOURCE_CHANGE;
    ret = ioctl(vid_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    perror_exit(ret != 0, "ioctl");
  }
}

void hardware_input_set_format(int file_index)
{
	struct v4l2_format fmt;
  int ret;
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width	= 0;
	fmt.fmt.pix.height = 0;
	fmt.fmt.pix.sizeimage	= jpegfiles[file_index].input_file_sz * 2;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	fmt.fmt.pix.field	= V4L2_FIELD_ANY;
	fmt.fmt.pix.bytesperline = 0;

	ret = ioctl(vid_fd, VIDIOC_S_FMT, &fmt);
	perror_exit(ret != 0, "ioctl");
}

void hardware_output_set_format()
{
	struct v4l2_format fmt;
  int ret;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.sizeimage	= width * height * 2;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix.field	= V4L2_FIELD_ANY;

	char fmt_name[30];
	get_format_name_by_fourcc(fmt.fmt.pix.pixelformat, fmt_name);

	ret = ioctl(vid_fd, VIDIOC_S_FMT, &fmt);
	perror_exit(ret != 0, "ioctl");

	get_format_name_by_fourcc(fmt.fmt.pix.pixelformat, fmt_name);
	debug("output format set: %s\n", fmt_name);
	debug("output image dimensions: %dx%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
  width = fmt.fmt.pix.width;
  height = fmt.fmt.pix.height;
}

void hardware_input_buffer(enum v4l2_memory memory)
{
	struct v4l2_requestbuffers reqbuf;
  int ret;

	memzero(reqbuf);
	reqbuf.count	= 1;
	reqbuf.type	= V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory	= memory;

	ret = ioctl(vid_fd, VIDIOC_REQBUFS, &reqbuf);
	perror_exit(ret != 0, "ioctl");
	debug("num_src_bufs: %d\n", reqbuf.count);

  if (memory == V4L2_MEMORY_MMAP) {
    struct v4l2_buffer buf;
    memzero(buf);
    buf.type	= V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory	= memory;
    buf.index	= 0;

    ret = ioctl(vid_fd, VIDIOC_QUERYBUF, &buf);
    perror_exit(ret != 0, "ioctl");

    src_buf_size = buf.length;
    /* mmap buffer */
    p_src_buf = (uint8 *)mmap(NULL, buf.length,
        PROT_READ | PROT_WRITE, MAP_SHARED,
        vid_fd, buf.m.offset);
    perror_exit(MAP_FAILED == p_src_buf, "mmap");
  } else if (memory == V4L2_MEMORY_USERPTR) {
    src_buf_size = jpegfiles[0].input_file_sz * 2;
    p_src_buf = (uint8 *)aligned_alloc(4096, src_buf_size);
		debug("p_src_buf: %p, size: %d\n", p_src_buf, src_buf_size);
  }
}

void hardware_output_buffer(enum v4l2_memory memory)
{
	struct v4l2_requestbuffers reqbuf;
  int ret;

	memzero(reqbuf);
	reqbuf.count = 1;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory	= memory;

	ret = ioctl(vid_fd, VIDIOC_REQBUFS, &reqbuf);
	perror_exit(ret != 0, "ioctl");
	debug("num_dst_bufs: %d\n", reqbuf.count);

  if (memory == V4L2_MEMORY_MMAP) {
    struct v4l2_buffer buf;
    /* query buffer parameters */
    memzero(buf);
    buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory	= memory;
    buf.index	= 0;

    ret = ioctl(vid_fd, VIDIOC_QUERYBUF, &buf);
    perror_exit(ret != 0, "ioctl");

    /* mmap buffer */
    dst_buf_size = buf.length;
    p_dst_buf = (uint8 *)mmap(NULL, buf.length,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            vid_fd, buf.m.offset);
    perror_exit(MAP_FAILED == p_dst_buf, "mmap");
  } else if (memory == V4L2_MEMORY_USERPTR) {
    dst_buf_size = width * height * 1.5;
    p_dst_buf = (uint8 *)aligned_alloc(4096, dst_buf_size);
  }

  int i;
  for (i = 0; i < num_files; i++) {
    jpegfiles[i].p_output_buffer = (uint8 *)aligned_alloc(4096, dst_buf_size);
  }
}

void hardware_input_destroy(enum v4l2_memory memory)
{
  int ret;
  if (input_stream_on) {
    __u32 type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(vid_fd, VIDIOC_STREAMOFF, &type);
    perror_exit(ret != 0, "ioctl");
    input_stream_on = 0;
  }

  struct v4l2_requestbuffers reqbuf;
  memzero(reqbuf);
  reqbuf.type	= V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.memory	= memory;
  ret = ioctl(vid_fd, VIDIOC_REQBUFS, &reqbuf);
  perror_exit(ret != 0, "ioctl");

  if (memory == V4L2_MEMORY_MMAP && src_buf_size > 0)
    munmap(p_src_buf, src_buf_size);
  src_buf_size = 0;
  p_src_buf = NULL;
}

void hardware_output_destroy(enum v4l2_memory memory)
{
  int ret;
  if (output_stream_on) {
    __u32 type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(vid_fd, VIDIOC_STREAMOFF, &type);
    perror_exit(ret != 0, "ioctl");
    output_stream_on = 0;
  }

  struct v4l2_requestbuffers reqbuf;
  memzero(reqbuf);
  reqbuf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory	= memory;
  ret = ioctl(vid_fd, VIDIOC_REQBUFS, &reqbuf);
  perror_exit(ret != 0, "ioctl");

  if (memory == V4L2_MEMORY_MMAP && dst_buf_size > 0)
    munmap(p_dst_buf, dst_buf_size);
  dst_buf_size = 0;
  p_dst_buf = NULL;
}

void hardware_input_check(int file_index, enum v4l2_memory memory)
{
  if(jpegfiles[file_index].input_file_sz > src_buf_size) {
    hardware_input_destroy(memory);
    hardware_input_set_format(file_index);
    hardware_input_buffer(memory);
  }
}

void hardware_output_check(enum v4l2_memory memory)
{
	struct v4l2_format fmt;
  int ret;
  memzero(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(vid_fd, VIDIOC_G_FMT, &fmt);
  perror_exit(ret != 0, "ioctl");
  debug("=========================================\n");
  debug("input dimension: %dx%d, format: %X\n",
      fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);
  debug("=========================================\n");
	char fmt_name[30];
	get_format_name_by_fourcc(fmt.fmt.pix.pixelformat, fmt_name);
	debug("expect output format: %s\n", fmt_name);

  width = fmt.fmt.pix.width;
  height = fmt.fmt.pix.height;
  if(width * height * 2 > dst_buf_size) {
    hardware_output_destroy(memory);
    hardware_output_set_format();
    hardware_output_buffer(memory);
  }
}

void hardware_input_qbuf(enum v4l2_memory memory, int input_size)
{
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;
  int ret;

  memzero(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buf.memory = memory;
  buf.index	= 0;
  if (memory == V4L2_MEMORY_USERPTR) {
    buf.m.userptr = (unsigned long)p_src_buf;
    buf.length = input_size;
    buf.bytesused = input_size;
  }

  ret = ioctl(vid_fd, VIDIOC_QBUF, &buf);
  perror_exit(ret != 0, "ioctl");

  if (!input_stream_on) {
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(vid_fd, VIDIOC_STREAMON, &type);
    perror_exit(ret != 0, "ioctl");
    input_stream_on = 1;
  }
}

void hardware_output_qbuf(enum v4l2_memory memory)
{
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;
  int ret;

  if (!p_dst_buf)
    return;

  memzero(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = memory;
  buf.index	= 0;
  if (memory == V4L2_MEMORY_USERPTR) {
    buf.m.userptr = (unsigned long)p_dst_buf;
    buf.length = dst_buf_size;
    buf.bytesused = dst_buf_size;
  }

  ret = ioctl(vid_fd, VIDIOC_QBUF, &buf);
  perror_exit(ret != 0, "ioctl");

  if (!output_stream_on) {
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(vid_fd, VIDIOC_STREAMON, &type);
    perror_exit(ret != 0, "ioctl");
    output_stream_on = 1;
  }
}

int hardware_input_dqbuf(enum v4l2_memory memory)
{
	struct v4l2_buffer buf;
	int ret;

	memzero(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buf.memory = memory;
	ret = ioctl(vid_fd, VIDIOC_DQBUF, &buf);
	debug("Dequeued source buffer, index: %d\n", buf.index);
  debug("source ret: %d, byteused: %d, flag: %X, error: %d\n",
      ret, buf.bytesused, buf.flags, errno);
	if (ret) {
		switch (errno) {
		case EAGAIN:
#ifdef MTK_JPEG_EMPTY_EVENT
			fprintf(stderr, "%s: \n", "Got EAGAIN\n");
			return -1;
#else
			debug("Got EAGAIN\n");
			return 0;
#endif
		case EIO:
			debug("Got EIO\n");
			return 0;
		default:
			perror("ioctl");
			return 0;
		}
	}
	return 0;
}

int hardware_output_dqbuf(enum v4l2_memory memory)
{
	struct v4l2_buffer buf;
	int ret;

	memzero(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = memory;
	ret = ioctl(vid_fd, VIDIOC_DQBUF, &buf);
	debug("Dequeued dst buffer, index: %d\n", buf.index);
  debug("dst ret: %d, byteused: %d, flag: %X, error: %d\n",
      ret, buf.bytesused, buf.flags, errno);
	if (ret) {
		switch (errno) {
		case EAGAIN:
#ifdef MTK_JPEG_EMPTY_EVENT
			fprintf(stderr, "%s: \n", "Got EAGAIN\n");
			return -1;
#else
			debug("Got EAGAIN\n");
			return 0;
#endif
		case EIO:
			debug("Got EIO\n");
			return 0;
		default:
			perror("ioctl");
			return 1;
		}
	}

	capture_buffer_sz = buf.bytesused;
	return 0;
}

void hardware_dequeue_event()
{
  struct v4l2_event ev;
  int ret;
  memset(&ev, 0, sizeof(ev));
  ret = ioctl(vid_fd, VIDIOC_DQEVENT, &ev);
  perror_exit(ret != 0, "ioctl");
}

int hardware_decoder(enum v4l2_memory memory)
{
  int loop;
  int latency = 0;
  int total_latency = 0;
  enum v4l2_memory input_memory = V4L2_MEMORY_MMAP;
  enum v4l2_memory output_memory = memory;
  int ret;

  hardware_init_dev();

  for (loop = 0; loop < frame; loop++) {
    int file_index = loop % num_files;

    showtime();

    hardware_input_check(file_index, input_memory);

    /* copy input file data to the buffer */
    memcpy(p_src_buf, jpegfiles[file_index].p_input_file, jpegfiles[file_index].input_file_sz);
    hardware_input_qbuf(input_memory, jpegfiles[file_index].input_file_sz);
#ifdef MTK_JPEG_EMPTY_EVENT
    if (!dst_buf_size)
      hardware_output_buffer(output_memory); // added by minghsiu
#endif
    hardware_output_qbuf(output_memory);

    ret = mypoll(vid_fd);
    if (ret) {
      hardware_dequeue_event();
      hardware_output_check(output_memory);
      hardware_output_qbuf(output_memory);
      ret = mypoll(vid_fd);
    }

    if (hardware_input_dqbuf(input_memory) || hardware_output_dqbuf(output_memory)) {
      fprintf(stderr, "dequeue frame failed\n");
      return -1;
    }
    jpegfiles[file_index].output_buffer_sz = capture_buffer_sz;

#ifdef BENCHMARK_NOMEMCPY
#else
    memcpy(jpegfiles[file_index].p_output_buffer, p_dst_buf, capture_buffer_sz);
#endif
    latency = showtime();

    total_latency += latency;
    if (1000000 / 30 - latency > 0)
      usleep(1000000 / 30 - latency);
  }
  fprintf(stderr, "=== Average latency: %lf ===\n", total_latency / frame / 1000.0);

#ifndef MTK_JPEG_NOT_SUPP
  struct v4l2_control ctrl;
  char subs_name[50];

  /* get input JPEG subsampling info */
  memzero(ctrl);
  ctrl.id = V4L2_CID_JPEG_CHROMA_SUBSAMPLING;

  ret = ioctl (vid_fd, VIDIOC_G_CTRL, &ctrl);
  perror_exit (ret != 0, "VIDIOC_G_CTRL v4l2_ioctl");

  get_subsampling_by_id(ctrl.value, subs_name);
  printf("decoded JPEG subsampling: %s\n", subs_name);

  /* get active area of decoded image */
  struct v4l2_selection sel2 = {
    .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
    .target = V4L2_SEL_TGT_COMPOSE_BOUNDS,
  };

  ret = ioctl(vid_fd, VIDIOC_G_SELECTION, &sel2);
  perror_exit(ret != 0, "ioctl");
  printf("active area dimensions: %dx%d\n", sel2.r.width, sel2.r.height);
#endif
  return 0;
}

int software_decoder()
{
  int loop;
  int latency = 0;
  int total_latency = 0;
  uint8 *yplane, *uplane, *vplane;
  int yplane_stride, uv_plane_stride;
  int i, sleep_time;

  dst_buf_size = width * height * 1.5;

  for (i = 0; i < num_files; i++) {
    jpegfiles[i].p_output_buffer = (uint8 *)aligned_alloc(16, dst_buf_size);
  }

  yplane_stride = width;
  uv_plane_stride = yplane_stride / 2;

  for (loop = 0; loop < frame; loop++) {
//    if ((loop + 1) % 100 == 0)
//      printf("frame: %d\n", loop + 1);
    int file_index = loop % num_files;

    yplane = jpegfiles[file_index].p_output_buffer;
    uplane = yplane + width * height;
    vplane = uplane + width * height / 4;
    showtime();

    if (libyuv::ConvertToI420((uint8 *)jpegfiles[file_index].p_input_file,
          jpegfiles[file_index].input_file_sz,
          yplane,
          yplane_stride,
          uplane,
          uv_plane_stride,
          vplane,
          uv_plane_stride,
          0,
          0,
          width,
          height,
          width,
          height,
          libyuv::kRotate0,
          libyuv::FOURCC_MJPG) != 0) {
      printf("fail width: %d, height: %d\n", width, height);
      return -1;
    }
    jpegfiles[file_index].output_buffer_sz = dst_buf_size;
    latency = showtime();

    total_latency += latency;
    sleep_time = 1000000 / 30 - latency;
    if (sleep_time > 0)
      usleep(sleep_time);
//    if ((loop + 1) % 100 == 0)
//      printf("end frame: %d, latency: %d\n", loop+1, latency);
  }
  fprintf(stderr, "=== Average latency: %lf, loop: %d ===\n", total_latency / frame/ 1000.0, loop);
  return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int ret = 0;
  int output = 1;

	ret = parse_args(argc, argv);
	if (ret < 0)
		return 0;

	init_input_files();

  if (!software_decode) {
    if (userptr_decode) {
      ret = hardware_decoder(V4L2_MEMORY_USERPTR);
    } else {
      ret = hardware_decoder(V4L2_MEMORY_MMAP);
    }
  } else {
    ret = software_decoder();
  }

  if (ret == 0 && output) {
    /* generate output file */
    printf("Generating output file...\n");
    for (i = 0; i < num_files; i++) {
      int out_fd = open(jpegfiles[i].output_filename,
          O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      ret = write(out_fd, jpegfiles[i].p_output_buffer, jpegfiles[i].output_buffer_sz);
      close(out_fd);
      printf("Output file: %s, size: %d\n",
          jpegfiles[i].output_filename, jpegfiles[i].output_buffer_sz);
    }
  }

  for (i = 0; i < num_files; i++) {
    close(jpegfiles[i].input_fd);
    munmap(jpegfiles[i].p_input_file, jpegfiles[i].input_file_sz);
    free(jpegfiles[i].p_output_buffer);
  }
  if (!software_decode && !userptr_decode) {
    munmap(p_src_buf, src_buf_size);
    munmap(p_dst_buf, dst_buf_size);
  }
	close(vid_fd);

//  while (1) {
//    printf("== busy loop ==\n");
//    sleep(1);
//  }
	return ret;
}

