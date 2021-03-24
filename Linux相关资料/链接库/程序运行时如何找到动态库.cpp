程序运行时如何找到动态库？

我们随便开发一个C/C++程序，都很大程度不可避免的需要用到动态库：
	#include <stdio.h>
	int main()
	{
		printf("hello，编程珠玑\n");
		return 0;
	}
	编译并查看使用到的动态库：
	$ gcc -o main main.c
	$ ldd main
		linux-vdso.so.1 (0x00007ffdf8fdf000)
		libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f1f8535e000)
		/lib64/ld-linux-x86-64.so.2 (0x00007f1f85951000)
	从ldd命令的结果我们可以看到main程序依赖了哪些动态库,并且在哪个路径。那么你有没有想过,
动态库的路径是怎么找到的,查找顺序又是怎样的呢？

1、准备动态库
	在此之前如果你还没有对动态库有一个基本的了解的话,建议你阅读《浅谈静态库和动态库》或其他相关资料。
为了说明后面的问题,这里我们先创建一个简单的动态库：
	#include <stdio.h>
	#include "test.h"
	#include "test1.h"
	void test()
	{
		printf("I am test；hello，编程珠玑\n");
		test1();
	}

	// test.h
	void test();

	//test1.c
	#include <stdio.h>
	#include "test1.h"
	void test1()
	{
		printf("test1,needed by test\n");
	}
	// test1.h
	void test1();
	
分别制作动态库libtest.so和libtest1.so,这在后面的示例中会用到：
	$ gcc test1.c -fPIC -shared -o libtest1.so
	$ gcc test.c -fPIC -shared -o libtest.so -L. -ltest1
	这样你在当前目录下就会看到有一个libtest.so和libtest1.so文件生成了，其中litest.so依赖libtest.so	
	注意:由于libtest.so依赖libtest1.so,这里用-L指定了要链接的test1的路径,因此我们看到：
	$ ldd libtest.so
    linux-vdso.so.1 (0x00007ffd1bbca000)
    libtest1.so => not found
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f9f1d0ae000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f9f1d6a1000)
	从这里可以看出libtest是依赖libtest1库的，但是特别注意到，libtest1.so指向的是not found,
这会有什么影响吗？我们后面就会看到。

2、链接时查找路径
	我们都知道，在编译成可执行文件前,链接器链接动态库也是需要查找动态库路径的,否则怎么链接上指定的动态库呢？那么这个顺序又是怎样的呢？
	首先会查找的会是编译时链接的路径。修改前面的main.c，让它调用libtest.so中的test函数：
	#include <stdio.h>
	#include "test.h"
	int main()
	{
		test(); // 调用libtest.so中的test函数
		return 0;
	}
	编译链接：
	$ gcc -o main main.c -I ./ -L./ -ltest -ltest1
	完美编译过。除此之外，如果我们把libtest.so和libtest1.so都移到/usr/lib下面，我们发现，即便不用-L也能编译过了:
	$ gcc -o main main.c -I ./  -ltest -ltest1
	这里需要说明的是，我们通过-L./来指定搜索库的路径，由于libtest.so依赖libtest1.so，因此在编译链接时，也需要链接上test1。
	
3、小结
	从上面的内容可以看到，在链接时，我们通过-L参数搜索要链接的库路径，但是这个路径信息不会写到ELF文件中，因此你会通过ldd命令看到not found，而通过-rpath可以指定链接时的搜索路径，这个信息会写入到ELF文件中，最终看到的结果是，由于libtest.so依赖libtest1.so，所以其他程序依赖libtest.so时，会自动从写入ELF的rpath中搜索它依赖的其他库，因此只需要链接libtest即可，例如：

在制作库libtest1.so时，加上-rpath-link选项：

$ gcc test.c -fPIC -shared -o libtest.so -L. -ltest1 -Wl,-rpath-link $(pwd)
在编译main的时候，就不需要特意指定链接libtest1.so

$ gcc -o main main.c -L ./ -ltest
只需要链接libtest.so，其依赖的libtest1.so也链接进来了。
当然了，如果-L指定的路径没有呢,它还会去查找其他地方，否则依赖的系统库怎么找到呢？总结大致顺序如下：

-L指定链接路径
对于依赖库中依赖的搜索顺序通过-rpath-link或-rpath选项查找（后面会提到）
gcc默认链接路径（gcc --print-search-dir | grep libraries 查看）
链接器配置的查找路径（ld -verbose | grep SEARCH_DIR查看）
针对具体的系统或链接器，可能有些差异，但是大致如此。

4、运行时查找路径
	虽然前面编译成功了，但是我们运行看看，发现运行失败了。

