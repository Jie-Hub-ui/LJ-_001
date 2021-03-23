/*
一、CA证书及密钥生成法(根证书服务端与客户端需要一致)
	CA证书及密钥生成方法一:直接生成CA密钥及其自签名证书
	如果想以后读取私钥文件ca_rsa_private.pem时不需要输入密码，亦即不对私钥进行加密存储，那么将-passout pass:123456替换成-nodes
		openssl req -newkey rsa:2048 -passout pass:123456 -keyout ca_rsa_private.pem -x509 -days 365 -out ca.crt -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=CA/emailAddress=youremail@qq.com"
	CA证书及密钥生成方法二:分步生成CA密钥及其自签名证书：
		openssl genrsa -aes256 -passout pass:123456 -out ca_rsa_private.pem 2048
		openssl req -new -x509 -days 365 -key ca_rsa_private.pem -passin pass:123456 -out ca.crt -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=CA/emailAddress=youremail@qq.com"

二、客户端证书及密钥生成法
	客户端证书及密钥生成方法一:直接生成客户端密钥及待签名证书
	如果想以后读取私钥文件client_rsa_private.pem时不需要输入密码,亦即不对私钥进行加密存储,那么将-passout pass:client替换成-nodes
		openssl req -newkey rsa:2048 -passout pass:client -keyout client_rsa_private.pem -out client.csr -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=CLIENT/emailAddress=youremail@qq.com"
	客户端证书及密钥生成方法二:分步生成客户端密钥及待签名证书：
		openssl genrsa -aes256 -passout pass:client -out client_rsa_private.pem 2048
		openssl req -new -key client_rsa_private.pem -passin pass:client -out client.csr -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=CLIENT/emailAddress=youremail@qq.com"
	
	使用CA证书及密钥对客户端证书进行签名：
		openssl x509 -req -days 365 -in client.csr -CA ca.crt -CAkey ca_rsa_private.pem -passin pass:123456 -CAcreateserial -out client.crt
	将加密的RSA密钥转成未加密的RSA密钥,避免每次读取都要求输入解密密码
	密码就是生成私钥文件时设置的passout、读取私钥文件时要输入的passin,比如这里要输入"client"
		openssl rsa -in client_rsa_private.pem -out client_rsa_private.pem.unsecure

注意:
	1、注意将其中的私钥加密密码（-passout参数）修改成自己的密码;
	   下边都是以带-passout参数生成私钥,如果使用-nodes参数,则最后一步"将加密的RSA密钥转成未加密的RSA密钥"不需要执行.
	2、证书和密钥给出了直接一步生成和分步生成两种形式,两种形式是等价的,这里使用直接生成形式(分步生成形式被注释).
	3、注意将其中的证书信息改成自己的组织信息的,其中证数各参数含义如下：
		C-----国家(Country Name)
		ST----省份(State or Province Name)
		L----城市(Locality Name)
		O----公司(Organization Name)
		OU----部门(Organizational Unit Name)
		CN----产品名(Common Name)
		emailAddress----邮箱(Email Address)
*/




#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 
#define MAXBUF 1024
 
void ShowCerts(SSL * ssl)
{
	X509 *cert;
	char *line;
 
	cert = SSL_get_peer_certificate(ssl);
	// SSL_get_verify_result()是重点，SSL_CTX_set_verify()只是配置启不启用并没有执行认证，调用该函数才会真证进行证书认证
	// 如果验证不通过，那么程序抛出异常中止连接
	if(SSL_get_verify_result(ssl) == X509_V_OK){
		printf("证书验证通过\n");
	}
	if (cert != NULL) {
		printf("数字证书信息:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("证书: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("颁发者: %s\n", line);
		free(line);
		X509_free(cert);
	} 
	else {
		printf("无证书信息！\n");
	}
}
 
int main(int argc, char **argv)
{
	int sockfd, len;
	struct sockaddr_in dest;
	char buffer[MAXBUF + 1];
	SSL_CTX *ctx;
	SSL *ssl;
	
	if (argc != 5) {
		printf("参数格式错误！正确用法如下：\n\t\t%s IP地址 端口\n\t比如:\t%s 127.0.0.1 80\n此程序用来从某个"
				"IP 地址的服务器某个端口接收最多 MAXBUF 个字节的消息", argv[0], argv[0]);
		exit(0);
	}
 
	// SSL 库初始化,参看 ssl-server.c 代码 
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
 
	// 双向验证
	// SSL_VERIFY_PEER---要求对证书进行认证，没有证书也会放行
	// SSL_VERIFY_FAIL_IF_NO_PEER_CERT---要求客户端需要提供证书，但验证发现单独使用没有证书也会放行
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	// 设置信任根证书(服务端与客户端根证书一致)
	if (SSL_CTX_load_verify_locations(ctx, "ca.crt",NULL)<=0){
		ERR_print_errors_fp(stdout);
		exit(1);
	}
 
	//载入用户的数字证书,此证书用来发送给客户端,证书里包含有公钥
	if (SSL_CTX_use_certificate_file(ctx, argv[3], SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
	// 载入用户私钥 
	if (SSL_CTX_use_PrivateKey_file(ctx, argv[4], SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
	// 检查用户私钥是否正确 
	if (!SSL_CTX_check_private_key(ctx)) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
 
	// 创建一个 socket 用于 tcp 通信
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket");
		exit(errno);
	}
	printf("socket created\n");
 
	// 初始化服务器端(对方)的地址和端口信息
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
		perror(argv[1]);
		exit(errno);
	}
	printf("address created\n");
 
	// 连接服务器 
	if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
		perror("Connect ");
		exit(errno);
	}
	printf("server connected\n");
 
	// 基于 ctx 产生一个新的 SSL 
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockfd);
	// 建立 SSL 连接 
	if (SSL_connect(ssl) == -1)
		ERR_print_errors_fp(stderr);
	else {
		printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
		// 显示服务端证书信息(认证,注释则不认证或变成单项认证)
		ShowCerts(ssl);
	}
 
	// 接收对方发过来的消息，最多接收 MAXBUF 个字节
	bzero(buffer, MAXBUF + 1);
	// 接收服务器来的消息
	len = SSL_read(ssl, buffer, MAXBUF);
	if (len > 0)
		printf("接收消息成功:'%s'，共%d个字节的数据\n", buffer, len);
	else {
		printf("消息接收失败！错误代码是%d，错误信息是'%s'\n", errno, strerror(errno));
		goto finish;
	}
	bzero(buffer, MAXBUF + 1);
	strcpy(buffer, "from client->server");
	// 发消息给服务器 
	len = SSL_write(ssl, buffer, strlen(buffer));
	if (len < 0) {
		printf("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",
				buffer, errno, strerror(errno));
	}
	else {
		printf("消息'%s'发送成功，共发送了%d个字节！\n", buffer, len);
	}
 
	finish:
	// 关闭连接 
	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(sockfd);
	SSL_CTX_free(ctx);
	return 0;
}