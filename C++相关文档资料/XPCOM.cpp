
一、AB编译XPCOM组件要术
	1、CMakeLists.txt中需要包含IDL文件目录、设置加载xpcom生成工具,例:
		INCLUDE_DIRECTORIES(${APP}/openstack/api/public)
		SET(CUSTOM_IDL_PATH ${APP}/openstack/api/public)
	2、IDL文件说明
		#include "nsISupports.idl"       // 包含此头文件
		
		[ref] native StringRef(String);                  // 表示 StringRef 引用变量 String
		[ref] native STLStrRef(std::string);             // 表示 STLStrRef 引用变量 std::string
		[ref] native OpenStackLoginRef(OpenStackLogin);
		      native Bool(bool);                         // 非引用
		// IDL文件接口中需要使用的类型都要在该文件中提前生明
		
		%{
			// 此范围可以定义c++相关变量、操作,最终调用工具将IDL生成的.h文件原样生成在.h文件中
			// 一般在IDL定义的接口中需要后面使用的变量,此时可定义在此,因为编译有顺序,不然会出现未定义现象
			// 也可将组件ID定义在此处,例:
			#define NC_OPENSTACK_API_CID                            \
				{    0xa7b0f9fc,0x370a,0x4351,{ 0xad, 0xc6, 0x0, 0x4e, 0xd1, 0xe9, 0x64, 0xf2 }}     // 不可与其他组件重复,值可所以修改

			#define NC_OPENSTACK_API_CONTRACTID                     \
			"eso_openstackapi"
		%}
		
		[uuid(261F0002-D6EF-48cd-990C-03FDAF781DBC)]     // 可用 uuidgen 命令生成
		interface ncIOpenStackApi : nsISupports          // 固定格式
		{
			[notxpcom] STLStr GetToken();                // 固定格式
			[notxpcom] String GetKeyStoneURL();
			[notxpcom] STLStr KeyStoneGet ([const] in StringRef url);
			....
		};
	3、IDL文件编译说明
		IDL文件在CMakeLists.txt编译时,AB这边在Apollo/Linux_el7_x64/Debug/src/cpp/app(编译时会在Debug目录下映射相应目录)对应目录下,
	通过加载xpcom工具(Debug下regxpcom)生成对应的.h文件(其实在实际代码目录结构中看不见,只是在编译的对应目录下会生成,主要是在编译时才产生)。
	生成的.h文件里面有详细的使用方法。例如：
		openstack api组件编译时编译路径和.h文件生成路径:
			Apollo/Linux_el7_x64/Debug/src/cpp/app/openstack/api/src/Build/public

