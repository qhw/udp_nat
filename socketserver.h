#ifndef __SOCKET_H__
#define __SOCKET_H__
#include<WinSock2.h>
#include<Windows.h>
#include<list>
#include<iostream>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

#define COMMANDMAXC 256
#define MAXRETRY    5

#define LOGIN 0
#define LOGOUT 1
#define P2PTRAN 2
#define GETUSERLIST 3

#define SERVER_PORT 1988

//�ͻ��˵�¼ʱ����������͵���Ϣ
struct stLoginMessage
{
	char stUserName[10];
	char passWord[10];
};
//�ͻ��˵ǳ�ʱ����������͵���Ϣ
struct stLogoutMessage
{
	char stUserName[10];
};
//P2Pʱ���͵��û���
struct stP2PTranslate
{
	char stUserName[10];
};
//�ͻ�������������͵���Ϣ��ʽ
struct stMessage
{
	int iMessageType;
	union _message
	{
		stLoginMessage loginMessage;
		stLogoutMessage loginoutMessage;
		stP2PTranslate translateMessage;
	}message;
};
//�ͻ��ڵ���Ϣ
struct stUserListNode
{
	char stUserName[10];
	unsigned int ip;
	unsigned short port;
};


// Server��Client���͵���Ϣ
struct stServerToClient
{
	int iMessageType;
	union _message
	{
		stUserListNode user;
	}message;
	
};

//======================================
// �����Э�����ڿͻ���֮���ͨ��
//======================================
#define P2PMESSAGE 100               // ������Ϣ
#define P2PMESSAGEACK 101            // �յ���Ϣ��Ӧ��
#define P2PSOMEONEWANTTOCALLYOU 102  // ��������ͻ��˷��͵���Ϣ
// ϣ���˿ͻ��˷���һ��UDP�򶴰�
#define P2PTRASH        103          // �ͻ��˷��͵Ĵ򶴰������ն�Ӧ�ú��Դ���Ϣ

// �ͻ���֮�䷢����Ϣ��ʽ
struct stP2PMessage
{
	int iMessageType;
	int iStringLen;         // or IP address
	unsigned short Port; 
};
typedef list<stUserListNode *> UserList;
#endif