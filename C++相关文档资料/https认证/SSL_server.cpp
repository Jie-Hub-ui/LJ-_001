/*
一、CA证书及密钥生成法(根证书服务端与客户端需要一致)
	CA证书及密钥生成方法一:直接生成CA密钥及其自签名证书
	如果想以后读取私钥文件ca_rsa_private.pem时不需要输入密码，亦即不对私钥进行加密存储，那么将-passout pass:123456替换成-nodes
		openssl req -newkey rsa:2048 -passout pass:123456 -keyout ca_rsa_private.pem -x509 -days 365 -out ca.crt -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=CA/emailAddress=youremail@qq.com"
	CA证书及密钥生成方法二:分步生成CA密钥及其自签名证书：
		openssl genrsa -aes256 -passout pass:123456 -out ca_rsa_private.pem 2048
		openssl req -new -x509 -days 365 -key ca_rsa_private.pem -passin pass:123456 -out ca.crt -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=CA/emailAddress=youremail@qq.com"

二、服务端证书及密钥生成法
	服务器证书及密钥生成方法一:直接生成服务器密钥及待签名证书
	如果想以后读取私钥文件server_rsa_private.pem时不需要输入密码，亦即不对私钥进行加密存储，那么将-passout pass:server替换成-nodes
		openssl req -newkey rsa:2048 -passout pass:server -keyout server_rsa_private.pem -out server.csr -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=SERVER/emailAddress=youremail@qq.com"
	服务器证书及密钥生成方法二:分步生成服务器密钥及待签名证书
		openssl genrsa -aes256 -passout pass:server -out server_rsa_private.pem 2048
		openssl req -new -key server_rsa_private.pem -passin pass:server -out server.csr -subj "/C=CN/ST=GD/L=SZ/O=COM/OU=NSP/CN=SERVER/emailAddress=youremail@qq.com"
	
	使用CA证书及密钥对服务器证书进行签名：
		openssl x509 -req -days 365 -in server.csr -CA ca.crt -CAkey ca_rsa_private.pem -passin pass:123456 -CAcreateserial -out server.crt
	将加密的RSA密钥转成未加密的RSA密钥，避免每次读取都要求输入解密密码
	密码就是生成私钥文件时设置的passout、读取私钥文件时要输入的passin，比如这里要输入"server"
		openssl rsa -in server_rsa_private.pem -out server_rsa_private.pem.unsecure

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
		
demo执行:
	./server server.crt server_rsa_private.pem.unsecure
	server.crt:数字证书,用来发送给客户端,证书中包含公钥
	server_rsa_private.pem.unsecure:用户私钥
*/




#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 
#define MAXBUF 1024
 
void ShowCerts(SSL * ssl)
{
	X509 *cert;
	char *line;
 
	cert = SSL_get_peer_certificate(ssl);
	// SSL_get_verify_result()是重点,SSL_CTX_set_verify()只是配置启不启用并没有执行认证,调用该函数才会真证进行证书认证
	// 如果验证不通过，那么程序抛出异常中止连接
	if (SSL_get_verify_result(ssl) == X509_V_OK){
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
	int sockfd, new_fd;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int myport = 5555;
	char buf[MAXBUF + 1];
	SSL_CTX *ctx;
 
	// SSL 库初始化 
	SSL_library_init();
	// 载入所有 SSL 算法 
	OpenSSL_add_all_algorithms();
	// 载入所有 SSL 错误消息
	SSL_load_error_strings();
	// 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX,即 SSL Content Text 
	ctx = SSL_CTX_new(SSLv23_server_method());
	// 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 
	if (ctx == NULL) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
 
	// 双向验证
	// SSL_VERIFY_PEER---要求对证书进行认证，没有证书也会放行
	// SSL_VERIFY_FAIL_IF_NO_PEER_CERT---要求客户端需要提供证书，但验证发现单独使用没有证书也会放行
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
  
	// 设置信任根证书(服务端与客户端需要一致)
	if (SSL_CTX_load_verify_locations(ctx, "ca.crt",NULL)<=0){
		ERR_print_errors_fp(stdout);
		exit(1);
	}
 
	// 载入用户的数字证书,此证书用来发送给客户端,证书里包含有公钥
	if (SSL_CTX_use_certificate_file(ctx, argv[1], SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
  
	// 载入用户私钥
	if (SSL_CTX_use_PrivateKey_file(ctx, argv[2], SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
	// 检查用户私钥是否正确 
	if (!SSL_CTX_check_private_key(ctx)) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
 
	// 开启一个socket监听 
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	} 
	else {
		printf("socket created\n");
	}
	

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons(myport);
	my_addr.sin_addr.s_addr = INADDR_ANY;
 
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	} 
	else {
		printf("binded\n");
	}
 
	if (listen(sockfd, lisnum) == -1) {
		perror("listen");
		exit(1);
	} 
	else {
		printf("begin listen\n");
	}
 
	while (1) {
		SSL *ssl;
		len = sizeof(struct sockaddr);
		// 等待客户端连上来
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len)) == -1) {
		perror("accept");
		exit(errno);
		} 
		else {
			printf("server: got connection from %s, port %d, socket %d\n",
			inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
		}
 
		// 基于 ctx 产生一个新的 SSL 
		ssl = SSL_new(ctx);
		// 将连接用户的 socket 加入到 SSL 
		SSL_set_fd(ssl, new_fd);
		// 建立 SSL 连接 
		if (SSL_accept(ssl) == -1) {
			perror("accept");
			close(new_fd);
			break;
		}
		// 显示客户端证书信息(认证,注释则不认证或变成单项认证)
		ShowCerts(ssl);
 
		// 开始处理每个新连接上的数据收发 
		bzero(buf, MAXBUF + 1);
		strcpy(buf, "server->client");
		// 发消息给客户端 
		len = SSL_write(ssl, buf, strlen(buf));
 
		if (len <= 0) {
			printf("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n", buf, errno,
			strerror(errno));
			goto finish;
		} 
		else {
			printf("消息'%s'发送成功，共发送了%d个字节！\n", buf, len);
		}

		bzero(buf, MAXBUF + 1);
		// 接收客户端的消息 
		len = SSL_read(ssl, buf, MAXBUF);
		if (len > 0) {
			printf("接收消息成功:'%s'，共%d个字节的数据\n", buf, len);
		}
		else {
			printf("消息接收失败！错误代码是%d，错误信息是'%s'\n", errno, strerror(errno));
		}
		// 处理每个新连接上的数据收发结束 
		finish:
		// 关闭 SSL 连接
		SSL_shutdown(ssl);
		// 释放 SSL
		SSL_free(ssl);
		// 关闭 socket 
		close(new_fd);
	}
	// 关闭监听的 socket
	close(sockfd);
	// 释放 CTX 
	SSL_CTX_free(ctx);
	return 0;
}