#undef UNICODE
#undef _UNICODE
#include <stdio.h>
#include <WinSock2.h>

int main(int argc, char* argv[]) {
	char *strAddr = "203.211.218.102:9190";

	char strAddrBuf[50];
	SOCKADDR_IN servAddr;
	int size;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	size = sizeof(servAddr);
	// ���ڿ� �ּ� -> ������ �ּҷ� ��ȯ�Ͽ� sockaddr_in ����ü�� ����
	// �ٸ� �ü������ �̽ļ��� �������Ƿ� ������� �ʴ� ���� ��ȯ �Լ�
	WSAStringToAddress(strAddr, AF_INET, NULL, (SOCKADDR*)&servAddr, &size);
	size = sizeof(strAddrBuf);
	// ������ �ּ� -> ���ڿ� �ּҷ� ��ȯ
	WSAAddressToString((SOCKADDR*)&servAddr, sizeof(servAddr), NULL, strAddrBuf, &size);
	
	printf("Second conv result : %s \n", strAddrBuf);
	WSACleanup();
	return 0;
}