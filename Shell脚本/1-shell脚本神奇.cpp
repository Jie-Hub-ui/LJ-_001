1、脚本神奇一[脚本语法检查器--shellcheck]
	shellcheck:用于检查脚本,可提前发现脚本错误并指明错误原因。
	1.安装
		yum -y install epel-release ShellCheck
	2.使用
		2.1 在线使用(提供了一个在线的检查地址,https://www.shellcheck.net/,进入网址即可使用,输入你的脚本内容)
		2.2 命令行使用
			shellcheck test.sh     // test.sh是要检查得脚本名称
		2.3 编辑器使用