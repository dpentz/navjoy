/* joynav - create joystick from a spacenav 6dof device */
/*Copyright (c) 2015 Damon Pentz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to 
deal in the Software without restriction, including without limitation the 
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
IN THE SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stropts.h>
#include <stdarg.h>
#include <signal.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <spnav.h>

#define FIRST_BUTTON_ID BTN_JOYSTICK
#define NUM_BUTTONS_TOTAL 21

/* levels of logging */
enum log_level {
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
};
enum log_level plog_level;

/* setup flags */
int daemonize;
char *uinput_device;

/* pid of process */
int pid;

/* uinput file descriptor */
int fd;

/* spacenavd event */
spnav_event sev;

/* input event */
struct input_event ev;

/* error output to stdout */
void
plog(enum log_level level, const char *format, ...);

/* parse commandline options */
void 
cmdlineopts(int argc, char **argv);

/* basic initialization*/
int
init();

/* initialize joystick */
int
joy_init();

/* called before exit() */
void
cleanup(int sig);

int
main(int argc, char **argv)
{
	int btn_id;

	/* handle quit signals */
	signal(SIGINT, cleanup);
	signal(SIGHUP, cleanup);

	cmdlineopts(argc, argv);

	if(init() != 0) {
		plog(LOG_ERROR,"Initialization failed");
		exit(EXIT_FAILURE);
	}
	
	while(spnav_wait_event(&sev)) {
		switch(sev.type) {
			case SPNAV_EVENT_MOTION:
				if(sev.type == SPNAV_EVENT_MOTION) {
				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_ABS;
				ev.code = ABS_X;
				ev.value = sev.motion.x;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}

				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_ABS;
				ev.code = ABS_Y;
				ev.value = sev.motion.y;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}

				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_ABS;
				ev.code = ABS_Z;
				ev.value = sev.motion.z;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}

				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_ABS;
				ev.code = ABS_RX;
				ev.value = sev.motion.rx;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}

				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_ABS;
				ev.code = ABS_RY;
				ev.value = sev.motion.ry;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}

				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_ABS;
				ev.code = ABS_RZ;
				ev.value = sev.motion.rz;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}

				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_SYN;
				ev.code = 0;
				ev.value = 0;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}
				break;
			case SPNAV_EVENT_BUTTON:
				btn_id = sev.button.bnum;
				if (btn_id < NUM_BUTTONS_TOTAL) {
					memset(&ev, 0, sizeof(struct input_event));
					ev.type = EV_KEY;
					ev.code = FIRST_BUTTON_ID + btn_id;
					ev.value = sev.button.press;
					if(write(fd, &ev, sizeof(struct input_event)) < 0) {
						plog(LOG_ERROR, "Could not write event");
					}
				}
				else {
					plog(LOG_ERROR, "Default Event for button %d", btn_id);
				}
				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_SYN;
				ev.code = 0;
				ev.value = 0;
				if(write(fd, &ev, sizeof(struct input_event)) < 0) {
					plog(LOG_ERROR, "Could not write event");
				}
				break;
			default:
				plog(LOG_ERROR, "Should not get here");
			}
		}
	}
	cleanup(EXIT_SUCCESS);
	return 0;
}

void
cmdlineopts(int argc, char **argv)
{
	int c;
	int level = 0;
	
	/* default options */
	daemonize = 0;
	plog_level = 0;
	uinput_device = strdup("/dev/uinput");

	while (1) {
		int option_index = -1;
		static struct option long_options[] = {
			{"daemon",  no_argument,       0, 'd'},
			{"logging", required_argument, 0, 'l'},
			{"uinput" , required_argument, 0, 'u'},
			{        0,                 0, 0,   0}
		};

		c = getopt_long(argc, argv, "dl:u:", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			case 'd':
				daemonize = 1;
				break;
			case 'l':
				level = atoi(optarg);
				if(level < 0 || level > 3) {
					plog(LOG_ERROR, "invalid log level %d, setting to 0", level);
					plog(LOG_ERROR, "0 for fatal, 1 for warnings 2 for information 3 for debug");
				} else {
					plog_level = level;
				}
				break;
			case 'u':
				uinput_device = strdup(optarg);
				plog(LOG_ERROR, "udevice is %s", uinput_device);
				break;
			default:
				plog(LOG_ERROR, "invalid option");
				break;
        }
    }
}

