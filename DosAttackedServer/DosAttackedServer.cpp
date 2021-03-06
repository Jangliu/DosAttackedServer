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
	bool RecvFunc;
};

typedef struct Unit
{
	struct unit Unit;
	struct Unit *front;
	struct Unit *next;
}Linklist;

unsigned _stdcall recv_proc(LPVOID lpParam);
Linklist *deletenode(Linklist *Head, Linklist*outnode);
Linklist *addnode(Linklist*Head, Linklist*addnode);
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
	//recv_proc((LPVOID)&flag);
}


unsigned _stdcall recv_proc(LPVOID lpParam)
{
	fd_set read_list;
	fd_set exception_list;
	fd_set write_list;
	struct timeval tmo;
	tmo.tv_sec = 1;
	tmo.tv_usec = 50;
	struct unit Units[10];
	int i = 0;
	SOCKET MainSocket,S;
	struct sockaddr_in remote_addr, local;
	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	local.sin_port = htons(0x7856);
	u_long arg;
	arg = 1;
	Linklist*Head, *Current;
	Head = (Linklist*)malloc(sizeof(Linklist));
	Head->front = NULL;
	Head->next = NULL;
	MainSocket = socket(AF_INET, SOCK_STREAM, 0);
	S = socket(AF_INET, SOCK_STREAM, 0);
	bind(MainSocket, (sockaddr*)&local, sizeof(local));
	ioctlsocket(MainSocket, FIONBIO, &arg);
	listen(MainSocket, 10);
	int countdown = 10;

	while (countdown > 0)
	{
		FD_ZERO(&read_list);
		FD_ZERO(&exception_list);
		FD_ZERO(&write_list);
		if (i < 10)
		{
			FD_SET(MainSocket, &read_list);
		}
		select(0, &read_list, &write_list, &exception_list, &tmo);
		Sleep(50);
		while (FD_ISSET(MainSocket, &read_list))
		{
			int len;
			len = sizeof(remote_addr);
			S = accept(MainSocket, (sockaddr*)&remote_addr, &len);
			if (S != -1)
			{
				Units[i].s = S;
				Units[i].RecvFunc = TRUE;
				i++;
				if (i == 10)
				{
					flag = TRUE;
					FD_CLR(MainSocket, &read_list);
					closesocket(MainSocket);
				}
			}
			else
				break;
		}
		for (int j = 0; j < i; j++)
		{
			FD_SET(Units[j].s, &read_list);
			FD_SET(Units[j].s, &write_list);
			FD_SET(Units[j].s, &exception_list);
			ioctlsocket(Units[j].s, FIONBIO, &arg);
		}
		bool x = TRUE;
		//int a = 0;
		//Sleep(100);
		while (x)
		{
			int N[10] = { 1 };
			int k = 0;
			//Sleep(100);
			for (int j = 0; j < i; j++)
			{
				if (FD_ISSET(Units[j].s, &read_list))
				{
					int Checkrecvnum;
					char*p = Units[j].buf;
					Checkrecvnum = recv(Units[j].s, p, sizeof(Units[j].buf), 0);
					if (Checkrecvnum > 0)
					{
						int sendnum;
						Sleep(50);
						sendnum = send(Units[j].s, p, sizeof(Units[j].buf), 0);
						if (sendnum < Checkrecvnum)
						{
							Units[j].recvnum = Checkrecvnum - sendnum;
							Linklist*node = (Linklist*)malloc(sizeof(Linklist));
							node->Unit = Units[j];
							node->next = NULL;
							Head = addnode(Head, node);
						}
						//a++;
						//printf("%d\n", a);
					}
					else if (Checkrecvnum == 0)
					{
						Units[j].RecvFunc = FALSE;
						N[j] = 0;
					}
				}
				else
				{
					N[j] = 0;
				}
			}
			for (int j = 0; j < i; j++)
			{
				if (N[j] == 0)
					k++;
			}
			if (k == i)
			{
				x = FALSE;
			}
		}
		int n = getlinklistlength(Head);
		if (n == 0)
		{
			for (int j = 0; j < i; j++)
			{
				if (Units[j].RecvFunc == FALSE)
				{
					closesocket(Units[j].s);
					countdown--;
				}
			}
		}
		else
		{
			while (n != 0)
			{
				Current = Head->next;
				while (Current->next != NULL)
				{
					if (FD_ISSET(Current->Unit.s, &write_list))
					{
						int sendnum;
						char *p = Current->Unit.buf;
						Sleep(50);
						sendnum = send(Current->Unit.s, p, sizeof(Current->Unit.buf), 0);
						if (sendnum == Current->Unit.recvnum)
						{
							/*if (Current->Unit.RecvFunc == FALSE)
							{
								closesocket(Current->Unit.s);
								printf("%d\n", Current->Unit.total);
								countdown--;
							}*/
							Linklist*outnode = Current;
							Current = Current->next;
							Head = deletenode(Head, outnode);
						}
						else
						{
							Linklist*outnode = Current;
							Linklist*innode = (Linklist*)malloc(sizeof(Linklist));
							innode->Unit.s = Current->Unit.s;
							innode->Unit.recvnum = Current->Unit.recvnum - sendnum;
							Current = Current->next;
							Head = deletenode(Head, outnode);
							Head = addnode(Head, innode);
						}
					}
				}
				if (FD_ISSET(Current->Unit.s, &write_list))
				{
					int sendnum;
					char *p = Current->Unit.buf;
					sendnum = send(Current->Unit.s, p, sizeof(Current->Unit.buf), 0);
					if (sendnum == Current->Unit.recvnum)
					{
						if (Current->Unit.RecvFunc == FALSE)
						{
							closesocket(Current->Unit.s);
							countdown--;
						}
						Linklist*outnode = Current;
						Current = Current->next;
						Head = deletenode(Head, outnode);
					}
					else
					{
						Linklist*outnode = Current;
						Linklist*innode = (Linklist*)malloc(sizeof(Linklist));
						innode->Unit.s = Current->Unit.s;
						innode->Unit.recvnum = Current->Unit.recvnum - sendnum;
						Current = Current->next;
						Head = deletenode(Head, outnode);
						Head = addnode(Head, innode);
					}
				}
				n = getlinklistlength(Head);
			}
			for (int j = 0; j < i; j++)
			{

				closesocket(Units[j].s);
				countdown--;
			}
		}
	}
	printf("A Thread finished.\n");
	return 0;
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
		free(p);
		return Head;
	}
	else if (p != outnode && p->next == NULL)
	{
		printf("delete node failed:invaild node.\n");
		return Head;
	}
	else if (p == outnode && p->next == NULL)
	{
		p->front->next = NULL;
		free(p);
		return Head;
	}
	return Head;
}

Linklist *addnode(Linklist*Head, Linklist*addnode)
{
	Linklist*Current = Head;
	while (Current->next != NULL)
	{
		Current = Current->next;
	}
	addnode->front = Current;
	Current->next = addnode;
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