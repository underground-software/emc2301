#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int my_dev = open("/dev/emc2301_fan", O_RDWR);

	if (my_dev == -1) {
		perror("open failed");
	}

	char a[5] = "abcde";
	int speed = atoi(argv[1]);

	printf("file descriptor: %d\n", my_dev);

	int err = write(my_dev, a, speed); /*set fan speed.*/

	if (err == -1) {
		perror("write failed");
		return 1;
	}

	return 0;
}
