 -----------------------------------------------------
|                  DocKer容器管理                     |
|                                                     |
 -----------------------------------------------------
 
一、单一容器管理
	1、容器标识
		每个容器创建后都会分配一个CONTAINER ID作为容器唯一标识,后续所有操作均是通过容器ID操作完成。
		CONTAINER ID默认128位,对于大多数主机来讲,ID的前16位足以保证其在本机唯一性,默认情况使用简写的前16位,
		查看完整ID可以使用:docker ps --no-trunc
		查看容器状态:
			docker ps -a | grep CONTAINERID
	2、容器内部命令
		可以登录Docker容器内部执行命令,可以再容器中启动sshd服务来响应用户登录。但是sshd方式存在进程开销大、
	增加被攻击的风险,同时违反了Docker倡导的一个容器一个进程的原则。
		Docker提供了原生的方式登录容器:
			docker exec [容器名] [容器内执行的命令]
		例如查看 MyWorldPress 容器内启动了那些进程:
			docker exec MyWorldPress ps aux
		如果希望在容器中连续执行多条命令可以加上 -it 参数,相当于以root的身份登录容器内,执行完毕后通过 exit 退出。
二、多容器管理
	1、Docker倡导"一个容器一个进程"的理念,但是一个服务由多个进程组成,就需要创建多个容器组成一个系统,相互分工配合对外提供完整服务。
	   docker run命令提供 "--link" 选项建立容器之间的相互关联,但有一个前提条件,使用 "--link containerA" 创建容器B时,容器 A 已经创建
	   完成并且运行。所以容器启动是按顺序的。
	   
	2、Docker Compose(智能编排工具)
		Docker 提供一个智能编排工具,允许用户在一个模板(YAML格式)中定义一组相关联的应用容器,这组容器会根据模板中的 "--link" 等参数对启动
		的优先级自动排序,简单执行 "docker-compose up" 就可以把同一个服务中的多个容器依次创建和启动。
		安装:
			sudo curl -L https://github.com/docker/compose/releases/download/1.6.0/docker-compose-`uname -s`-`uname -m` > /docker-compose
			sudo chmod +x /docker-compose
		使用:
			创建一个 docker-compose.yml 的文件,内容就是一个服务所需要启动的容器,例:
				wordpress:           // 容器名称,最终创建的容器名称: wordpress_db_1
					image:wordpress  // 镜像名称
					links:
						- db:mysql   // 依赖的容器
					ports:
						- 8080:80    // 依赖的容器端口映射
				db:                  // 容器名称,最终创建的容器名称: db_db_1
					image:db         // 镜像名称
					environment:
						MYSQL_ROOT_PASSEORD:example  // 环境变量设置
				...
			创建和启动服务:
				docker-compose up
				此时docker-compose 会自动根据顺序创建和启动容器服务。
			后续启动容器
				docker-compose start   // 根据对应顺序启动
			关闭容器
				docker-compose stop
			
		Docker compose仅仅是启动容器的顺序,至于容器内的进程是否在被其他容器中的进程使用时启动不一定。
		通过配置文件可以对该项目中的容器进行查询、启动、停止等操作:
			查看所有容器状态:
				docker-compose -f docker-compose.yml ps
			停止项目中所有容器:
				docker-compose -f docker-compose.yml stop
			启动项目中所有容器:
				docker-compose -f docker-compose.yml start
			删除项目中所有容器:
				docker-compose -f docker-compose.yml down

三、实例
	
 
 
 
 
 