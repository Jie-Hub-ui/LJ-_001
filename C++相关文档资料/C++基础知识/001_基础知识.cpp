1、C++ String操作
	1.beforeLast('(')    // 最后一个 '(' 前的内容
	2.beforeFirst('/')   // 第一个 '/' 前的内容
	3.afterLast('/')     // 最后一个'/'之后的内容
	4.afterFirst('/')    // 第一个'/'之后的内容
	5.str.afterLast('/').beforeLast('(')   // 最后一个'/'之后和最后一个'('之前之间的内容
	
	6.str.split("/" , ds , false)    // 将字符串str以 / 为分割，将分割后的内容依次存放在容器 ds 中
	7.str.split("/")                 // 查找字符串str中"/"字串的偏移位置
	
	8.小写的string转为char* ：c_str()
	9.大写的String转为char* : getCStr()
	

	10.String转std::string ------> ::toSTLString()
	11.std::string转String ------> ::toCFLString()
	
	
	