#include "pch.h"
#include <winsock2.h>
#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment (lib, "wsock32.lib")


bool flag = TRUE;

struct unit
{
	SOCKET s;
	char buf[100000];
	int recvnum;
	bool SendFunc;
	bool RecvFunc;
};

typedef struct Unit
{
	struct unit Unit;
	struct Unit *front;
	struct Unit *next;
}Linklist;

unsigned _stdcall recv_proc(LPVOID lpParam);
Linklist *initlinklist(int n);
Linklist *deletenode(Linklist *Head, Linklist*outnode);
Linklist *addnode(Linklist*Head, Linklist *endnode, Linklist*addnode);
int getlinklistlength(Linklist*Head);

int main()
{
	
	WSADATA wsa;
	sockaddr_in local;
	
	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	local.sin_port = htons(0x7856);

	WSAStartup(0x101, &wsa);

	int count = 0;
	while (1)
	{
		if (flag == TRUE)
		{
			int judge;
			judge = _beginthreadex(NULL, 0, recv_proc, (LPVOID)&flag, 0, NULL);
			if (judge == 0)
			{
				printf("Create new thread failed.\n");
				printf("The server may get its max power.\n");
			}
			else
			{
				flag = FALSE;
				printf("Thread %d Created.\n", count + 1);
				count++;
			}
		}
	}
}


unsigned _stdcall recv_proc(LPVOID lpParam)
{
	fd_set read_list;
	fd_set exception_list;
	fd_set write_list;
	struct timeval tmo;
	tmo.tv_sec = 2;
	tmo.tv_usec = 0;
	struct unit Units[10];
	int i = 0;
	SOCKET MainSocket;
	struct sockaddr_in remote_addr,local;
	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	local.sin_port = htons(0x7856);
	//struct sockaddr_in*lo=&local;
	//lo = (sockaddr_in*)lpParam;
	u_long arg;
	arg = 1;
	Linklist*Head,*Current;
	Head = (Linklist*)malloc(sizeof(Linklist));
	Head->front = NULL;
	Head->next = NULL;
	Current = Head;
	MainSocket = socket(AF_INET, SOCK_STREAM, 0);
	bind(MainSocket, (sockaddr*)&local, sizeof(local));
	ioctlsocket(MainSocket, FIONBIO, &arg);
	listen(MainSocket, 10);
	int countdown = 10;
	
	while (1)
	{
		FD_ZERO(&read_list);
		FD_ZERO(&exception_list);
		FD_ZERO(&write_list);
		if (i < 10)
		{
			FD_SET(MainSocket, &read_list);
		}
		select(0, &read_list, &write_list, &exception_list, &tmo);
		if (FD_ISSET(MainSocket, &read_list))
		{
			int len;
			len = sizeof(remote_addr);
			Units[i].s = accept(MainSocket, (sockaddr*)&remote_addr, &len);
			Units[i].SendFunc = TRUE;
			Units[i].RecvFunc = TRUE;
			i++;
			if (i == 10)
			{
				flag = TRUE;
				closesocket(MainSocket);
			}
			for (int j = 0; j < i; j++)
			{
				FD_SET(Units[j].s, &read_list);
				FD_SET(Units[j].s, &write_list);
				FD_SET(Units[j].s, &exception_list);
				ioctlsocket(Units[j].s, FIONBIO, &arg);
			}
		}
		for (int j = 0; j < i; j++)
		{
			char *p = Units[j].buf;
			while (FD_ISSET(Units[j].s, &read_list))
			{
				int Checkrecvnum;
				Checkrecvnum = recv(Units[j].s, p, sizeof(Units[j].buf), 0);
				if (Checkrecvnum != -1)
				{
					Units[j].recvnum = Checkrecvnum;
					Linklist*node = (Linklist*)malloc(sizeof(Linklist));
					Current->next = node;
					Current->Unit = Units[j];
				}
				/*else if (Checkrecvnum == -1)
				{

					countdown--;
					shutdown(Units[j].s, SD_RECEIVE);
					Units[j].RecvFunc = FALSE;
				}*/
				else
					break;
			}
		}
		Current = Head->next;
		int n = getlinklistlength(Head);
		if (n > 0)
		{
			for (int j = 0; j < n; j++)
			{
				if (FD_ISSET(Current->Unit.s, &write_list))
				{
					int sendnum;
					char *p = Current->Unit.buf;
					sendnum = send(Current->Unit.s, p, Current->Unit.recvnum, 0);
					if (sendnum == Current->Unit.recvnum&&Current->Unit.RecvFunc==FALSE)
					{
						Linklist*outnode = Current;
						Current = Current->next;
						deletenode(Head, outnode);
						closesocket(Current->Unit.s);
					}
					else if (sendnum < Current->Unit.recvnum)
					{
						Linklist*outnode = Current;
						Linklist*innode = (Linklist*)malloc(sizeof(Linklist));
						innode = Current;
						Linklist*endnode = Head;
						Current = Current->next;
						Head = deletenode(Head, outnode);
						while (endnode->next != NULL)
						{
							endnode = endnode->next;
						}
						Head = addnode(Head, endnode, innode);
					}
				}
			}
		}
	}

	return 0;
}

Linklist *initlinklist(int n)
{
	Linklist *Head, *node;
	Head = (Linklist*)malloc(sizeof(Unit));
	Head->front = NULL;
	Linklist*head = Head;
	for (int i = 0; i < n; i++)
	{
		node = (Linklist*)malloc(sizeof(Unit));
		head->next = node;
		node->front = head;
		head = node;
	}
	head->next = NULL;
	return Head;
}

Linklist *deletenode(Linklist *Head, Linklist*outnode)
{
	Linklist*p = Head->next;
	while (p != outnode&&p->next!=NULL)
	{
		p = p->next;
	}
	if (p == outnode&&p->next!=NULL)
	{
		p->front->next = p->next;
		p->next->front = p->front;
	}
	else if (p != outnode && p->next == NULL)
	{
		printf("delete node failed:invaild node.\n");
	}
	else if (p == outnode && p->next == NULL)
	{
		p->front->next = NULL;
	}
	return Head;
}

Linklist *addnode(Linklist*Head, Linklist *endnode, Linklist*addnode)
{
	endnode->next = addnode;
	addnode->front = endnode;
	addnode->next = NULL;
	return Head;
}

int getlinklistlength(Linklist*Head)
{
	int length = 1;
	Linklist*Current;
	Current = Head;
	while (Current->next != NULL)
	{
		length++;
		Current = Current->next;
	}
	return length-1;
}