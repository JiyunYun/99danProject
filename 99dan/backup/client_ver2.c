#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	
	int msg[5];
	int answer, correct;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	if ( sock == -1 )
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(8279);

	if ( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1 )
		error_handling("connect() error");

	while(1)
	{
		read(sock, msg, sizeof(msg));
		
		printf("\n");
		printf("Message from server : \n");

		printf("%d * %d = ?\n", msg[0], msg[1]);
		printf("Enter the Answer : ");
		scanf("%d", &answer);

		if (answer == msg[2]) {
			correct = 1;
			printf("Correct!\n");
		} else {
			correct = 0;
			printf("Wrong Answer...\n");
		}

		write(sock, &correct, sizeof(correct));
	}

	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
