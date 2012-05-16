#include "../socketserver.h"

UserList clientList;
SOCKET primarySock;
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
			cout << "Init winsock fail!"<<endl;
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
 /**
	用户登陆时，记录用户信息
	并输出所有登陆的用户信息
*/
void clientLogin(stMessage *message, sockaddr_in *sender)
{
	cout << "client connected :" << message->message.loginMessage.stUserName <<endl;
	stUserListNode *node = new stUserListNode;
	strcpy(node->stUserName,message->message.loginMessage.stUserName);
	node->ip = ntohl(sender->sin_addr.S_un.S_addr);
	
	node->port = ntohs(sender->sin_port);
	clientList.push_back(node);
	cout << "total client is " << clientList.size() <<endl;
	for(UserList::iterator UserIterator = clientList.begin(); UserIterator != clientList.end(); ++UserIterator)
	{
		stUserListNode *tmpNode = (stUserListNode *)(*UserIterator);
		u_long ip = htonl(tmpNode->ip);
		cout << "username :" << tmpNode->stUserName <<" ip:"<<inet_ntoa(*(in_addr *)&ip) <<" port:"<<tmpNode->port <<endl;
	}
}
//向客户端发送所有的用户信息
void getUserList(sockaddr_in sender)
{
	int len = (int)clientList.size();
	sendto(primarySock,(const char *)&len,sizeof(len),0,(SOCKADDR *)&sender,sizeof(sender));
	for(UserList::iterator UserIterator = clientList.begin(); UserIterator != clientList.end(); ++UserIterator)
	{
		sendto(primarySock,(const char *)(*UserIterator),sizeof(stUserListNode),0,(SOCKADDR *)&sender,sizeof(sender));
	}
}

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
void removeUser(char *name)
{
	stUserListNode *removeNode;
	
	removeNode = getUser(name);
	clientList.remove(removeNode);
	for(UserList::iterator UserIterator = clientList.begin(); UserIterator != clientList.end(); ++UserIterator)
	{
		stUserListNode *tmpNode = (stUserListNode *)(*UserIterator);
		u_long ip = htonl(tmpNode->ip);
		cout << "username :" << tmpNode->stUserName <<" ip:"<<inet_ntoa(*(in_addr *)&ip) <<" port:"<<tmpNode->port <<endl;
	}
}

void P2PTran(char *name, SOCKET sock, sockaddr_in sender)
{
	stUserListNode *node = getUser(name);
	sockaddr_in remote;
	remote.sin_family = AF_INET;
	remote.sin_addr.S_un.S_addr = htonl(node->ip);
	remote.sin_port = htons(node->port);
	stP2PMessage m_message;
	m_message.iMessageType = P2PSOMEONEWANTTOCALLYOU;
	m_message.iStringLen = htonl(sender.sin_addr.S_un.S_addr);
	m_message.Port = htons(sender.sin_port);
	sendto(sock,(const char *)&m_message,sizeof(stP2PMessage),0,(const SOCKADDR *)&remote,sizeof(remote));
}
void main()
{
	InitWinSock();
	primarySock = mksocket();
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl(INADDR_ANY);
	service.sin_port = htons(SERVER_PORT);

	 if (bind( primarySock, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) 
	 {
		printf("bind() failed.\n");
		closesocket(primarySock);
		return;
	  }

	 sockaddr_in sender;
	 int senderlen = sizeof(sender);
	 stMessage recvMessage;

	 cout <<"recving..."<<endl;
	 for(;;)
	 {
		 int iret = recvfrom(primarySock,(char*)&recvMessage,sizeof(stMessage),0,(SOCKADDR *)&sender,&senderlen);
		 if(iret < 0)
		 {
			 cout << "recv data error!"<<endl;
		 }else
		 {
			 int type = recvMessage.iMessageType;
			 switch(type)
			 {
			case LOGIN:
				clientLogin(&recvMessage,&sender);
				getUserList(sender);
				break;
			case LOGOUT:
				removeUser(recvMessage.message.loginoutMessage.stUserName);
				break;
			case GETUSERLIST:
				{
					stP2PMessage command;
					command.iMessageType = GETUSERLIST;
					sendto(primarySock,(const char *)&command,sizeof(stP2PMessage),0,(const SOCKADDR *)&sender,senderlen);
					getUserList(sender);
					break;
				}
			case P2PTRAN:
				P2PTran(recvMessage.message.translateMessage.stUserName,primarySock,sender);
				break;
			 }
		 }
	 }

	getchar();
}