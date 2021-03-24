/*
函数名称:gethostbyaddr
函数原型:
	struct hostent *gethostbyaddr(const char *addr, sockelen_t len, int family)
函数功能:通过二进制的IP地址查找到对应主机名,返回一个hostent结构体指针。
函数参数:
	addr:指向网络字节顺序地址的指针
	len :地址长度
	type:地址类型,AF_INET
函数返回值:指向一个hostent结构指针

struct hostent{
	char *h_name;		// 地址的规范名称
	char **h_aliases;	// 空字节,地址的预备名称的指针
	int h_addrtype;		// 地址类型,通常是AF_INET
	int h_length;		// 地址的长度
	char _haddr_list;	// 零字节,主机网络地址,网络字节序(即大端序),只能查找返回ipv4
};

*/

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	char **pptr;
	char str[INET_ADDRSTRLEN];
	char ptr[64] = "127.0.0.1";
	struct hostent *hptr;
	struct in_addr *addr;
	struct sockaddr_in saddr;
	
	if (!inet_aton(ptr, &saddr.sin_addr)) {
		cout << "inet_aton error !" << endl;
		return -1;
	}
	
	hptr = gethostbyaddr((void *)&saddr.sin_addr, 4, AF_INET);
	if (hptr == NULL) {
		cout << "gethostbyaddr error!" << endl;
		return -1;
	}
	
	cout << "hostname = " << hptr->h_name << endl;
	
	for (pptr = hptr->h_aliases; *pptr != NULL ; pptr++) {
		cout << "alias = " << *pptr << endl;
	}
	cout << "addrtype = " << hptr->h_addrtype << endl;
	
	pptr = hptr->h_addr_list;
	for (;*pptr != NULL; pptr++) {
		inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
		cout << "address = " << str << endl;
 	}

	return 0;
}