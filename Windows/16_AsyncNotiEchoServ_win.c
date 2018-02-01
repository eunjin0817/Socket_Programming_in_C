#include <stdio.h>
#include <string.h>
#include <WinSock2.h>

#define BUF_SIZE 100

void CompressSockets(SOCKET sockAddr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char* msg);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET servSock, clntSock;
	SOCKADDR_IN servAddr, clntAddr;

	SOCKET sockArr[WSA_MAXIMUM_WAIT_EVENTS]; // 64
	WSAEVENT eventArr[WSA_MAXIMUM_WAIT_EVENTS]; // 64
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;
	
	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx;
	int clntAddrLen;
	char msg[BUF_SIZE];

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	servSock = socket(PF_INET, SOCK_STREAM, 0);
	if (servSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(servSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	// Create Event object of 'non-signaled' state in 'menual-reset' mode.
	newEvent = WSACreateEvent();

	// Targeting arbitrary socket, observe whether any event occur or not.
	if (WSAEventSelect(servSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	// Respectivly save the socket connected with observed Event object.
	sockArr[numOfClntSock] = servSock;
	eventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1) {
		// WSAWaitForMultipleEvents() returns the index of array storing handle of the signaled event object.
		// First, check occurence of the event, FD_ACCEPT
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, eventArr, FALSE, WSA_INFINITE, FALSE);
		startIdx = posInfo - WSA_WAIT_EVENT_0;  // next index.

		for (i = startIdx; i < numOfClntSock; i++) {
			int sigEventIdx = WSAWaitForMultipleEvents(1, &eventArr[i], TRUE, 0, FALSE);
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
				continue;
			
			sigEventIdx = i;
			// Check cause of event occurrence
			WSAEnumNetworkEvents(sockArr[sigEventIdx], eventArr[sigEventIdx], &netEvents);
			if (netEvents.lNetworkEvents & FD_ACCEPT) { // when received connecting request
				if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
					puts("Accept Error");
					break;
				}
				clntAddrLen = sizeof(clntAddr);
				clntSock = accept(sockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrLen);
				newEvent = WSACreateEvent();
				WSAEventSelect(clntSock, newEvent, FD_READ | FD_CLOSE);

				eventArr[numOfClntSock] = newEvent;
				sockArr[numOfClntSock] = clntSock;
				numOfClntSock++;
				puts("connected new client....");
			}

			if (netEvents.lNetworkEvents & FD_READ) { // when received data from a client.
				if (netEvents.iErrorCode[FD_READ_BIT] != 0) {
					puts("Read Error");
					break;
				}
				strLen = recv(sockArr[sigEventIdx], msg, sizeof(msg), 0);
				send(sockArr[sigEventIdx], msg, strLen, 0);  // echo
			}

			if (netEvents.lNetworkEvents & FD_CLOSE) { // when connected client close
				if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
					puts("Close Error");
					break;
				}
				WSACloseEvent(eventArr[sigEventIdx]);
				closesocket(sockArr[sigEventIdx]);

				numOfClntSock--;
				CompressSockets(sockArr, sigEventIdx, numOfClntSock); // remove closed socket in array
				CompressEvents(eventArr, sigEventIdx, numOfClntSock); // removed closed event in array
			}
		}
	}

	WSACleanup();
	return 0;
}

void CompressSockets(SOCKET sockArr[], int idx, int total) {
	int i;
	for (i = idx; i < total; i++)
		sockArr[i] = sockArr[i + 1];
}

void CompressEvents(WSAEVENT eventArr[], int idx, int total) {
	int i;
	for (i = idx; i < total; i++)
		eventArr[i] = eventArr[i + 1];
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}