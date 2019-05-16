#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <process.h>
#include <windows.h>
#pragma comment (lib, "wsock32.lib")
struct unit
{
	SOCKET s;
	char buf[100000];
	int recvnum;
};

typedef struct Unit
{
	struct unit;
	struct Unit *front;
	struct Unit *next;
}Linklist;

int main()
{
	Linklist *Head, *node;
	Head = (Linklist*)malloc(sizeof(Unit));
	Head->front = NULL;
	Linklist*head = Head;
	for (int i = 0; i < 10; i++)
	{
		node = (Linklist*)malloc(sizeof(Unit));
		head->next = node;
		node->front = head;
		head = node;
	}
	head->next = NULL;
	return 0;
}