#include "../socketserver.h"
UserList clientList;
SOCKET primarySock;
char userName[10];
bool recvACK = false;
bool sendMessageTo(char *username, char *message);
void InitWinSock()
{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
 
		wVersionRequested = MAKEWORD( 2, 2 );
 
		err = WSAStartup( wVersionRequested, &wsaData );
		if ( err != 0 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			cout << "winsock fail!"<<endl;
		}
 
		/* Confirm that the WinSock DLL supports 2.2.*/
		/* Note that if the DLL supports versions greater    */
		/* than 2.2 in addition to 2.2, it will still return */
		/* 2.2 in wVersion since that is the version we      */
		/* requested.                                        */
 
		if ( LOBYTE( wsaData.wVersion ) != 2 ||
				HIBYTE( wsaData.wVersion ) != 2 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			WSACleanup( );
			cout << "版本不正确!" <<endl;
		}
	
		/* The WinSock DLL is acceptable. Proceed. */

}

SOCKET mksocket()
{
	SOCKET sock = socket(AF_INET,SOCK_DGRAM,0);
	int error = WSAGetLastError();
	if(error == WSANOTINITIALISED)
	{
		cout << "before socket start up, must call  WSAStartup init!" <<endl;
	}
	if(sock > 0)
	{
		cout << "socket start up successed"<<endl;
		return sock;
	}else
	{
		cout <<" socket create fail!"<<endl;
		exit(1);
	}
}

void connectToServer(SOCKET sock)
{
	 sockaddr_in remote;
	 remote.sin_family = AF_INET;
	 remote.sin_addr.s_addr = inet_addr("127.0.0.1");
	 remote.sin_port = htons(SERVER_PORT);
	 cout << "Enter your nickname:";
	 cin >> userName;
	 stMessage message;
	 message.iMessageType = LOGIN;
	 strcpy(message.message.loginMessage.stUserName,userName);

	 sendto(sock,(const char *)&message,sizeof(stMessage),0,(SOCKADDR *)&remote,sizeof(remote));
	 cout << "login successed..."<<endl;
	 int len  = sizeof(remote);
	 int size = 0;
	 recvfrom(sock,(char *)&size,sizeof(size),0,(SOCKADDR *)&remote,&len);
	 cout << "total client is " << size <<endl;
	 stUserListNode *node = new stUserListNode;
	 for(int i = 0; i < size; ++i)
	 {
		recvfrom(sock,(char *)node,sizeof(stUserListNode),0,(SOCKADDR *)&remote,&len);
		u_long ip = htonl(node->ip);
		cout << "username :" << node->stUserName <<" ip:"<<inet_ntoa(*(in_addr *)&ip) <<" port:"<<node->port <<endl;
		clientList.push_back(node);
	 }
}
//接收p2p信息
void P_2_P_Message(int length)
{
	sockaddr_in remote;
	int len = sizeof(remote);
	char *message = new char[length];
	recvfrom(primarySock,message,length,0,(SOCKADDR *)&remote,&len);
	message[length-1]='\0';
	cout << message <<endl;
	delete []message;
	stP2PMessage p_message;
	p_message.iMessageType = P2PMESSAGEACK;
	sendto(primarySock,(const char *)&p_message,sizeof(stP2PMessage),0,(SOCKADDR *)&remote,len);
}
//尝试p2p连接
void p_2_p_SomeoneWanttoCall(stP2PMessage message)
{
	sockaddr_in remote;
	remote.sin_addr.S_un.S_addr = htonl(message.iStringLen);
	remote.sin_port = htons(message.Port);
	remote.sin_family = AF_INET;
	stP2PMessage p_message;
	p_message.iMessageType = P2PTRASH;
	sendto(primarySock,(const char *)&p_message,sizeof(stP2PMessage),0,(SOCKADDR *)&remote,sizeof(remote));
}
//得到所有用户列表
void getAllUser()
{
	clientList.clear();
	sockaddr_in remote;
	int len  = sizeof(remote);
	int size = 0;
	recvfrom(primarySock,(char *)&size,sizeof(size),0,(SOCKADDR *)&remote,&len);
	cout << "total client is " << size <<endl;
	
	for(int i = 0; i < size; ++i)
	{
		stUserListNode *node = new stUserListNode;
		recvfrom(primarySock,(char *)node,sizeof(stUserListNode),0,(SOCKADDR *)&remote,&len);
		u_long ip = htonl(node->ip);
		cout << "username :" << node->stUserName <<" ip:"<<inet_ntoa(*(in_addr *)&ip) <<" port:"<<node->port <<endl;
		clientList.push_back(node);
	}
}
DWORD WINAPI RecvThreadProc(LPVOID lpParameter)
{
	sockaddr_in remote;
	int len = sizeof(remote);
	stP2PMessage p_message;
	while(1)
	{
		recvfrom(primarySock,(char *)&p_message,sizeof(stP2PMessage),0,(SOCKADDR *)&remote,&len);
		switch(p_message.iMessageType)
		{
		case P2PMESSAGE:
			P_2_P_Message(p_message.iStringLen);
		  break;
		case P2PSOMEONEWANTTOCALLYOU:
			p_2_p_SomeoneWanttoCall(p_message);
			break;
		case P2PTRASH:
			// 对方发送的打洞消息，忽略掉。
			//do nothing ...
			{
				cout << "ip:" << inet_ntoa(remote.sin_addr) << "port:"<<ntohs(remote.sin_port)<<endl;
				cout << "Recv p2ptrash data" <<endl;
				break;
			}
			
		case P2PMESSAGEACK:
			recvACK = true;
			break;
		case GETUSERLIST:
			getAllUser();
			break;
		}
	}
}

