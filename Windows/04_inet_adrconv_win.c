#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// inet_addr �Լ� ȣ���� ��
	{
		char *addr = "127.212.124.78";
		unsigned long conv_addr = inet_addr(addr); // ���ڿ� -> ����
		if (conv_addr == INADDR_NONE)
			printf("Error occured! \n");
		else
			printf("Network ordered integer addr : %#lx \n", conv_addr);
	}

	// inet_ntoa �Լ� ȣ���� ��
	{
		struct sockaddr_in addr;
		char *strPtr;
		char strArr[20];

		addr.sin_addr.s_addr = htonl(0x1020304);
		strPtr = inet_ntoa(addr.sin_addr); // ��Ʈ��ũ ����Ʈ ������ ������ IP �ּ� -> ���ڿ� IP �ּ�
		strcpy(strArr, strPtr); // ���������� �޸𸮸� �Ҵ��ϱ� ������ ��ȣ��Ǹ� ���� ���Ѵ�. ���� ���� �����Ͽ� ������ ��
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