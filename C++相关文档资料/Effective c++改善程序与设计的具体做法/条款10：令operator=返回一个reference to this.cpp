条款10：令operator=返回一个reference to *this
1、为实现"连锁赋值"，赋值操作符必须返回一个reference指向操作符的左侧实参，这是你为class实现赋值操作符时应该遵循的协议
	class Widget{
	public:
	....
		Widget &operator=(const Widget &rhs){
			...
			return *this;
		}
	};
2、此协议不仅仅适合于以上标准赋值操作形式，也适用于所有赋值相关运算
	class Widget{
	public:
	....
		Widget &operator+=(const Widget &rhs){
			...
			return *this;
		}
		Widget &operator=(int rhs){
			...
			return *this;
		}
		...
	};
3、注意：这只是个协议，并无强制要求。不遵循同样可以编译通过，然而这份协议被所有内置类型和标准程序库提供的类型如：
		 string、vector、complex、share_ptr或即将提供的类型共同遵循。

请记住：令赋值操作符返回一个referencr to *this.