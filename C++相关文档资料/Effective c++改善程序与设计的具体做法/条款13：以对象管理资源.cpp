资源管理
	所谓资源就是一旦使用，就必须还给系统。C++中最常使用的资源就是动态分配内存(若使用后不归还便会造成资源泄露)，但内存只是
你必须管理的资源之一;其他常见资源还有：文件描述符、互斥锁、数据库连接、网络socket等。

1、以对象管理资源
	假设设计一个类：
		class Investment{...};
		Investment *createInvestment();  // 返回指针，指向Investment继承体系内的动态分配对象，调用者有责任delete它
	现在考虑一个函数：
		void funtion(){
			Investmnt *pInv = createInvestment();
			...
			delete pInv;
		}
	这看起来视乎妥当，但是当"..."区域内有一个过早的return语句,控制流就绝不会触碰到delete pInv语句;或createInvestment
的使用和delete位于某个循环体内，而该循环体由于某个continue或goto语句过早退出;抑或"..."区域内抛出异常，delete便不会有
幸的被执行到。因此我们泄露的不只是内含对象的那块资源，还包括对象所保存的任何资源。

	为确保资源总是被释放，我们需要将资源放进对象内，当控制流离开时,该对象析构函数会自动释放资源。即：把资源放进对象内，
便可依赖c++的析构函数自动调用机制释放资源。

	许多资源被动态分配与heap内而后被用于单一区域或函数内。他们应当在控制流离开时被释放。
	标准程序库提供auto_ptr(类指针对象),也就是所谓的"智能指针",其析构函数自动对起所指对象调用delete。
		void funtion(){
			std::auto_ptr<Investmnt> pInv(createInvestment());  // 经由auto_ptr的析构函数自动删除
			...
		}
	两个关键想法：
		1.获得资源后立刻放进管理对象内：实际是"以对象管理资源"的观念常被称为"资源取得时机便是初始化时机"(RAII),因为我们总是
		获得一笔资源后于同一语句内以它初始化对象。
		2.管理对象运用析构函数确保资源被释放:不论控制流是否离开区域，一旦对象被销毁其析构函数自然会被调动释放资源。
	auto_ptr被销毁时会自动删除所指物，但是不要让多个auto_ptr指向同一对象。auto_ptr有一个不寻常的性质:通过copy构造函数和copying
赋值操作符复制他们，他们会变成null，而复制所得指针将取得唯一资源。
		std::auto_ptr<Investmnt> pInv1(createInvestment()); 
		std::auto_ptr<Investmnt> pInv2(pInv1);  // pInv1被设置为null，pInv2指向pInv1 
		
2、auto_ptr复用方案
	shared_ptr:RCSP智能指针(引用计数智慧指针),持续跟踪有多少个对象指向某笔资源,在没有指向的时候删除。但是不能打破两个没有使用的对象互指。
	本条款不专门针对这两个智能指针，只是强调以对象管理资源的重要性。
	auto_ptr和share_ptr两者都在析构函数里面做了delete而非delete[],意味着动态分配array是个馊主意。并没有针对c++动态分配数组而设计的智能指针。
因为vector和string可以取代。

请记住:
	为防止资源泄露，请使用RAII对象，他们在构造函数中获得资源并在析构函数中释放资源。
	
		
		
		