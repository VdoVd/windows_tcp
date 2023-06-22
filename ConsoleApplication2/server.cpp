// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<WinSock2.h>
#include<stdio.h>
#include<Windows.h>
#include<process.h>
#include<vector>
#include<conio.h>
#include<string.h>
#include<string>

#pragma comment(lib,"ws2_32.lib")
SOCKET sockSrv;
std::vector<SOCKET> vec_sockConn;
std::vector<SOCKADDR_IN>vec_sockaddr_in;
std::vector<int>vec_sockIndex;

typedef struct SERVER_CLIENT {
    SOCKET server;
    SOCKADDR_IN client;
    int clientIndex;
}SC;

bool IsSocketClosed(SOCKET clientSocket) {
    bool ret = false;
    HANDLE closeEvvent = WSACreateEvent();
    WSAEventSelect(clientSocket, CloseEventLog, FD_CLOSE);
    DWORD dwRet = WaitForSingleObject(closeEvvent, 0);
    if (dwRet == WSA_WAIT_EVENT_0) {
        ret = true;
    }
    else if (dwRet == WSA_WAIT_TIMEOUT)
        return false;
    WSACloseEvent(closeEvvent);
    return ret;
}

unsigned int WINAPI ThreadAccept(LPVOID p) {
    static int i = 0;
    while (1) {
        SOCKADDR_IN addrCli;
        int len = sizeof(SOCKADDR);

        SOCKET sockConn = accept(sockSrv, (SOCKADDR *)&addrCli, &len);
        if (sockConn == SOCKET_ERROR) {
            printf("Accept failed:%d\n", WSAGetLastError());
        }
        vec_sockIndex.emplace_back(i++);
        vec_sockaddr_in.emplace_back(addrCli);
        vec_sockConn.emplace_back(sockConn);

        printf("\033[0;%d;40m client [%d] online \033 [0m\n", 31, i);
    }
    return 0;
}

unsigned int WINAPI _ThreadRecv(LPVOID p) {
    char recvBuf[100];
    memset(recvBuf, 0, 100);
    SC _sc = *(SC*)p;
    while (1) {
        Sleep(20);
        if (IsSocketClosed(_sc.server) == true) {
            printf("client [%d] disconnect! \n", _sc.clientIndex + 1);
            break;
        }

        recv(_sc.server, recvBuf, 100, 0);
        if (strlen(recvBuf) == 0) {
            continue;
        }

        printf("receive message from client [%d]:%s\n", _sc.clientIndex + 1,recvBuf);
        memset(recvBuf, 0, 100);
    }
    return 0;
}

unsigned int WINAPI ThreadRecv(LPVOID p) {
    static int index = 0;
    while (1) {
        if (vec_sockConn.size() == 0) {
            continue;
        }
        if (vec_sockaddr_in.size() == index) {
            continue;
        }
        SC sc;
        sc.server = vec_sockConn.at(index);
        sc.client = vec_sockaddr_in.at(index);
        sc.clientIndex = vec_sockIndex.at(index);

        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, _ThreadRecv, (void*)&sc, 0, NULL);

        index++;
        Sleep(20);
    }
    return 0;
}
int main()
{
    WORD wVersion;
    WSADATA wsaData;
    int err;
    wVersion = MAKEWORD(1, 1);

    err = WSAStartup(wVersion, &wsaData);
    if (err != 0) {
        return err;
    }
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup();
        return -1;
    }
    sockSrv = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(6000);
    printf("server cpp run...\n");

    int retVal = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if (retVal == SOCKET_ERROR) {
        printf("Failed bind:%d\n", WSAGetLastError());
        return -1;
    }

    if (listen(sockSrv, 10) == SOCKET_ERROR) {
        printf("Listen failed :%d", WSAGetLastError());
        return -1;
    }

    std::cout << "server start at port: 6000" << std::endl;

    HANDLE hThread_1 = (HANDLE)_beginthreadex(NULL, 0, ThreadAccept, NULL, 0, NULL);
    HANDLE hThread_2 = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, NULL, 0, NULL);

    char sendBuff[100];
    while (1)
    {
        char c = getchar();
        scanf_s("%s", sendBuff, 100);
        if (strlen(sendBuff) == 0) {
            printf("input content is null or long!\n");
        }

        if (c < '1' | c>'9' || vec_sockConn.size() == 0 || c - '0' >= vec_sockConn.size() + 1) {
            while ((c = getchar()) != '\n');
            memset(sendBuff, 0, 100);
            printf("input content is unfit the rule\n");
            continue;
        }

        int index = --c - '0';
        int iSend = send(vec_sockConn.at(index), sendBuff, strlen(sendBuff) + 1, 0);
        if (iSend == SOCKET_ERROR) {
            std::cout << "send failed!\n";
            break;
        }
        memset(sendBuff, 0, 100);
        while ((c = getchar()) != '\n');

    }
    std::vector<SOCKET>::iterator it = vec_sockConn.begin();
    for (; it != vec_sockConn.end(); it++) {
        closesocket((SOCKET)(*it));
    }

    WaitForSingleObject(hThread_1, INFINITE);
    WaitForSingleObject(hThread_2, INFINITE);

    CloseHandle(hThread_1);
    CloseHandle(hThread_2);

    closesocket(sockSrv);

    WSACleanup();
    return 0;

}
    

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
