/*
函数名称:gethostbyname
函数原型:
	struct hostent *gethostbyname(const char *hostname)
函数功能:查找对应给定主机名的包含主机名称和地址信息的hostent结构体指针。
函数参数:指向主机名的指针
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
	struct hostent *hptr;
	char str[32];
	char ptr[64] = "www.baidu.com";
	
	hptr = gethostbyname(ptr);
	if (hptr == NULL) {
		cout << "gethostbyname error !" << endl;
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