$ ./main
./main: error while loading shared libraries: libtest.so: cannot open shared object file: No such file or directory
其实我们用ldd命令看一下也能看到：

    linux-vdso.so.1 (0x00007ffcd566e000)
    libtest.so => not found
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f356d1f6000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f356d7e9000)
LD_PRELOAD环境变量

这个环境变量在介绍《性能优化-使用高效内存分配器》中的时候，也有提到，用来做测试非常方便，同样的，这种方式也最好仅仅只是用于测试，因为它的优先级非常高，并且影响全局。使用也很简单：

$ export LD_PRELOAD=./libtest.so
$ ./main
为了避免影响后面的验证，这里取消设置该环境变量：

unset LD_PREALOD

5、查找rpath
	上面的情况是找不到动态库，那么它首先会去rpath指定路径去查找，这需要在编译时指定：

$ gcc test.c -fPIC -shared -o libtest.so -L. -ltest1 -Wl,-rpath $(pwd)
$ gcc -o main main.c -L . -ltest -Wl,-rpath $(pwd)
$ ./main
I am test；hello，编程珠玑
test1,needed by test
也就是说，如果我们编译时指定了路径，就可以找到了，但是这些信息被写入到了ELF文件中。

6、LD_LIBRARY_PATH环境变量
	另外也可以通过这个环境变量来设置要搜索库的路径。
	$ gcc -o main main.c -L . -ltest
	$ export LD_LIBRARY_PATH=./
	$ ./main
	这样运行也是没有问题的。
	同样,为了避免对后面测试产生影响,取消设置该环境变量：
	unset LD_LIBRARY_PATH
	/etc/ld.so.conf中的路径
	我的机器上这个文件的内容如下：

	$ cat /etc/ld.so.conf
		include /etc/ld.so.conf.d/*.conf
		$ ls /etc/ld.so.conf.d/
		fakeroot-x86_64-linux-gnu.conf  libc.conf  x86_64-linux-gnu.conf
	所以它实际指的是/etc/ld.so.conf.d/目录下所有conf路径包含路径,打开其中一个libc.conf,它里面包含的路径为：
		$ /usr/local/lib
	既然如此,我们把前面的libtest.so复制到该目录下(可能需要sudo权限)：
	$ sudo cp libtest.so /usr/local/lib
	$ sudo ldconfig
	$ ./main
		I am test；hello，编程珠玑
		test1,needed by test
	注意，这里拷贝完成后，需要执行ldconfig,它会更新相应的共享库,以便可执行程序能够找到。
实际上,执行完成后,你确实就能在/etc/ld.so.cache文件中找到：
$ grep -a libtest.so /etc/ld.so.cache
	同样，为了影响后面测试，记得删除：
		rm /usr/local/lib/libtest.so
	实际上这里是先从/etc/ld.so.cache中的路径查找,然后再从ld.so.conf的路径中查找。后面我们可以通过命令看到。

7、查找/usr/lib、/lib/
	当然了,如果以上路径都没有,最终还会在lib或/usr/lib下找,为了验证,我们将库拷贝到/lib目录下
	$ cp libtest.so /lib
	$ ./main
		I am test；hello，编程珠玑
		test1,needed by test
	同样能正常运行。

8、小结一下，动态库的搜索顺序如下：
	LD_PRELOAD环境变量指定库路径
	-rpath链接时指定路径
	LD_LIBRARY_PATH环境变量设置路径
	/etc/ld.so.conf配置文件指定路径
	默认共享库路径:/usr/lib、lib
	以上这些查找路径你很容易来验证它们的优先级,简单的做法就是这几个位置分别放置同名不同作用的库,
来看看它到底先使用哪个路径下的库,可自行尝试。

9、LD_DEBUG
这个环境通常用来调试。例如,查看整个装载过程：
	$ LD_DEBUG=files ./main
	或者查看依赖的库的查找过程：
	$ LD_DEBUG=libs ./main
      3557:    find library=libtest.so [0]; searching
      3557:     search cache=/etc/ld.so.cache
      3557:      trying file=/usr/local/lib/libtest.so
	另外还可以显示符号的查找过程：
	$ LD_DEBUG=symbols ./main
	
10、总结
	了解动态库的搜索路径,可以在开发中很好的帮助你定位找不到库的问题,同时LD_DEBUG环境变量也能够很好的帮助你调试,
例如查看库搜索的路径,显示符号的查找过程等等。
	虽然程序运行能够有多种途径获取动态库路径,但是并不是每种方式都合适,有的方式甚至完全不该用,但这超出了本文的讨论范围了。
	
	
	
	
	
	
	