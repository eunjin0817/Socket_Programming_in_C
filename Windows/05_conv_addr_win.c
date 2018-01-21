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
	// 문자열 주소 -> 정수형 주소로 변환하여 sockaddr_in 구조체에 대입
	// 다른 운영체제로의 이식성이 떨어지므로 권장되지 않는 윈속 변환 함수
	WSAStringToAddress(strAddr, AF_INET, NULL, (SOCKADDR*)&servAddr, &size);
	size = sizeof(strAddrBuf);
	// 정수형 주소 -> 문자열 주소로 변환
	WSAAddressToString((SOCKADDR*)&servAddr, sizeof(servAddr), NULL, strAddrBuf, &size);
	
	printf("Second conv result : %s \n", strAddrBuf);
	WSACleanup();
	return 0;
}