int
init()
{
	/* start a non-X11 connection to spacenavd */
	if(spnav_open() == -1) {
		plog(LOG_ERROR, "Could not connect to spacenavd. Is it running?");
		cleanup(EXIT_FAILURE);
	}

	/* create uinput device */
	if(joy_init()) {
		plog(LOG_ERROR, "Could not create joystick device on %s", uinput_device);
		cleanup(EXIT_FAILURE);
	}
	
	/* create daemon if needed */
	if(daemonize) {
		pid = fork();
		if(pid < 0) {
			plog(LOG_ERROR, "Could not fork...exiting");
			cleanup(EXIT_FAILURE);
		}
		if(pid > 0) {
			exit(EXIT_SUCCESS);
		}
		/* we're now in the child */
	}
	return 0;
}

int
joy_init()
{
	/* joystick information */
	struct uinput_user_dev joystick;
	memset(&joystick, 0, sizeof(joystick));
	snprintf(joystick.name, UINPUT_MAX_NAME_SIZE, "Spacenav Joystick");
	joystick.id.bustype = BUS_USB;
	joystick.id.vendor = 0xDEAD;
	joystick.id.product = 0xBEEF;
	joystick.id.version = 1;
	joystick.absmin[ABS_X] 	= -350;
	joystick.absmax[ABS_X] 	=  350;
	joystick.absmin[ABS_Y] 	= -350;
	joystick.absmax[ABS_Y] 	=  350;
	joystick.absmin[ABS_Z] 	= -350;
	joystick.absmax[ABS_Z] 	=  350;
	joystick.absmin[ABS_RX] = -350;
	joystick.absmax[ABS_RX] =  350;
	joystick.absmin[ABS_RY] = -350;
	joystick.absmax[ABS_RY] =  350;
	joystick.absmin[ABS_RZ] = -350;
	joystick.absmax[ABS_RZ]	=  350;

	fd = open(uinput_device, O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		plog(LOG_ERROR, "Could not open %s", uinput_device);
		plog(LOG_ERROR, "Do you have the correct path?");
		cleanup(EXIT_FAILURE);
	}
	write(fd, &joystick, sizeof(joystick));

	/* create 6 axis joystick with two buttons */
	if(ioctl(fd, UI_SET_EVBIT, EV_ABS) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}
	
	if(ioctl(fd, UI_SET_ABSBIT, ABS_X) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}
	
	if(ioctl(fd, UI_SET_ABSBIT, ABS_Y) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}
	
	if(ioctl(fd, UI_SET_ABSBIT, ABS_Z) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}
	if(ioctl(fd, UI_SET_ABSBIT, ABS_RX) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}
	
	if(ioctl(fd, UI_SET_ABSBIT, ABS_RY) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}
	
	if(ioctl(fd, UI_SET_ABSBIT, ABS_RZ) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}

	if(ioctl(fd, UI_SET_EVBIT, EV_KEY) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}

	int i;
	for ( i = 0; i < NUM_BUTTONS_TOTAL; i++ ) {
		if(ioctl(fd, UI_SET_KEYBIT, FIRST_BUTTON_ID + i) == -1) {
			plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
			cleanup(EXIT_FAILURE);
		}
	}

	if(ioctl(fd, UI_SET_EVBIT, EV_SYN) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}

	if(ioctl(fd, UI_DEV_CREATE) == -1) {
		plog(LOG_ERROR, "Error with ioctl: %s", strerror(errno));
		cleanup(EXIT_FAILURE);
	}

	return 0;
}


void
cleanup(int sig)
{
	/* don't care about error handling here */
	spnav_close();
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);
	exit(sig);
}

void
plog(enum log_level level, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	if(plog_level < level)
		return;
	switch(level){
		case LOG_ERROR:
			printf("ERROR: ");
			vprintf(format, args);
			printf("\n");
			break;
		case LOG_WARN:
			printf("WARNING: ");
			vprintf(format, args);
			printf("\n");
			break;
		case LOG_INFO:
			printf("INFO: ");
			vprintf(format, args);
			printf("\n");
			break;
		case LOG_DEBUG:
			printf("DEBUG: ");
			vprintf(format,args);
			printf("\n");
			break;
		default:
			break;
	}

	fflush(stdout);
	va_end(args);
}
