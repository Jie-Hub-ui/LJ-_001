条款12：复制对象时勿忘其每一个成分

	设计良好的面向对象系统会将对象的内部全部封装起来，只留下两个函数负责对象拷贝(copy构造函数、copy assignment操作符)，称为copying函数。
编译器在必要的时候为我们创建copying函数，并说明此行为：将拷贝对象的所有成员变量。

1、当我们自行实现copying函数时，实现代码必然出错时编译器并不会告诉你。
	class Customer{
	public:
		Customer(const Customer &rhs);
		Customer &operator=(const Customer &rhs);
		...
	private:
		std::string name;
	};
	Customer::Customer(const Customer &rhs)
		:name(rhs.name)
	{
		...
	}
	Customer &Customer::operator=(const Customer &rhs)
	{
		name = rhs.name;
		return *this;
	}
	这是正常实现我们自行设计的copying函数，若此时加入其它成员变量，而copying函数并未进行处理，这时编译器并不会做出任何处理，
就算在最高警告级别中。因此你为class添加一个成员变量，你必须同时修改copying函数(以及构造函数和非标准形式的operator=,若忘记，编译器不太可能提醒)。

2、若发生在继承体系中，会存在潜在危机
	在继承体系中，若子类构造函数或copying函数初始化列表未对基类显示调用初始化，会被不带实参的基类构造或copying函数初始化。
	任何时候都要为Derived class撰写copying函数，必须很小心的复制其base class成员。往往这些成员是private,需要让Derived class的copying函数
调用相应base class函数：
	class Priorty:publice Customer{
	pulic:
		...
		Priorty(const Priorty &rhs)
			:Customer(rhs),            // 调用base class的copy函数
			 priorty(rhs.priorty)
		{
		};
		Priorty &operator=(const Priorty &rhs)
		{
			Customer::operator=(rhs); // 对base class 成分进行赋值动作
			priorty = rhs.priorty;
			return *this;
		};
	private:
		int priorty;
	};
	当编写一个copying函数时：(1)赋值所有local成员变量；(2)调用base class内的适当copying函数。
	
3、不该令copy assignment操作符调用copy构造函数：这像是试图构造一个已经存在的对象。
   不该令copy构造函数调用copy assignment操作符，同样无意义。构造函数只是用来初始化对象，操作符知识实施于已经初始化的对象身上。
对于一个尚未构造好的对象赋值，就像在一个尚未初始化的对象身上做"只对已经初始化对象才有意义"的事一样。

4、若发现copy构造函数和copy assignment操作符有相近代码，可以建立一个新的成员函数给两者调用。

请注意：
	copying函数应确保复制对象内所有成员变量及所有base class成分。
	不要尝试以某个copying函数实现另一个copying函数，应该将共同机制放进第三个函数中，并由两个函数共同调用。
	
	