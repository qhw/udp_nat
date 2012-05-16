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

//客户端登录时向服务器发送的信息
struct stLoginMessage
{
	char stUserName[10];
	char passWord[10];
};
//客户端登出时向服务器发送的信息
struct stLogoutMessage
{
	char stUserName[10];
};
//P2P时发送的用户名
struct stP2PTranslate
{
	char stUserName[10];
};
//客户端向服务器发送的消息格式
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
//客户节点信息
struct stUserListNode
{
	char stUserName[10];
	unsigned int ip;
	unsigned short port;
};


// Server向Client发送的消息
struct stServerToClient
{
	int iMessageType;
	union _message
	{
		stUserListNode user;
	}message;
	
};

//======================================
// 下面的协议用于客户端之间的通信
//======================================
#define P2PMESSAGE 100               // 发送消息
#define P2PMESSAGEACK 101            // 收到消息的应答
#define P2PSOMEONEWANTTOCALLYOU 102  // 服务器向客户端发送的消息
// 希望此客户端发送一个UDP打洞包
#define P2PTRASH        103          // 客户端发送的打洞包，接收端应该忽略此消息

// 客户端之间发送消息格式
struct stP2PMessage
{
	int iMessageType;
	int iStringLen;         // or IP address
	unsigned short Port; 
};
typedef list<stUserListNode *> UserList;
#endif