void OutputUsage()
{
	cout<<"You can input you command:\n"
		<<"Command Type:\"send\",\"exit\",\"getu\"\n"
		<<"Example : send Username Message\n"
		<<"          exit\n"
		<<"          getu\n"
		<<endl;
}
//解析指令
void parseCommand(char *CommandLine)
{
	if(strlen(CommandLine)<4)
		return;
	char Command[10];
	strncpy(Command, CommandLine, 4);
	Command[4]='\0';
	
	if(strcmp(Command,"exit")==0)
	{
		stMessage sendbuf;
		sendbuf.iMessageType = LOGOUT;
		strcpy(sendbuf.message.loginoutMessage.stUserName,userName);
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);
		
		sendto(primarySock,(const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr *)&server, sizeof(server));
		shutdown(primarySock, 2);
		closesocket(primarySock);
		exit(0);
	}
	else if(strcmp(Command,"send")==0)
	{
		char sendname[20];
		char message[COMMANDMAXC];
		int i;
		for(i=5;;i++)
		{
			if(CommandLine[i]!=' ')
				sendname[i-5]=CommandLine[i];
			else
			{
				sendname[i-5]='\0';
				break;
			}
		}
		strcpy(message, &(CommandLine[i+1]));
		if(sendMessageTo(sendname, message))
			printf("Send OK!\n");
		else 
			printf("Send Failure!\n");
	}
	else if(strcmp(Command,"getu")==0)
	{
		stMessage command;
		command.iMessageType = GETUSERLIST;
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);
		
		sendto(primarySock,(const char*)&command, sizeof(command), 0, (const sockaddr *)&server, sizeof(server));
	}

}
//查询指定用户
stUserListNode* getUser(char *name)
{
	stUserListNode *node = NULL;
	stUserListNode *removeNode = NULL;
	for(UserList::iterator UserIterator = clientList.begin(); UserIterator != clientList.end(); ++UserIterator)
	{
		node = (stUserListNode *)(*UserIterator);
		if(strcmp(node->stUserName,name) == 0)
		{
			removeNode = node;
			break;
		}
	}
	return removeNode;
}
//p2p发送信息
bool sendMessageTo(char *username, char *message)
{
	stUserListNode *node = getUser(username);
	sockaddr_in remote;
	remote.sin_addr.S_un.S_addr = htonl(node->ip);
	remote.sin_port = htons(node->port);
	remote.sin_family = AF_INET;

	stP2PMessage p_message;
	p_message.iMessageType = P2PMESSAGE;
	p_message.iStringLen = (int)strlen(message) + 1;

	for(int i = 0; i < MAXRETRY; ++i)
	{
		recvACK = false;
		sendto(primarySock,(const char *)&p_message,sizeof(stP2PMessage),0,(SOCKADDR *)&remote,sizeof(remote));
		sendto(primarySock,(const char *)message,strlen(message),0,(SOCKADDR *)&remote,sizeof(remote));
		for(int j = 0; j < 3; ++j)
		{
			if(recvACK)
				return true;
			else
				Sleep(300);
		}

		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		server.sin_port = htons(SERVER_PORT);
		server.sin_family = AF_INET;
		stMessage m_message;
		m_message.iMessageType = P2PTRAN;
		strcpy(m_message.message.translateMessage.stUserName,username);
		sendto(primarySock,(const char *)&m_message,sizeof(stMessage),0,(SOCKADDR *)&server,sizeof(server));
		Sleep(100);
	}
	return false;
}
void main()
{
	InitWinSock();
	primarySock = mksocket();
	connectToServer(primarySock);
	
	HANDLE threadhandle = CreateThread(NULL, 0, RecvThreadProc, NULL, NULL, NULL);
	CloseHandle(threadhandle);
	OutputUsage();
	while(1)
	{
		char command[100];
		gets(command);
		parseCommand(command);
	}
	getchar();
}