#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// inet_addr 함수 호출의 예
	{
		char *addr = "127.212.124.78";
		unsigned long conv_addr = inet_addr(addr); // 문자열 -> 정수
		if (conv_addr == INADDR_NONE)
			printf("Error occured! \n");
		else
			printf("Network ordered integer addr : %#lx \n", conv_addr);
	}

	// inet_ntoa 함수 호출의 예
	{
		struct sockaddr_in addr;
		char *strPtr;
		char strArr[20];

		addr.sin_addr.s_addr = htonl(0x1020304);
		strPtr = inet_ntoa(addr.sin_addr); // 네트워크 바이트 순서의 정수형 IP 주소 -> 문자열 IP 주소
		strcpy(strArr, strPtr); // 내부적으로 메모리를 할당하기 때문에 재호출되면 값이 변한다. 따라서 따로 복사하여 보관할 것
		printf("Dotted-Decimal notation 3 %s\n", strArr);
	}
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}