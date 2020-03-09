#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <errno.h>
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>

#define GPIO83_ON	0x01
#define GPIO82_ON	0x02
#define GPIO83_OFF	0x10
#define GPIO82_OFF	0x20
#define ALL_ON		0x03
#define ALL_OFF		0x30

#define MAP_SIZE        0x1000          /* 페이지 크기(4096)의 정수배 */
#define MAP_PHYS        0x14000000      /* 절대번지 */
                                                                                
#define LEDREG          (*((volatile unsigned int *)(map_base + 0)))

void *gugudan();
void *led_fnd();
void error_handling(char *message);

void *map_base;

int correct = 2; // none answer
int time_over = 0;
int score = 0;

unsigned int led_val[] = {
	0x00FFFF, // 8sec (ALL ON)
        0x01FFFF, // 7sec
        0x03FFFF, // 6sec
        0x07FFFF, // 5sec
        0x0FFFFF, // 4sec
        0x1FFFFF, // 3sec
        0x3FFFFF, // 2sec
        0x7FFFFF, // 1sec
        0xFFFFFF  // 0sec (ALL OFF)
};

unsigned int fnd_val[] = { // index: 7segment
	0xFFFFC0, // 0: 00
	0xFFFFF9, // 1: 01
	0xFFFFA4, // 2: 02
	0xFFFFB0, // 3: 03
	0xFFFF99, // 4: 04
	0xFFFF92, // 5: 05
	0xFFFF82, // 6: 06
	0xFFFFD8, // 7: 07
	0xFFFF80, // 8: 08
	0xFFFF90, // 9: 09
	0xFFF9FF, // 10: 10
	0xFFA4FF, // 11: 20
	0xFFB0FF, // 12: 30
	0xFF99FF, // 13: 40
	0xFF92FF, // 14: 50
	0xFF82FF, // 15: 60
	0xFFD8FF, // 16: 70
	0xFF80FF, // 17: 80
	0xFF90FF  // 18: 90
};

int main(int argc, char *argv[])
{
	pthread_t p1, p2;
	
	pthread_create(&p1, NULL, gugudan, NULL);
	pthread_create(&p2, NULL, led_fnd, NULL);

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
}

void *gugudan()
{
	int sock;
	struct sockaddr_in serv_addr;
	
	int msg[5];
	int answer;

	int fd, i;
	int x, y;
	unsigned char c;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	if ( sock == -1 )
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("192.168.1.151");
	serv_addr.sin_port = htons(8279);

	if ( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1 )
		error_handling("connect() error");

	fd = open("/dev/gpio_test", O_WRONLY);
	if (fd < 0) {
		printf("Device Open Error\n");
		exit(1);
	}
	
	while(1)
	{
		correct = 2;
		read(sock, msg, sizeof(msg));
		
		printf("\n");
		printf("Message from server : \n");
		printf("%d * %d = ?\n", msg[0], msg[1]);
                               
		printf("Enter the Answer : ");

		scanf("%d", &answer);
		
		if (answer == msg[2]) {
			correct = 1;
			score++;
			printf("Correct!\n");
			
			x = score / 10;
			y = score % 10;
			if (x == 0)
				LEDREG = fnd_val[y];
			else
				LEDREG = fnd_val[x + 10 - 1] + fnd_val[y] - 0x00FFFF;
			sleep(1.5);

			for (i = 0; i < 2; i++)	{
				c = ALL_ON;
				write(fd, &c, 1);
				sleep(1);

				c = ALL_OFF;
				write(fd, &c, 1);
				sleep(1);
			}
		} else {
			correct = 0;
			printf("Wrong Answer\n");
			sleep(2);
		}
		
		write(sock, &correct, sizeof(correct));
	}

	close(fd);
	close(sock);
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
 * led_fnd.c
 */

void *led_fnd()
{
	int fd, i;

	/*
	 * 주의: O_SYNC 플래그를 줘야지만 캐쉬되지 않는다.
	 */
	fd = open( "/dev/mem", O_RDWR | O_SYNC );
	if (fd == -1) {
		perror("open(\"/dev/mem\")");
		exit(1);
	}

	/*
         * MAP_SIZE에 해당하는  페이지 만큼의 영역을 매핑한다.
         */
        map_base = mmap( NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MAP_PHYS );
        if (map_base == MAP_FAILED) {
                perror("mmap()");
                exit(2);
        }

	while(1) {
		for (i = 0; i < 9; i++) {
			if (correct != 2) {
				LEDREG = led_val[8];
				break;
			}
			LEDREG = led_val[i];
			sleep(1);
		}
		
		if (i == 9) {
			printf("\n ----- Time Over ----- \n");
			break;
		}
	}

	/*
         * 할당받았던 매핑 영역을 해제한다.
         */
        if (munmap(map_base, MAP_SIZE) == -1) {
                perror("munmap()");
                exit(3);
        }
                                                                                
        close(fd);
}

