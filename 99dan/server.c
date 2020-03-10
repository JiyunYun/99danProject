#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

void error_handling(char * message);

int main()
{
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	
	int buff[5];
	int correct;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if( serv_sock == -1 )
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // auto find server ip address 32bit num
	serv_addr.sin_port = htons(8279);
	
	if( bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1 )
		error_handling("bind() error");

	if( listen(serv_sock, 5) == -1 )
		error_handling("listen() error");
	
	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
	if ( clnt_sock == -1 )
		error_handling("accept() error");

	srand(time(NULL));
	while(1) {
		buff[0] = (rand() % 8) + 2;
		buff[1] = (rand() % 8) + 2;
		buff[2] = buff[0] * buff[1]; // answer

		printf("%d * %d = %d\n", buff[0], buff[1], buff[2]);
		write(clnt_sock, buff, sizeof(buff));

		read(clnt_sock, &correct, sizeof(correct));
		
		if (correct)
			printf("User Input Right Answer\n");
		else
			printf("User Input Wrong Answer\n");
		printf("\n");
	}	

	close(clnt_sock);
	close(serv_sock);

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}	
