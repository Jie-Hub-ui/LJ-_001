 --------------------------------------------------------
|             C++ 虚函数表及多态内部原理详解             |
|     原文位置：微信搜索C++虚函数表及多态内部原理详解    |
 --------------------------------------------------------
 
	C++ 中的虚函数的作用主要是实现了多态的机制。关于多态,简而言之就是用父类型的指针指向其子类的实例,然后通过父类的指针调用实际子类的成员函数。
这种技术可以让父类的指针有"多种形态",这是一种泛型技术。

1、虚函数表
	每个含有虚函数的类都有一个虚函数表(Virtual Table)来实现的,简称为V-Table。C++的编译器应该是保证虚函数表的指针存在于对象实例中最前面的位置
(这是为了保证取到的虚函数表有最高的性能——如果有多层继承或是多重继承的情况下)。这意味着我们通过对象实例的地址得到这张虚函数表,然后就可以遍历其中函数指针,
并调用相应的函数。
	1、每一个类都有虚函数列表。
	2、虚表可以继承,如果子类没有重写虚函数,那么子类虚表中仍然会有该函数的地址,只不过这个地址指向的是基类的虚函数实现。如果基类有3个虚函数那么基类的虚表中就有三项
		(虚函数地址),派生类也会有虚表,至少有三项;如果重写了相应的虚函数,那么虚表中的地址就会改变,指向自身的虚函数实现。如果派生类有自己的虚函数,那么虚表中就会添加该项。
	3、派生类的虚表中虚函数地址的排列顺序和基类的虚表中虚函数地址排列顺序相同,子类独有的虚函数放在后面。
	当定义一个有虚函数类的对象时,对象的第一块的内存空间就是一个指向虚函数列表的指针。

	假设我们有这样的一个类:
		class Base{
		public:
			virtual void f(){ cout <<　std::cout << "Base::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base::g" << std::endl; };
			virtual void h(){ cout <<　std::cout << "Base::h" << std::endl; };
		};
		
		typedef void(*Fun)(void);
		
		int main(int argc, char *argv[])
		{
			Base b;
			Fun pfun = NULL;
			std::cout << (long*)(&b) << std::endl;
			pfun = (Fun)*((long*)*(long*)(&b));
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 1);
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 2);
			pfun();
			return 0;
		}
	由于例程的操作环境是64位系统,所以用long*强转。其中(long*)(&b)就是虚函数表地址,(long*)*(long*)(&b)就是第一个函数地址,代码运行结果如下：
		0x7ffd107f2788
		Base::f
		Base::g
		Base::h
	在程序中取出对象b的地址,根据对象的布局可以得出就是虚表的地址,根据这个地址可以把虚表的第一个内存单元的内容取出,然后强制转换成一个函数指针,
利用这个函数指针来访问虚函数。又因为虚表是连续的,利用每次+1可以来访问下一个内存单元。
	注意:虚函数表在最后会有一个结束标志,为1说明还有虚表,为0表示没有虚表了。(编译器不同,结束标志可能存在差异)

2、无虚函数覆盖
	子类没有任何的继承(子类没有实现基类的虚函数),如以下例子：
		class Base{
		public:
			virtual void f(){ cout <<　std::cout << "Base::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base::g" << std::endl; }
		};
		class Derived:public Base{
		public:
			virtual void f1(){ cout <<　std::cout << "Derived::f1" << std::endl; }
			virtual void g1(){ cout <<　std::cout << "Derived::g1" << std::endl; }
		};
		typedef void(*Fun)(void);
		
		int main(int argc, char *argv[])
		{
			Base b;
			Fun pfun = NULL;
			std::cout << (long*)(&b) << std::endl;
			pfun = (Fun)*((long*)*(long*)(&b));
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 1);
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 2);
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 3);
			pfun();
			return 0;
		}
		和上一个程序一样,根据取出虚表里面的地址强制转换成函数指针,同样,利用虚表的连续性,每次指针+1调用对应的虚函数。
	可以得出虚函数按照其声明顺序存放于虚函数表中的,子类自己的虚函数是排在父类虚函数之后的。运行结果如下:
		0x7ffd107f2788
		Base::f
		Base::g
		Derived::f1
		Derived::g1
