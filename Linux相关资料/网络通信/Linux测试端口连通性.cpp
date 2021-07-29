一、Linux测试端口的连通性的四种方法
1、telnet
2、ssh
3、crul
4、wget

方法一、telnet
telnet 为用户提供了在本地计算机上完成远程主机工作的能力,因此可以通过telnet来测试端口的连通性,具体用法格式:
	telnet ip port
	说明：
		ip:是测试主机的ip地址
		port:是端口,比如80

方法二、ssh
SSH 是目前较可靠,专为远程登录会话和其他网络服务提供安全性的协议,在linux上可以通过ssh命令来测试端口的连通性,具体用法格式如下:
	ssh -v -p port username@ip
	说明：
		-v 调试模式(会打印日志)
		-p 指定端口
		username:远程主机的登录用户
		ip:远程主机

方法三、curl
curl 是利用URL语法在命令行方式下工作的开源文件传输工具.也可以用来测试端口的连通性,具体用法:
	curl ip:port
	说明：
		ip:是测试主机的ip地址
		port:是端口,比如80
	如果远程主机开通了相应的端口,都会输出信息;如果没有开通相应的端口,则没有任何提示,需要CTRL+C断开。

方法四、wget
wget 是一个从网络上自动下载文件的自由工具,支持通过HTTP、HTTPS、FTP三个最常见的TCP/IP协议下载,并可以使用HTTP代理。
	wget ip:port
	说明：
		ip:是测试主机的ip地址
		port:是端口，比如80
		如果远程主机不存在端口则会一直提示连接主机。
		
二、Linux显示tcp、udp的端口和进程等情况
1、netstat 查看端口占用语法格式：
	netstat -tunlp | grep 端口号
	说明：
		-t (tcp) 仅显示tcp相关选项
		-u (udp)仅显示udp相关选项
		-n 拒绝显示别名，能显示数字的全部转化为数字
		-l 仅列出在Listen(监听)的服务状态
		-p 显示建立相关链接的程序名
