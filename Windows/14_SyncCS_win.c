#include <stdio.h>
#include <Windows.h>
#include <process.h>

#define NUM_THREAD 50
unsigned WINAPI threadInc(void* arg);
unsigned WINAPI threadDes(void* arg);

long long num = 0;
CRITICAL_SECTION cs;  // for synchronization in User mode

int main(int argc, char** argv) {
	HANDLE tHandles[NUM_THREAD];
	int i;

	InitializeCriticalSection(&cs);  // Init resources of a critical section object using
	for (i = 0; i < NUM_THREAD; i++) {
		if (i % 2)
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}

	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);
	DeleteCriticalSection(&cs);  // remove resources
	printf("result: %lld \n", num);
	return 0;
}

unsigned WINAPI threadInc(void* arg) {
	int i;
	EnterCriticalSection(&cs);  // Gain critical section object
	for (i = 0; i < 50000000; i++)
		num += i;
	LeaveCriticalSection(&cs); // Return cs object
	return 0;
}

unsigned WINAPI threadDes(void* arg) {
	int i;
	EnterCriticalSection(&cs);
	for (i = 0; i < 50000000; i++)
		num -= i;
	LeaveCriticalSection(&cs);
	return 0;
}