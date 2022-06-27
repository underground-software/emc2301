#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
	int my_dev = open("/dev/emc2301_fan", O_RDWR);

	if (my_dev == -1) {
		perror("open failed");
	}

	char a;
	int speed = 0;

	printf("file descriptor: %d\n", my_dev);

	int err = write(my_dev, &speed, 1); /*set fan speed to 0.*/

	if (err == -1) {
		perror("write failed");
		return 1;
	}

	return 0;
}
