#include<iostream>
#include<WinSock2.h>
#include<process.h>
#include<stdio.h>

#pragma comment(lib,"ws2_32.lib")

SOCKET sockCli;

bool IsSockwtClosed(SOCKET clientSocket) {
	bool ret = false;
	HANDLE closeEvent = WSACreateEvent();
	WSAEventSelect(clientSocket, closeEvent, FD_CLOSE);

	DWORD dwRet = WaitForSingleObject(closeEvent, 0);

	if (dwRet == WSA_WAIT_EVENT_0) {
		ret = true;
	}
	else if (dwRet == WSA_WAIT_TIMEOUT) {
		ret = false;
	}
	WSACloseEvent(closeEvent);
	return ret;
}

unsigned int WINAPI ThreadRecv(LPVOID p) {
	char recvBuf[100];
	memset(recvBuf, 0, 100);
	while (1) {
		Sleep(20);

		if (IsSockwtClosed(sockCli) == true) {
			printf("server disconnect!\n");
			break;
		}
		recv(sockCli, recvBuf, sizeof(recvBuf), 0);
		if (strlen(recvBuf) == 0)continue;
		std::cout << recv << std::endl;
		memset(recvBuf, 0, 100);
	}
	return 0;
}
int main() {
	WORD wVwerion;
	WSADATA wsaData;
	int err;

	wVwerion = MAKEWORD(1, 1);

	err = WSAStartup(wVwerion, &wsaData);
	if (err != 0) {
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	sockCli = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_port = htons(6000);
	addrSrv.sin_family = AF_INET;

	int err_log = connect(sockCli, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (err_log == 0) {
		printf("connect server success \n");
	}
	else {
		printf("connect server failed \n");
		return -1;
	}

	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, NULL, 0, NULL);

	char sendBuf[100];

	while (1)
	{
		scanf_s("%s", sendBuf, 100);
		send(sockCli, sendBuf, strlen(sendBuf) + 1, 0);
		memset(sendBuf, 0, 100);
		char c;
		while ((c=getchar())!='\n');
	
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	closesocket(sockCli);
	WSACleanup();
	system("pause");
	return 0;
}
