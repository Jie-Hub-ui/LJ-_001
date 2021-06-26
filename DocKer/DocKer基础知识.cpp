 -----------------------------------------------------
|                  DocKer基础知识                     |
|                                                     |
 -----------------------------------------------------
 
一、DocKer基本概念和常用操作指令
	1.1 DocKer三大组件
		DocKer有三个重要的概念:仓库(Repository)、镜像(Image)、容器(Container)
		仓库(Repository):DocKer仓库地址：https://hub.docker.com, 仓库中有各大厂商打包的应用,也有大量个人开发者提供的应用;
							我们可以将自己的应用程序打包后上传到仓库中。
		镜像(Image):镜像其实就是仓库中打包好的应用程序,用户可以搜索自己需要下载的镜像下载到本地,有点类似于Windows里面的安装包。
					Docker官方镜像仓库存储着大量Docker化应用镜像,我们可以基于Docker官方仓库的镜像创建我们的应用。
		容器(Container):安装好DocKer后,运行我们从仓库中下载的镜像(即我们的应用),每个运行中的镜像都叫做容器;容器都是基于镜像
							创建的,基于一个镜像可以创建若干个名字不同但功能相同的容器。我们可以把容器理解为在一个相对独立环境中运行一个(组)进程，
							这个独立的环境拥有这个(组)进程运行所需一切,包括文件系统、库文件、shell脚本等。容器中镜像修改操作,修改内容会保存在容器中。
							
	1.2 centos下安装DocKer
		sudo yum install -y curl
		curl -sSL https://get.docker.com/ | sh
		检查是否安装成功:
			sudo docker run hello-world   // 运行一个应用检测
			
	1.3 DocKer运行说明
		DocKer运行一个镜像,首先会在本机查找有没有对应的镜像,如果没有:就到DocKer仓库中查找该镜像,然后下载至本机运行。
	
	1.4 Docker组织结构
		Docker位于操作系统和虚拟容器之上,通过调用cgropu、namespaces、libcontainer等系统层面的接口来完成资源的分配和隔离。
		在一台主机上首先启动一个守护进程(Docker Daemon),所有容器都被守护进程控制,同时守护进程监听并接受Docker客户端指令,并将
	执行结果返回给Docker客户端。
		Docker引擎有两部分:Daemon和Client。
			Daemon:Service端的守护进程,接收客户端指令,管理本机上的镜像和容器;
			Client:通过Docker命令和Daemon交互,对Docker的镜像和容器查询、添加、修改、启动、停止等操作。
			
	
二、DocKer常用指令
	2.1 docker run [imagename] [...]
		运行一个镜像。至少需要两个参数,一个是镜像名,一个是在容器中需要运行的命令。
	2.2 查看所有运行中的docker进程(即容器):
			docker ps
		查看单个容器详细信息:
			docker inspect containerID
			说明:containerID也可只指定前3~4个字符
	2.3 docker
		查看docker的指令用法级支持的指令。
	2.4 docker基本用法
		docker + 命令关键字(COMMAND) + 一系列的参数([arg...])
		例：docker run --name MyWordPress --Link db:mysql -p 8080:80 -d wordpress
			run是关键字,后面的内容全是参数。
	2.5 查看某个关键字命令支持哪些参数(COMMAND为关键字)
		docker COMMAND --help
	2.6 操作对象主要针对四个方面
		2.6.1 守护进程的系统资源设置和全局信息的获取,如：docker info、docker deamon ...
		2.6.2 docker仓库的查询、下载,如:docker search、docker pull ...
		2.6.3 docker镜像的查询、创建、删除操作,如:docker images、docker build ...
		2.6.4 docker容器的查询、创建、开启、停止操作,如:docker ps、docker run ...
	2.7 查询Docker版本
		docker version
	2.8 通过名称查找镜像
		docker search [ImageName]           // 查询所有名叫ImageName的镜像
		docker search [username/ImageName]  // 查询 username 用户下 ImageName 的镜像
		说明:Docker官方镜像仓库中镜像全名称格式:<username>/<ImageName>
			每个用户都可以在Docker官方镜像仓库注册自己的账户,发布自己的镜像,通过"用户名+镜像名"的方式可以让不同用户
			拥有相同的镜像名而互不干扰。
	2.9 下载镜像
		docker pull [username/ImageName]
	2.10 创建新镜像(Docker支持在原有镜像基础上,只提交增量修改部分,形成一个新镜像)
		显示本机上所有容器:
			docker ps -l
		将修改后过的容器提交为镜像:
			docker commit containerID(容器ID) username/ImageNewName
			containerID:需要提交为镜像的容器ID
			username/ImageNewName:新建镜像名称(username为自己用户名)
	2.11 查询本机上所有镜像
		docker images
	2.12 查看停止的容器状态:
			docker ps -a | grep CONTAINERID
	2.13 停止容器	
			docker stop CONTAINERID     或者
			docker stop CONTAINERNAME
	2.14 启动运行容器
			docker start CONTAINERID    或者
			docker start CONTAINERNAME
	2.15 查询容器日志
			docker logs CONTAINERNAME
		 说明:若需要实时打印最新日志,可以加上-f选项
	2.16 查看容器所占系统资源
			docker stats CONTAINERNAME
		
三、实践例子
	1、查询镜像
		docker search tutorial
		docker search learn/tutorial  // 查询learn用户下的tutorial镜像
	2、下载镜像
		docker pull learn/tutorial
	3、创建并启动容器
		docker run learn/tutorial echo "hello world"
		说明:docker run learn/tutorial表示启动 tutorial 镜像为模板的一个容器
			 echo "hello world" :表示在该容器中执行的命令,即输出一句 hello world
	4、修改容器
		docker run learn/tutorial apt-get install -y ping
		说明:在容器中安装一个ping工具(此处镜像是基于Ubuntu操作系统构建的,所以需要使用apt-get),一旦ping软件包安装成功,容器立刻
			停止,但是容器中安装的软件包会一直保留。
	5、创建新镜像
		查看本机上所有容器:
			docker ps -l
		将修改的容器提交为新镜像:
			docker commit containerID username/ImageNewName
			说明:containerID:需要提交为镜像的容器ID
				username/ImageNewName:新建镜像名称(username为自己用户名)
	6、使用新镜像
		基于容器生成的镜像包含ping软件包,可以运行在任何安装有Docker引擎的机器上。
		docker run username/ImageNewName ping www.baidu.coms
	7、查询容器信息
		查看所有运行中的容器:
			docker ps   
		查看单个容器详细信息:
			docker inspect containerID
	8、上传新镜像到Docker仓库
		查看本机上所有镜像
			docker images
		推送新建的镜像到仓库
			docker push [username/ImageNewName]
	说明:可以自行在DockerHub上注册一个用户
		https://hub.docker.com/account/signup
			
							
							