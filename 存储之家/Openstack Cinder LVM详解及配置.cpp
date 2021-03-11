1、背景
	OpenStack中的实例是不能持久化的,cinder服务重启,实例消失。如果需要挂载volume,需要在volume中实现持久化。Cinder提供持久的块存储,目前仅供给虚拟机挂载使用。
它并没有实现对块设备的管理和实际服务,而是为后端不同的存储结构提供了统一的接口,不同的块设备服务厂商在Cinder中实现其驱动支持以与OpenStack进行整合。它通过整
合后端多种存储,用API接口为外界提供存储服务。主要核心是对卷的管理,允许都卷、类型和快照进行处理。

	Cinder存储分为本地块存储、分布式块存储和SAN存储等多种后端存储类型:
	1. 本地存储:默认通过LVM支持Linux。cinder volume将该服务所在的节点变为存储节点,将上面的volume group作为共享存储池暴露给计算节点。
	2. SAN存储：
		(1)通过NFS协议支持NAS存储,比如Netapp。
		(2)通过添加不同厂商的制定driver来为了支持不同类型和型号的商业存储设备,比如EMC,IBM的存储。
	3. 分布式系统:支持sheepdog,ceph,和IBM的GPFS等。

	对于本地存储,cinder-volume默认使用LVM驱动,该驱动当前的实现需要在主机上事先用LVM命令创建一个的卷组,当该主机接受到创建卷请求的时候,
cinder-volume在该卷组上创建一个逻辑卷,并且用openiscsi将这个卷当作一个iscsi tgt给输出;还可以将若干主机的本地存储用sheepdog虚拟成一个共享存储,然后使用sheepdog驱动。

2、Cinder LVM配置
	2.1 创建LVM物理卷/dev/sdb(一块物理裸磁盘或磁盘分区)
			pvcreate /dev/sdb
		创建LVM卷组(名称:cinder-volumes,可以在卷组上创建一个或多个lv逻辑卷)
			vgcreate cinder-volumes /dev/sdb
	2.2 修改cinder配置文件
		[lvm]
		volume_group  = cinder-volumes   // 指定Cinder使用的volume group;在实际部署cinder的时候其默认名称是cinder-volumes。
		volume_driver = cinder.volume.drivers.lvm.LVMISCSIDriver  
		volume_backend_name = lvm
	注释:
		1.volume_driver指定driver类型.Cinder目前支持两种传输协议,iSCSI and iSER。
			iSCSI的话,将其值设为 cinder.volume.drivers.lvm.LVMISCSIDriver。
			iSER的话,将其值设为 cinder.volume.drivers.lvm.LVMISERDriver。 
			LVM是cinder.volume.drivers.lvm.LVMISCSIDriver
		2.volume_backend_name指定backend name。当有多个volume backend时,需要创建volume type,它会绑定一个或者多个backend。
	用户在创建volume时需要选择某个volume type,相当于选择了某个volume backend。要这个参数的原因是因为一个openstack环境中
	可能有多个cinder-volume存储节点,此时就有多个volume backend,此时就需要给每个backend 一个 name 来区分。

3、挂载LVM卷到云主机流程(这里用块设备说明)
	3.1 安装iscsi服务
		yum install targetcli -y
		systemctl enable target
		systemctl start target
	3.2 进入targetcli shell
		targetcli
	3.3 ls查看当前路径(所有命令同linux下一样,处于不同目录有不一样操作)
		3.3.1 创建block类型的backstore
			cd /backstores/block
			create test_back /dev/cinder-volumes/volume-02f986b0-418f-4f84-a5ec-7af61f5d5293
			说明:创建一个名为test_back的block类型的backstore,/dev/cinder-volumes/volume-02f986b0-418f-4f84-a5ec-7af61f5d5293为块设备路径
				 OpenStack Cinder创建的卷存放于/dev/cinder-volumes(创建的卷组)下面,lvs命令可以查看所有卷,lvdisplay 命令可以查看某个卷详细信息。
		3.3.2 创建一个target
			cd /iscsi
			create iqn.2017-09.me.int32bit:int32bit               // 创建一个名称为:iqn.2017-09.me.int32bit:int32bit的target
		3.3.3 将创建的target export出去,即进入监听状态,称之为:portal
			cd iqn.2017-09.me.int32bit:int32bit/tpg1/portals/     // 进入创建的target目录下的tpg1/portals/目录下
			create 10.0.0.4                                       // 10.0.0.4是server的ip，不指定端口的话就会使用默认的端口3260
		3.3.4 将创建的backstore加入target中
			cd ../luns
			create /backstores/block/test_back
			此时创建的target包含一个lun设备了,可以查看:ls /iscsi/iqn.2017-09.me.int32bit:int32bit/
	3.4 配置客户端
		yum install iscsi-initiator-utils -y
		systemctl enable iscsid iscsi
		systemctl start iscsid iscsi
		3.4.1 拿到客户端本机initiator name
			cat /etc/iscsi/initiatorname.iscsi
			client需要连接server target，还需要ACL认证，我们在server端增加client的访问权限，在server端运行:
				cd /iscsi/iqn.2017-09.me.int32bit:int32bit/tpg1/acls
				create iqn.1994-05.com.redhat:e0db637c5ce    // iqn.1994-05.com.redhat:e0db637c5ce为客户端本机的initiator name
		3.4.2 iscsiadm命令自动发现本地可见的target列表
			iscsiadm --mode discovery --type sendtargets --portal 10.0.0.4 | grep int32bit
		3.4.3 登录验证
			iscsiadm -m node -T iqn.2017-09.me.int32bit:int32bit -l
		3.4.4 查看所有已经登录的target
			iscsiadm -m session
		3.4.5 target已经自动映射到本地块设备,我们可以使用lsblk查看
			lsblk --scsi
		
		
// OpenStack虚拟机挂载卷过程:https://zhuanlan.zhihu.com/p/29229499
// OpenStack Cinder LVM配置:https://www.cnblogs.com/sammyliu/p/4159147.html
		
		
		
		
		
		