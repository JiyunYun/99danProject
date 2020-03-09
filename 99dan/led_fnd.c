/*
 * led_fnd.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>

#define MAP_SIZE	0x1000		/* ������ ũ��(4096)�� ������ */
#define MAP_PHYS	0x14000000	/* ������� */

#define LEDREG		(*((volatile unsigned int *)(map_base + 0)))
unsigned int val[] = { 0x01f9f9, 0x02a4a4, 0x04b0b0, 0x089999, 0x109292, 0x208282, 0x40d8d8, 0x808080, 0x009090 };

void *map_base;

int quit = 0;

void quit_signal(int sig)
{
	quit = 1;
}

int main (int argc, char *argv[])
{
	int fd;
	int idx = 0;
	static unsigned char led;
	static unsigned int count;

	/*
	 * ����: O_SYNC �÷��׸� ������� ĳ������ �ʴ´�.
	 */
	fd = open( "/dev/mem", O_RDWR | O_SYNC );
	if (fd == -1) {
		perror("open(\"/dev/mem\")");
		exit(1);
	}

	/*
	 * MAP_SIZE�� �ش��ϴ�  ������ ��ŭ�� ������ �����Ѵ�.
	 */
	map_base = mmap( NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MAP_PHYS );
	if (map_base == MAP_FAILED) {
		perror("mmap()");
		exit(2);
	}

	/* 
	 *	<ctrl+c> �� ������ ���ᵵ�� signal�� ����Ѵ�.
	 */
	signal(SIGINT, quit_signal );
	printf("\nPress <ctrl+c> to quit.\n\n");

	while(!quit)
	{
        	LEDREG = (val[(led++) % 9]);
		usleep(35000);
	}
	LEDREG = 0xffffff;

	/*
	 * �Ҵ�޾Ҵ� ���� ������ �����Ѵ�.
	 */
	if (munmap(map_base, MAP_SIZE) == -1) {
		perror("munmap()");
		exit(3);
	}

	close(fd);

	return 0;
}

