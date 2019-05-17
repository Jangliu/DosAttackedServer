#include "pch.h"
#include <winsock2.h>
#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment (lib, "wsock32.lib")


bool flag = TRUE;

struct argv
{
	SOCKET s;
	bool flag;
};

struct unit
{
	SOCKET s;
	char buf[100000];
	int recvnum;
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

int main()
{
	int ThreadID = 0;
	struct argv Argv;
	SOCKET MainSocket;
	WSADATA wsa;
	sockaddr_in local;
	HANDLE* hThread;
	
	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_port = htons(0x7856);

	WSAStartup(0x101, &wsa);
	MainSocket = socket(AF_INET, SOCK_STREAM, 0);
	bind(MainSocket, (sockaddr*)&local, sizeof(local));

	listen(MainSocket, 10);

	/*while (flag == TRUE)
	{
		Argv.s = MainSocket;
		Argv.flag = flag;
		int judge;
		judge = _beginthreadex(NULL, 0, recv_proc, (LPVOID)&Argv, 0, NULL);
		if (judge == 0)
		{
			printf("Create new thread failed.\n");
			printf("The server may get its max power.\n");
		}
		else
		{
			flag == FALSE;
		}
	}*/
	Argv.s = MainSocket;
	Argv.flag = flag;
	recv_proc((LPVOID)&Argv);
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
	struct argv *Argv;
	Argv = (struct argv *)lpParam;
	SOCKET MainSocket;
	struct sockaddr_in remote_addr;
	MainSocket = Argv->s;
	u_long arg;
	arg = 0;
	//ioctlsocket(MainSocket, FIONBIO, &arg);
	Linklist*Head,*Current;
	Head = initlinklist(10);
	Current = Head->next;
	for (int j = 0; j < 10; j++)
	{
		ioctlsocket(Units[i].s, FIONBIO, &arg);
	}
	
	while (1)
	{
		if (i < 10)
		{
			int len;
			len = sizeof(remote_addr);
			Units[i].s = accept(MainSocket, (sockaddr*)&remote_addr, &len);
			if (Units[i].s != -1)
			{
				i++;
			}
			if (i == 10)
			{
				flag = FALSE;
			}
		}
		FD_ZERO(&read_list);
		FD_ZERO(&exception_list);
		FD_ZERO(&write_list);
		for (int j=0; j < i; j++)
		{
			FD_SET(Units[j].s, &read_list);
			FD_SET(Units[j].s, &write_list);
			FD_SET(Units[j].s, &exception_list);
		}
		select(0, &read_list, &write_list, &exception_list, &tmo);
		for (int j = 0; j < i; j++)
		{
			if (FD_ISSET(Units[j].s, &read_list))
			{
				char *p = Units[j].buf;
				Units[j].recvnum = recv(Units[j].s, p, sizeof(Units[j].buf),0);
				Current->Unit = Units[j];
				Current = Current->next;
			}
		}
		Current = Head->next;
		for (int j = 0; j < i; j++)
		{
			if (FD_ISSET(Current->Unit.s, &write_list) && Current->Unit.recvnum > 0)
			{
				int sendnum;
				char *p = Current->Unit.buf;
				sendnum = send(Current->Unit.s, p, Current->Unit.recvnum, 0);
				if (sendnum == Current->Unit.recvnum)
				{
					Linklist*outnode = Current;
					Current = Current->next;
					deletenode(Head, outnode);
				}
				else if (sendnum < Current->Unit.recvnum)
				{
					Linklist*outnode = Current;
					Linklist*innode = Current;
					Linklist*endnode=Head;
					Current = Current->next;
					Head=deletenode(Head, outnode);
					while (endnode->next != NULL)
					{
						endnode = endnode->next;
					}
					Head=addnode(Head,endnode, innode);
				}
			}
		}
	}

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