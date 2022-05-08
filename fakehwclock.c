#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <endian.h>
#include <stdint.h>
#include <time.h>

enum { FHWC_READ, FHWC_LOAD, FHWC_SAVE };
enum { FHWC_UTC = 1, FHWC_LOCAL };
static char *svclockf = "/etc/fakehwclock.sav";
static uint16_t direction = FHWC_READ;
static uint16_t clockset;

struct fakehwclock {
	uint32_t fhwc_dir;
	uint16_t fhwc_utc;
	struct timespec fhwc_tsp;
	uint8_t __pad[10];
} __attribute__((packed));

struct fakehwclock fhwc;
struct tm *fhwc_tm;
static char data[256];

int main(int argc, char **argv)
{
	int c, fd = -1;
	char *s, *d;
	time_t t;
	struct timespec tsp;
	int optindex = 0;
	static struct option longopts[] = {
		{"show",	no_argument,		0,	'r'},
		{"get",		no_argument,		0,	'r'},
		{"set",		no_argument,		0,	's'},
		{"hctosys",	no_argument,		0,	's'},
		{"systohc",	no_argument,		0,	'w'},
		{"systz",	no_argument,		0,	 0 },
		{"adjust",	no_argument,		0,	'a'},
		{"predict",	no_argument,		0,	 0 },
		{"utc",		no_argument,		0,	'u'},
		{"localtime",	no_argument,		0,	'l'},
		{"rtc",		required_argument,	0,	'f'},
		{"directisa",	no_argument,		0,	 0 },
		{"date",	required_argument,	0,	 0 },
		{"delay",	required_argument,	0,	 0 },
		{"update-drift",required_argument,	0,	 0 },
		{"noadjfile",	no_argument,		0,	 0 },
		{"adjfile",	required_argument,	0,	 0 },
		{"test",	no_argument,		0,	'v'},
		{"verbose",	no_argument,		0,	'v'},
		{0,		0,			0,	 0 },
	};

	while ((c = getopt_long(argc, argv, "arwsulf:v", longopts, &optindex)) != -1) {
		switch (c) {
			case 'u': clockset = FHWC_UTC; break;
			case 'l': clockset = FHWC_LOCAL; break;
			case 'f': svclockf = optarg; break;
			case 'r': direction = FHWC_READ; break;
			case 'w': direction = FHWC_SAVE; break;
			case 's': direction = FHWC_LOAD; break;
			/* ignored ones */
			default: break;
		}
	}

	switch (direction) {
		case FHWC_READ:
		case FHWC_LOAD:
			fd = open(svclockf, O_RDONLY);
			if (fd == -1) {
				perror(svclockf);
				return errno;
			}
			if (read(fd, &fhwc, sizeof(fhwc)) < sizeof(fhwc)) {
				errno = ERANGE;
				perror(svclockf);
				return errno;
			}
			close(fd);
			if (fhwc.fhwc_dir != 0x66687701) { /* wrong endian */
				errno = ENOMSG;
				perror(svclockf);
				return errno;
			}
			if (!clockset) clockset = fhwc.fhwc_utc;
			t = fhwc.fhwc_tsp.tv_sec;
			if (clockset == FHWC_UTC) fhwc_tm = gmtime(&t);
			else if (clockset == FHWC_LOCAL) fhwc_tm = localtime(&t);

			if (direction == FHWC_READ) {
				s = data;
				strftime(s, sizeof(data)/2, "%Y-%m-%d %H:%M:%S", fhwc_tm);
				d = data+(sizeof(data)/2);
				strftime(d, sizeof(data)/2, "%z", fhwc_tm);
				printf("%s.%llu%s\n", s, (unsigned long long)fhwc.fhwc_tsp.tv_nsec / 1000, d);
			}
			else if (direction == FHWC_LOAD) {
				fhwc.fhwc_tsp.tv_sec = mktime(fhwc_tm);
				memcpy(&tsp, &fhwc.fhwc_tsp, sizeof(struct timespec));
				if (clock_settime(CLOCK_REALTIME, &tsp) == -1) {
					perror(svclockf);
					return errno;
				}
			}
		break;
		case FHWC_SAVE:
			if (clock_gettime(CLOCK_REALTIME, &tsp) == -1) {
				perror("clock_gettime");
				return errno;
			}
			memcpy(&fhwc.fhwc_tsp, &tsp, sizeof(struct timespec));
			fhwc.fhwc_dir = 0x66687701;
			if (clockset) fhwc.fhwc_utc = clockset;
			else fhwc.fhwc_utc = FHWC_UTC;
			fd = open(svclockf, O_WRONLY | O_TRUNC | O_CREAT, 0666);
			if (fd == -1) {
				perror(svclockf);
				return errno;
			}
			if (write(fd, &fhwc, sizeof(fhwc)) < sizeof(fhwc)) {
				perror(svclockf);
				return errno;
			}
			close(fd);
		break;
	}

	return 0;
}