二、AB编译XPCOM组件流程
	1、编写IDL文件(即接口编写,此文件中编写的接口不可同名,不能实现c++中的重载等操作),例：
		文件名：
			ncIOpenStackApi.idl
		文件内容：
			#include "nsISupports.idl"

			[ref] native StringRef(String);
			[ref] native STLStrRef(std::string);
			[ref] native OpenStackLoginRef(OpenStackLogin);
			native Bool(bool);
			....

			%{C++
				#define NC_OPENSTACK_API_CID                            \
				{    0xa7b0f9fc,0x370a,0x4351,{ 0xad, 0xc6, 0x0, 0x4e, 0xd1, 0xe9, 0x64, 0xf2 }}

				#define NC_OPENSTACK_API_CONTRACTID                     \
					"eso_openstackapi"
				struct OpenStackLogin
				{
					int    port;
					....

				};
    
				enum Http_Opt
				{
					HTTP_GET,
					HTTP_PUT,
					HTTP_POST,
					HTTP_DELETE,
				};
			%}

			[uuid(261F0002-D6EF-48cd-990C-03FDAF781DBC)]
			interface ncIOpenStackApi : nsISupports
			{
				[notxpcom] void Init([const] in OpenStackLoginRef logininfo);
				/*
				* 登陆，获取token
				*/
				[notxpcom] void Login([const] in StringRef region);
				....
			};
			
	2、编写组件实现.cpp和.h文件:.h文件中编写类继承idl中接口类(包含#include <nsISupports.h>头文件和ild通过编译工具生成的头文件,
		编译时调用xpcom工具会将idl文件生成.h文件);该类中必须要实现基类接口类的方法,且必须包含[ NS_DECL_ISUPPORTS 和 NS_DECL_NCIOPENSTACKAPI(接口类中类名大写) ];
		例：
			文件名:
				ncOpenStackApi.h
			文件内容:
				#ifndef _OPENSTACKAPI_H_
				#define _OPENSTACKAPI_H_
				#include <abprec.h>
				#include <ncIOpenStackApi.h>   // 头文件(编译时调用xpcom工具生成)
				....
				class ncOpenStackApi : public ncIOpenStackApi
				{
				public:
					NS_DECL_ISUPPORTS;          // 必须包含  
					NS_DECL_NCIOPENSTACKAPI;    // 必须包含

					ncOpenStackApi();
				privat:
					virtual ~ncOpenStackApi();
				......
				}
				#endif
			文件名：
				ncOpenStackApi.cpp
			文件内容:
				#include <abprec.h>
				#include "ncOpenStackApi.h"
				....
				
				NS_IMPL_ISUPPORTS1(ncOpenStackApi, ncIOpenStackApi)   // 开头必须含有
				// 构造析构
				ncOpenStackApi::ncOpenStackApi() {}
				ncOpenStackApi::~ncOpenStackApi() {}
				// 实现XPCOM组件接口(即基类接口)
				NS_IMETHODIMP_(void)
				ncOpenStackApi::Init(const OpenStackLogin &logininfo)
				{
					
				}
				NS_IMETHODIMP_(void)
				ncOpenStackApi::Login(const String &region)
				{
					
				}
				......
	3、绑定组件ID等信息,这个操作一般放在 modulefactory.cpp里面(也可放在其他地方或ncOpenStackApi.h里面)
		文件名：	
			modulefactory.cpp
		文件内容:
			#include <abprec.h>
			#include "ncOpenStackApi.h"
			.....
			
			#define LIB_NAME _T("openstackapi")   
			
			// 通俗讲就是绑定组件ID,必须实现
			NS_GENERIC_FACTORY_CONSTRUCTOR (ncOpenStackApi);
			static const nsModuleComponentInfo components[] = {
				{
					"ncOpenStackApi",
					NC_OPENSTACK_API_CID,        // 在IDL中已声明
					NC_OPENSTACK_API_CONTRACTID, // 在IDL中已声明
					ncOpenStackApiConstructor    // 值由上面的宏产生(NS_GENERIC_FACTORY_CONSTRUCTOR)
				},
			};
			
			// 这个类必须实现
			class ncSharedLibrary : public ISharedLibrary
			{
			public:
				virtual void onInitLibrary (const AppSettings* appSettings, const class AppContext* appCtx)
				{
					try {
						init(appSettings, appCtx, LIB_NAME);  // 这个函数主要实现trace,如果不需要实现trace可以不实现,注释掉
					}
					catch (Exception& e) {
						SystemLog::getInstance ()->log (ERROR_LOG_TYPE, _T("Unabled to load resource (%s). [Error: %s]"),LIB_NAME, e.toString ().getCStr ());
					}
				}

				virtual void onCloseLibrary (void) NO_THROW
				{
					fini();       // 同init 
				}

				virtual void onInstall (const AppSettings* appSettings, const AppContext* appCtx)
				{
				}

				virtual void onUninstall (void) NO_THROW
				{
				}

				virtual const tchar_t* getLibName (void) const
				{
					return LIB_NAME;
				}
			};
			// 必须实现
			AB_IMPL_NSGETMODULE_WITH_LIB (LIB_NAME, components, ncSharedLibrary);
			
		说明:上诉两个函数(init和fini)一般在modulefactoryDepend.cpp和modulefactoryDepend.h实现,自己测试写demo可不用这两个。
	
	4、编译XPCOM组件
		文件名:
			CMakeLists.txt
		文件内容:
			#project name :openstackapi
			PROJECT(openstackapi)

			#包含prep.cmake(定义编译时的选项)
			INCLUDE($ENV{ABCMAKE}/common/prep.cmake)

			#设置版本号，和向下兼容性
			CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
			CMAKE_POLICY(VERSION 2.8)

			SET(NEED_BASECORE TRUE)
			SET(NEED_APPCORE TRUE)
			SET(NEED_RESTCLIENT  TRUE)
			.....

			INCLUDE_DIRECTORIES(../../private;
								$ENV{APP}/openstack/api/public; // idl文件目录
								.....)
			SOURCE_DIRS(../../private;
						../include;
						.....)

			#target name:openstackapi
			SET(TARGET_NAME openstackapi)

			#TARGET_TYPE 
			#LIBRARY | ARCHIVE | RUNTIME | COMPONENT
			#动态库  | 静态库  | 可执行   |  组件
			SET(TARGET_TYPE COMPONENT)   // 注意:使用XPCOM组件是编译成动态链接库去使用,因此编译XPCOM组件必须设置成编译为 COMPONENT
			SET(CUSTOM_IDL_PATH $ENV{APP}/openstack/api/public;)  // IDL文件目录
			INCLUDE($ENV{ABCMAKE}/common/target.cmake)
			
	5、说明
		XPCOM组件形式是编译成动态链接库,每个组件都需要单独编译成.so,然后在demo里面包含对应实现头文件即可。
	6、例子
		比如此时动态链接组件库已经实现,我们写一个demo,在demo里面包含这个组件实现的头文件即可创建组件对象实现。
		自己写的demo里面需要初始化,即main第一句需要添加以下这个函数:
		
			#include <nsIOpenStackApi.h>   // 使用该组件只需要调用IDL生成的对应头文件即可,不必添加实现该接口类的头文件
			static void InitDll()
			{
				static AppContext appCtx(AB_APPLICATION_NAME);
				AppContext::setInstance(&appCtx);
				AppSettings *appSettings = AppSettings::getCFLAppSettings();
				LibManager::getInstance()-initLibs(appSettings, &appCtx, nullptr);
				
				ncInitXPCOM();
			}
			
			int main(int argc, char *argv[])
			{
				InitDll();
				nsresult result;
				nsCOMPtr <ncIOpenStackApi> openstack = do_CreateInstance(NC_OPENSTACK_API_CONTRACTID, &result);
				
				openstack->Login("hello");
				return 0;
			}
			
三、创建组件对象方法说明
	do_CreateInstance(const nsCID &aCID, nsresult *error = 0);
		参数说明:
			aCID:组件CID,唯一标识一个组件,通过此ID去创建对应组件对象,可以是 NC_OPENSTACK_API_CID 或 NC_OPENSTACK_API_CONTRACTID
			error:错误值,若创建组件对象失败返回对应错误码,成功返回0
	例:
		nsCOMPtr <ncIOpenStackApi> openstack = do_CreateInstance(NC_OPENSTACK_API_CONTRACTID, &result);
		说明: ncIOpenStackApi 为组件类名(idl中声明的名称)
		注意: 创建的组件对象只能调用组件接口里面的对应接口函数,不能调用继承子类的特有函数
	
	
			

		
		
	