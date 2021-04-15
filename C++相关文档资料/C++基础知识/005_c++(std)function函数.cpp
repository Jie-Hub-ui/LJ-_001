 -----------------------------------------------------
|                C++11中的std::function               |
|        原文位置：C++11中的std::function             |
 -----------------------------------------------------
 
1、阐述
	std::function为可调用对象包装器,C++可调用对象(Callable Objects)定义如下:
		1.1 函数指针:与C语言一致;
		1.2 类成员函数指针;
		1.3 仿函数(functor):也成函数对象,重载operator()运算符的类/结构体对象；
		1.4 lambda表达式。
	std::function是Callable Objects的包装器(Wrapper),可接收除了类成员函数指针以外的任意Callable Objects。
	std::function可用来处理函数回调,与C语言函数指针类似,允许保存以上Callable Objects,并延迟执行它们,但它可保存除函数指针外的其他Callable Objects,因此它比C语言更强大。
	当我们为std::function具现化一个函数签名(函数类型,包括返回值和参数列表),它就成为一个可容纳所有这类调用的函数包装器。

2、基本用法
	#include <iostream>
	#include <functional> // std::function

	// global function
	void func(void) {
		std::cout << __FUNCTION__ << std::endl;
	}

	class Foo {
	public:
		// class static function
		static int foo_func(int a) {
			std::cout << __FUNCTION__ << "(" << a << "):";
			return a;
		}

		// class non-static member function
		int foo_func_nonstatic(int a) {
			std::cout << __FUNCTION__ << "(" << a  << "):";
			return a;
		}
	};
	class Bar {
	public:
		// functor
		int operator()(int a) {
			std::cout << __FUNCTION__ << "(" << a << "):";
			return a;
		}
	};
	
	int main() 
	{
		// 传入合适函数签名给std::function模板参数即可绑定对应签名的
		// 普通函数或
		// 类静态成员函数或
		// 借助std::bind绑定类非静态成员函数
		std::function<void(void)> func1 = func;
		std::function<int(int)> func2   = Foo::foo_func;
		Foo foo;
		std::function<int(int)> func3   = std::bind(&Foo::foo_func_nonstatic, &foo,std::placeholders::_1);
		// 然后,直接像函数一样调用
		func1(); // func
		std::cout << func2(1)  << std::endl; // foo_func(1):1
		std::cout << func3(11) << std::endl; // foo_func_nonstatic(11):11

		// 当函数签名一致时,func2可像一个变量一样复用
		// Bar重载了operator()即成为functor,可直接包装到std::function
		Bar bar;
		func2 = bar;
		std::cout << func2(2) << std::endl; // operator()(2):2

		// 也可绑定lambda表达式
		auto func_lambda = [](int a){
			std::cout << "bind lambda sample(" << a << ")" << std::endl;
		};
		func_lambda(3); // bind lambda sample(3)

		return 0;
	}
	
既然std::function可作为左值接收函数对象,那么它可作为函数的形参;且std::function可绑定函数指针,实现函数的延迟执行,所以std::function可取代std::function作为回调函数.
std::function实现回调机制的例子如下:
	#include <iostream>
	#include <functional> // std::function

	// 任意可调用对象，如普通全局函数
	void func_callback(int a) {
		std::cout << __FUNCTION__ << ":Output, a=" << a << std::endl;
	}

	class Foo {
	public:
		explicit Foo(std::function<void(int)> cb)
			:cb_(cb), a_(0){
		}
		~Foo() = default;

		// setter
		void set_a(int a) {
			a_ = a;
		}

		void OutputCallback() {
			cb_(a_);
		}

	private:
		std::function<void(int)> cb_;
		int a_;
	};

	int main()
	{
		// 实例化Foo,并参数注入回调函数
		// 处理
		// 回调输出处理结果
		Foo foo(func_callback);
		foo.set_a(1);
		foo.OutputCallback(); // func_callback:Output, a=1

		return 0;
	}
	
注:C++11提供的std::function可替代C语言中函数指针作为回调函数,前者是C++编程风格,后者是C编程风格。
	
	
	