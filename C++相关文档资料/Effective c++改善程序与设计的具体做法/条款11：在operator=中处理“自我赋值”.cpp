条款11：在operator=中处理"自我赋值"

1、自我赋值发生在对象被赋值给自己时。
	class Widget{};
	Widget w;
	...
	w = w;
这看起来有点愚蠢，但是却合法。

2、自我赋值并不那么一眼识别
	a[i] = a[j];   // 潜在自我赋值(若i=j)
	*px  = *py;    // 潜在自我赋值(若px和py指向同一地址)
这些并不明显的自我赋值是"别名"带来的结果，所谓别名：有一个以上的方法指称(指涉)某对象。
一般而言，若代码操作pointers或references而他们被用来指向多个相同类型的对象，就需要考虑对象是否为同一对象。
实际上，两个对象若来自同一继承体系，它们甚至不用声明为相同类型就可能造成"别名"产生自我赋值。
	class Base{...};
	class Derived:public Base{...};
	void doSomething(const Base &rb, Derived *pd);  // rb 和 pd可能其实是同一对象，因为基类references和pointers可以指向子类对象

3、若运用对象来管理资源，而且可以确定所谓资源管理对象在copy发生时能有正确举措，这种情况下你的自我赋值或许是安全的。
4、若尝试自行管理资源，就要注意在停止资源使用前意外释放它的陷阱
	class Bitmip{...};
	class Widget{
		...
	private:
		Bitmip *pb;
	};
	下面是operator=实现代码，看似合理，但自我赋值出现时并不安全
	Widget &Widget::operator=(const Widget &rhs){     // 一份不安全的operator实现版
		delete pb;                // 停止当前的bitmip
		pb = new Bitmip(*rhs.pb); // 使用rhs bitmip 副本
		return *this;             
	}
	这里自我赋值问题是：operstor=函数内的*this和rhs可能是同一对象，此时delete就不仅仅销毁当前对象的bitmip，也销毁了rhs的bitmip；
	在函数末尾，Widget本不该被自我赋值改变动作的却发现自己持有一个已被删除的对象！
	
	欲阻止这种错误，传统做法是在operator=前面加一个证同测试达到自我赋值：
	Widget &Widget::operator=(const Widget &rhs){     
		if (this == &rhs) return *this; // 证同测试，若是自我赋值不作任何处理
		delete pb;                      
		pb = new Bitmip(*rhs.pb);       
		return *this;             
	}

	但是，此时解决了自我赋值安全性，但是有可能出现异常安全性；若new导致异常(不论是内存分配异常还是bitmip copy构造异常)，Widget最终都会
持有一个指针指向被删除的bitmip。让operator=具备异常安全性往往自动获得自我赋值安全性回报。可做如下处理：
	Widget &Widget::operator=(const Widget &rhs){     
		Bitmip *pOrig = pb;        // 记住原有pb             
		pb = new Bitmip(*rhs.pb);  // 令原有pb指向*pb的一个复件
		delete pOrig;              // 删除原有pb
		return *this;             
	}
	此时即使没有证同测试，但也能处理自我赋值，也许不是最高效办法，但是行得通。若你考虑到效率，可以讲证同测试再次放回原处，这一般在自我赋值
发生频率较高场合。

5、在operator=函数内手工排列语句的一个替代方案(确保异常安全和自我赋值安全),使用所谓 copy and swap技术：
	class Widget{
	...
	void swap(Widget &rhs);  // 交换*this 和 rhs的数据
	...
	};
	Widget &Widget::operator=(const Widget &rhs){      // references传递
		Widget temp(rhs);  // 产生rhs的一份复制
  		swap(temp);        // 将*this数据和上述复制文件的数据交换
		return *this;
	}
	Widget &Widget::operator=(Widget rhs){      // by value传递，函数构造期间便生成临时复件
  		swap(temp);        // 将*this数据和上述复制文件的数据交换
		return *this;
	}
	
请记住：
	确保对象自我赋值时operator=有良好行为。其中技术包括比较来源对象和目标对象的地址、精心周到的语句顺序、以及copy-and-swap。
	确定任何函数如果操作一个以上的对象，而其中多个对象是同一对象时，其行为依然正确。
	
	
	
	
	
	