3、一般继承(有虚函数覆盖)
	如果子类中有虚函数重载了父类的虚函数:
		1.覆盖的函数被放到了虚表中原来父类虚函数的位置。
		2.没有被覆盖的函数依旧。
	这样就会出现虚调用,例如:
		Base *b = new Derive();
		b->f();
		由b所指的内存中的虚函数表的f()的位置已经被Derive::f()函数地址所取代,于是在实际调用发生时,是Derive::f()被调用了。
	这就实现了多态。下面我们用一个示例代码来看一下:
		class Base{
		public:
			virtual void f(){ cout <<　std::cout << "Base::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base::g" << std::endl; }
		};
		class Derived:public Base{
		public:
			virtual void f(){ cout <<　std::cout << "Derived::f1" << std::endl; }
			virtual void g1(){ cout <<　std::cout << "Derived::g1" << std::endl; }
		};
		typedef void(*Fun)(void);
		int main(int argc, char *argv[])
		{
			Base *b = new Derived;
			Fun pfun = NULL;
			std::cout << (long*)(&b) << std::endl;
			pfun = (Fun)*((long*)*(long*)(&b));
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 1);
			pfun();
			pfun = (Fun)*((long*)*(long*)(&b) + 2);
			pfun();
			return 0;
		}
		运行结果如下,确实如我们以上分析的那样,由b所指的内存中的虚函数表的f()的位置已经被Derive::f()函数地址所取代：
		Derived::f
		Base::g
		Derived::g1
4、多重继承(无虚函数覆盖)
	多重继承情况下：
		1.每个父类都有自己的虚表。
		2.子类的成员函数被放到了第一个父类的表中。(所谓的第一个父类是按照声明顺序来判断的)
	这样做就是为了解决不同的父类类型的指针指向同一个子类实例,而能够调用到实际的函数。
	举例说明:
		class Base1{
		public:
			virtual void f(){ cout <<　std::cout << "Base1::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base1::g" << std::endl; }
		};
		class Base2{
		public:
			virtual void f(){ cout <<　std::cout << "Base2::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base2::g" << std::endl; }
		};
		class Base3{
		public:
			virtual void f(){ cout <<　std::cout << "Base3::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base3::g" << std::endl; }
		};
		class Derived:public Base1, Base2, Base3{
		public:
			virtual void f1(){ cout <<　std::cout << "Derived::f1" << std::endl; }
			virtual void g1(){ cout <<　std::cout << "Derived::g1" << std::endl; }
		};
		typedef void(*Fun)(void);
		int main(int argc, char *argv[])
		{
			Derived d;
			Fun pfun = NULL;
			long **pvtab = (long **)&d;
			pfun = (Fun)ptab[0][0];
			pfun();
			pfun = (Fun)ptab[1][0];
			pfun();
			pfun = (Fun)ptab[2][0];
			pfun();
			pfun = (Fun)ptab[0][2];
			pfun();
			pfun = (Fun)ptab[0][3];
			pfun();
			return 0;
		}
		运行结果如下:
			Base1::f
			Base2::f
			Base3::f
			Derived::f1
			Derived::g1
	在这个程序中,子类有多个父类,因此从每个父类都继承了一个虚表,因此会有3个虚表,根据代码和运行结果会发现,
排列的顺序和继承的顺序一样,子类自己的虚函数排在第一个虚表的后面。
	程序中没有改写虚函数,因此没有覆盖。同时主函数中应用的是一个二重指针,利用二维数组取每个虚函数地址。

5、多重继承(有虚函数覆盖)
	如果发生虚函数覆盖的情况:
		父类虚函数表中的虚函数的位置被替换成了子类的函数指针。这样,我们就可以任一静态类型的父类来指向子类,并调用子类实现基类的虚函数了。
	举例说明:
		class Base1{
		public:
			virtual void f(){ cout <<　std::cout << "Base1::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base1::g" << std::endl; }
		};
		class Base2{
		public:
			virtual void f(){ cout <<　std::cout << "Base2::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base2::g" << std::endl; }
		};
		class Base3{
		public:
			virtual void f(){ cout <<　std::cout << "Base3::f" << std::endl; }
			virtual void g(){ cout <<　std::cout << "Base3::g" << std::endl; }
		};
		class Derived:public Base1, Base2, Base3{
		public:
			virtual void f(){ cout <<　std::cout << "Derived::f" << std::endl; }
			virtual void g1(){ cout <<　std::cout << "Derived::g1" << std::endl; }
		};
		typedef void(*Fun)(void);
		int main(int argc, char *argv[])
		{
			Derived d;
			Fun pfun = NULL;
			long **pvtab = (long **)&d;
			pfun = (Fun)ptab[0][0];
			pfun();
			pfun = (Fun)ptab[1][0];
			pfun();
			pfun = (Fun)ptab[2][0];
			pfun();
			pfun = (Fun)ptab[0][2];
			pfun();
			return 0;
		}
		运行结果如下:
			Derived::f
			Derived::f
			Derived::f
			Derived::g1
	本程序中,子类重写了f函数,把所有父类里面的f函数都屏蔽了。在虚表中父类f函数的位置全部换成了子类f函数的地址。
因此在输出时父类的f函数没有了,全部是子类f函数的输出。
	
	
	