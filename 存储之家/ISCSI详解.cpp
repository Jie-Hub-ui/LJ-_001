ISCSI 详解

1、iscsi概述
	 iSCSI(Internet Small Computer System Interface,Internet 小型计算机系统接口)是一种由IBM公司研究开发的IP SAN技术，
该技术是将现有SCSI接口与以太网络(Ethernet)技术结合,基于 TCP/IP的协议连接iSCSI服务端(Target)和客户端(Initiator),使得
封装后的SCSI数据包可以在通用互联网传输,最终实现iSCSI服务端映射为一个存储空间(磁盘)提供给已连接认证后的客户端。
	iscsi 实现网络存储，提供存储端叫target,使用存储端叫initiator。target上可以提供存储空间,initiator负责连接ISCSI设备，
在ISCSI设备中创建文件系统,以及存取数据,在initiator上看上去是多了一块硬盘。
1.1 SCSI
	SCSI(Small Computer System Interface)是一种I/O技术，规范了一种并行的I/O总线和相关的协议，SCSI的数据传输是以块的方式进行的。
	SCSI总线通过SCSI控制器来和硬盘之类的设备进行通信,SCSI控制器称为Target,访问的客户端应用称为Initiator。窄SCSI总线最多允许8个、
宽SCSI总线最多允许16个不同的SCSI设备和它进行连接,每个SCSI设备都必须有自己唯一的SCSI ID(设备的地址)。
	
	LUN(Logical Unit Number，逻辑单元号)是为了使用和描述更多设备及对象而引进的一个方法,每个SCSI ID上最多有32个LUN,一个LUN对应一个逻辑设备。
	广泛应用于小型机上,正在成为PC 服务器的标准接口,实现高速数据传输(可达320MB/s),常见的SCSI设备:硬盘、磁盘阵列、打印机、光盘刻录机等。
	
1.2 ISCSI实现
	1.2.1 iSCSI Initiator
		iSCSI启动器,从本质上说,iSCSI启动器是一个客户端设备,用于将请求连接并启动到服务器(iSCSI 目标)。
		iSCSI 启动器有三种实现方式:可以完全基于硬件实现,比如 iSCSI HBA 卡;硬件TOE卡与软件结合的方式;
	完全基于软件实现,而软件iSCSI启动器适用于大部分主流操作系统平台。
	
	1.2.2 iSCSI Target
		即iSCSI目标,它是iSCSI网络的服务器组件,通常是一个存储设备,用于包含所需的数据并回应来自iSCSI启动器的请求。

1.3 工作过程
	Initiator发出请求后,会在本地的操作系统会生成了相应的SCSI命令和数据I/O请求,然后这些命令和请求被封装加密成IP信息包,通过以太网(TCP/IP)传输到Targer。
	当Targer接收到信息包时,将进行解密和解析,将SCSI命令和I/O请求分开。SCSI命令被发送到SCSI控制器,再传送到SCSI存储设备。
	设备执行SCSI命令后的响应,经过Target封装成iSCSI响应PDU,再通过已连接的TCP/IP网络传送给Initiator。
	Initiator会从iSCSI响应PDU里解析出SCSI响应并传送给操作系统,操作系统再响应给应用程序。
	
2、Initiator端配置工具(iscsiadm)
	Initiator各节点安装iscsi-initiator-utils后,也提供了一些配置管理工具,其中最主要的是iscsiadm 命令,
提供了对iSCSI目标节点、会话、连接以及发现记录的操作，iscsiadm使用语法如下:
	iscsiadm
        -m, 同 --mode,其选项有discovery, node, fw(启动值), host, iface(iSCSI 接口), session。
        -I，同 --interface,表示接口。
        -o, 同 --op,可实现对数据库的操作,其选项有new, delete, update, show。
        -n, 同 --name,指定其数据记录里的名字。
        -t, 同 --type,其选项有 sendtargets(st), fw, isns,仅在 -m 模式为 discovery 时有效。
        -p, 同 --portal,指定其目标主机的 IP 地址和端口,默认端口为 3260。
        -L, 同 --loginall,其选项有 all, manual, automatic,指定其登录会话时的类型。
        -T, 同 --targetname,表示其目标名字。
        -v, 同 --value,通常在 --op=update 时指定其值。
		
3、target端配置工具(tatadm)
	Target端安装scsi-target-util后,提供了相应的管理配置工具tgtadm,可以用target、LUN、用户都进管理,不过因为iSCSI模块工作在内核,
tgtadm的配置只在内存中,下次开机重启不会生效,所以可以用过配置文件/etc/tgt/targets.conf来配置,启动时另一个工具tgt-adm会读取该文件。
	tatadm使用语法如下:
		tgtadm --lld [driver] --op [operation] --mode [mode] [OPTION]...
        (1)、添加一个新的 target 且其ID为 [id],名字为 [name]
            --lld [driver] --op new --mode target --tid=[id] --targetname [name]

        (2)、显示所有或某个特定的target:
            --lld [driver] --op show --mode target [--tid=[id]]

        (3)、向某ID为[id]的设备上添加一个新的LUN,其号码为[lun],且此设备提供给initiator使用。
				[path]是某"块设备"的路径,此块设备也可以是raid或lvm设备。lun0已经被系统预留。
            --lld [driver] --op new --mode=logicalunit --tid=[id] --lun=[lun] --backing-store [path]

        (4)、删除ID为[id]的target:
            --lld [driver] --op delete --mode target --tid=[id]

        (5)、删除target [id]中的LUN [lun]：
            -lld [driver] --op delete --mode=logicalunit --tid=[id] --lun=[lun]

        (6)、定义某target的基于主机的访问控制列表，其中，[address]表示允许访问此target的initiator客户端的列表：
            --lld [driver] --op bind --mode=target --tid=[id] --initiator-address=[address]

        (7)、解除target [id]的访问控制列表中[address]的访问控制权限：
            --lld [driver] --op unbind --mode=target --tid=[id] --initiator-address=[address]

			
// 文档链接:https://blog.csdn.net/tjiyu/article/details/52811458
// iscsiadm用法:https://blog.csdn.net/onlyshenmin/article/details/81075209

