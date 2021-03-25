【导读】:
	可将std::bind函数看作一个通用的函数适配器,它接受一个可调用对象,生成一个新的可调用对象来"适应"原对象的参数列表。
	std::bind将可调用对象与其参数一起进行绑定,绑定后的结果可以使用std::function保存。
1、使用场景
	先将可调用的对象保存起来,在需要的时候再调用,是一种延迟计算的思想。不论是普通函数、函数对象、还是成员函数,成员变量都可以绑定,其中成员函数都可以绑定是相当灵活的。
2、头文件	
	#include <functional>
3、bind原型
	a).
	template< class F, class... Args >
		/*unspecified*/ bind( F&& f, Args&&... args );
	b).
	template< class R, class F, class... Args >
		/*unspecified*/ bind( F&& f, Args&&... args );

4、绑定普通函数
	// 头文件
	#include <functional>
	#include <iostream>
	// 普通函数
	int TestFunc(int a, char c, float f)
	{
		std::cout << a << std::endl;
		std::cout << c << std::endl;
		std::cout << f << std::endl;
		return a;
	}
	// 绑定函数
	auto fun1 = std::bind(TestFunc, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	auto fun2 = std::bind(TestFunc, std::placeholders::_2, std::placeholders::_3, std::placeholders::_1);
	auto fun3 = std::bind(TestFunc, std::placeholders::_1, std::placeholders::_2, 98.77);
	// 使用
	fun1(30, 'C', 100.1);
	fun2(100.1, 30, 'C');
	fun3(30, 'C');
说明:
	fun1说明:占位符->第一个参数和函数第一个参数匹配(int),第二个参数和第二个参数匹配(char),第三个参数和第三个参数匹配。
	fun2说明:显然,可以通过占位符调整顺序,fun2绑定说明:占位符->第二个参数和函数第一个参数匹配(int),第三个参数和第二个参数匹配(char),第一个参数和第三个参数匹配。
	fun3说明:占位符->第一个参数和函数第一个参数匹配(int),第二个参数和第二个参数匹配(char),第三个参数默认为98.77。如果第三个参数也要填的话被忽略。

5、绑定成员函数
	定义一个测试类:		
	class TestClass
	{
	public:
		int ClassMember(int a) { return 55+a; }
		int ClassMember2(int a, char ch, float f)
		{
			std::cout << ch << " " << f << " " << a << std::endl;
			return 55 + a;
		}
		static int StaticMember(int a) { return 66+a; }
	};
	先看一个简单的绑定:		
		TestClass test;
		auto fun4 = std::bind(&TestClass::ClassMember, test, std::placeholders::_1);
		fun4(4);
	通过对象test绑定,和绑定普通函数一样,用一个占位符占用绑定位置,当有多个参数时：		
		auto fun5 = std::bind(&TestClass::ClassMember2,test,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		fun5(1,'A',3.1);
	绑定静态成员函数就更简单了:	
		auto fun6 = &TestClass::StaticMember;
		fun6(3);
	auto的类型:
		std::function<int(int)> fun7 = &TestClass::StaticMember;
		
5、绑定成员变量
	定义一个有public成员变量的测试类:
		class TestClass2
		{
		public:
			TestClass2()
			:m_a(100)
			{}
		public:
			int m_a;
		};
	绑定成员变量:
		TestClass2 test2;
		auto fun8 = std::bind(&TestClass2::m_a, std::placeholders::_1);
	使用:
		int var =fun8(test2);
		std::cout<< var << std::endl;

6、绑定仿函数
	定义测试类:
		class TestClass3
		{
		public:
			TestClass3()=default;
			TestClass3(const TestClass3& obj)
			{
				std::cout<<"TestClass3 copy construct."<<std::endl;
			}
			void operator()(int a)
			{
				std::cout<<a<<std::endl;
			}
		};
	绑定使用:
		TestClass3 test3;
		auto fun9 = test3;
		fun9(2018);
	这里多定义了一个拷贝构造函数,多验证了一个东西,即绑定时调用了拷贝构造,即用test3拷贝构造了一个新的对象,并绑定这个对象,所以fun9是新的对象,跟test3没关系了。
	实际上其他的类成员函数也是一样的,拷贝构造完再绑定。

7、绑定成员函数，是拷贝构造新对象再绑定
	修改下TestClass	
		class TestClass
		{
		public:
			TestClass(int a):m_a(a){}
			TestClass(const TestClass& obj)
			{
				m_a = obj.m_a+100;
				std::cout<<"copy construct."<<std::endl;
			}
			int ClassMember(int a) { std::cout<<" this:"<<this<<" :"<<&m_a<<" "<<m_a<<std::endl;return 55+a; }
			int ClassMember2(int a,char ch,float f)
			{
				std::cout <<ch <<" "<< f << " "<<a<<std::endl;
				return 55+a;
			}
			static int StaticMember(int a) {return 66+a; }
		public:
			int m_a;
		};
	测试:	
		std::cout<<"------"<<std::endl;
		TestClass test(67);
		std::cout<<"&test "<<&test<<" "<<test.m_a<<" &test.m_a "<<&test.m_a<<std::endl;
		auto fun4 = std::bind(&TestClass::ClassMember,test,std::placeholders::_1);
		fun4(4);
		fun4(4);
		std::cout<<"------"<<std::endl;
	输出:	
		------
		&test 0x63fde4 67 &test.m_a 0x63fde4
		copy construct.
		this:0x63fde0 :0x63fde0 167
		this:0x63fde0 :0x63fde0 167
		------
		显然调用了拷贝构造,指针地址也都不一样。
	绑定静态成员函数、成员变量就是直接绑定,没用拷贝构造。

8、通过指针,取消拷贝构造
	先定义一个对象,再利用std::function保存成员函数,在调用时候把test对象传进去即可,测试代码：
		TestClass test(67);
		std::cout<<"############"<<std::endl;
		std::function<int(TestClass*,int)> fg = &TestClass::ClassMember;
		fg(&test,5);
		std::cout<<"#################"<<std::endl;
9、通过引用保存,取消拷贝构造	
	std::function<int(TestClass&,int)> hj = &TestClass::ClassMember;
	TestClass tt(8);
	std::cout<<&tt<<" "<<&tt.m_a<<std::endl;
	hj(tt,45)