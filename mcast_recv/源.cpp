//在程序运行之前，需使用NTP协议完成局域网内部的对时操作
//目的：统一集群内各机器的时间轴线，方便机器的同步协调操
//     作.
//方法：查看系统是否安装有NTP对时程序，对NTP server与
//     NTP client 进行相应的配置即可。
#include<iostream>  
#include<WinSock2.h>  
#pragma comment(lib,"Ws2_32.lib")  
#include<ws2tcpip.h>  
#include<Windows.h>
#include<time.h>
using namespace std;
#define MCAST_PORT 8888  
#define MCAST_ADDR "224.0.0.88"  
#define LOCAL_ADDR "192.168.155.136"  
#define MCAST_INTERVAL 1000  
#define MCAST_SIZE 1000

struct Command
{
	uint16_t head;
	uint16_t type;
	uint16_t order;
	SYSTEMTIME time;
	uint16_t tail;

};

void main()
{
	SYSTEMTIME sys;

	//初始化命令数据包
	Command Cmd;
	Cmd.head = 0x01;
	Cmd.type = 0x01;
	Cmd.order = 0x01;
	GetLocalTime(&Cmd.time);
	Cmd.tail = 0xf0;



	SOCKET sock;
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wasData;
	sockaddr_in local_addr;
	int err = WSAStartup(wVersionRequested, &wasData);
	if (err != 0)
	{
		cout << err << endl;
		cout << "套接字版本错误" << endl;
		return;
	}
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MCAST_PORT);
	local_addr.sin_addr.s_addr = inet_addr(LOCAL_ADDR);
	err = bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if (err < 0)
	{
		cout << "bind error" << endl;
		cout << err << endl;
		return;
	}
	bool loop = 1;
	err = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<char FAR *>(&loop), sizeof(loop));
	if (err<0)
	{
		cout << "set sock error" << endl;
		return;
	}
	struct ip_mreq mreq;
	mreq.imr_multiaddr.S_un.S_addr = inet_addr(MCAST_ADDR);
	mreq.imr_interface.s_addr = inet_addr(LOCAL_ADDR);
	err = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char FAR *>(&mreq), sizeof(mreq));
	if (err< 0)
	{
		cout << err << endl;
		cout << "set sock error2" << endl;
		return;
	}
	socklen_t addr_len = 0;
	int times = 0;
	char buff[MCAST_SIZE];
	int n = 0;
	for (times = 0;; times++)
	{
		addr_len = sizeof(local_addr);
		memset(buff, 0, MCAST_SIZE);
		n = recvfrom(sock, buff, MCAST_SIZE, 0, (struct sockaddr*)&local_addr, &addr_len);
		
		//解析数组，重回结构体
		/*char *p = buff;
		Cmd.head = *((uint16_t*)p);
		p + sizeof(uint16_t);
		Cmd.type = *((uint16_t*)p);
		p + sizeof(uint16_t);
		Cmd.order = *((uint16_t*)p);
*/
        //直接使用结构体数据类型解析即可
		Cmd = *(Command*)buff;
		
		GetLocalTime(&sys);
		if (n == -1)
		{
			cout << "cont recv" << endl;
			times = 0;
			return;
		}
		
		cout << times + 1 << " " << buff <<" receive localTime:"<<sys.wMinute<<":"<<sys.wSecond<<"."<<sys.wMilliseconds <<endl;
		
		cout << "Recv Data-order:" << Cmd.order << "    send_time:" << Cmd.time.wMilliseconds << endl;
		//Sleep(MCAST_INTERVAL);




		//延时程序
		DWORD dwStart = GetTickCount();

		DWORD dwEnd = dwStart;

		do

		{

			dwEnd = GetTickCount() - dwStart;

		} while (dwEnd < 1000);
	
	
	}
	err = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char FAR *>(&mreq), sizeof(mreq));
	closesocket(sock);
	return;


}
