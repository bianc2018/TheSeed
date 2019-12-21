/*
    插件系统定义
    hql 2019/12/14
*/
#ifndef SYSTEM_DEFINE_H_
#define SYSTEM_DEFINE_H_
#include <map>
#include <atomic>

#include <boost/any.hpp>
#include <boost/noncopyable.hpp>


namespace package
{
	/**************************************系统定义*****************************************/
	enum class LogLevel
	{
		LERR,
		LWARN,
		LINFO,
		LDBG,
	};

	class PackageInstance;

	typedef boost::any value;

	//作为参数传递给插件，系统调用 纯虚函数
	class System:boost::noncopyable
	{
		
	public:
		System() {};
		virtual ~System() {};

		//根据变量名称获取变量引用 不存在返回nullptr
		virtual std::shared_ptr<value> ref(const std::string& name)=0;

		//构建变量 如果存在则覆盖
		virtual bool struct_ref(const std::string& name, std::shared_ptr<value> obj)=0;

		//删除
		virtual bool delete_ref(const std::string& name) = 0;

		//打印日志
		virtual void print_log(LogLevel level,const std::string& msg)=0;

		//运行一个实例 失败返回空
		virtual bool run_instance(\
			const std::string packename, std::string instance_name="")=0;

		//停止一个实例的运行
		virtual bool stop_instance(const std::string& instance_name)=0;

		//扫描对应目录下的插件信息并添加
		virtual bool scan_package(const std::string& path)=0;

		//查看包名对应包是否已经加载
		virtual bool is_loaded(const std::string& name) = 0;

		/********************************消息传递******************************/
		//设置消息回调
		virtual bool set_message_callback(\
			std::shared_ptr<PackageInstance> self,\
			std::function<void(const std::string& from, const std::string & message)>) = 0;

		//发送消息 to 格式如 ip:port/instance_name
		virtual bool send_message(const std::string& to, const std::string& message) = 0;

	private:
		//std::hash_map<std::string, std::shared_ptr<value>> ctx_;
	};

	/**************************************插件定义*****************************************/

	//版本
	typedef std::int64_t Ver;
	//基本参数
	struct Basic_info
	{
		//版本
		Ver version;
		//名称
		std::string name;
		//author
		std::string author;

		//依赖项 name -> min version 名称 最小版本
		std::map<std::string, Ver> depends;
	};

	//运行状态
	enum class PACKAGE_INSTANCE_RUN_FLAG
	{
		//用户设置的
		//正在运行
		RUNNING =0,
		//未启动
		UNSTART,
		//已经停止
		STOPED,
		//插件错误 插件已经停止运行
		PACK_ERROR,
		//系统错误 插件必须停止运行
		SYSTEM_ERROR,
		//系统下发退出指令
		QUIT,
	};
	typedef std::atomic<PACKAGE_INSTANCE_RUN_FLAG> PackageRunFlag;
	//插件实例
	class PackageInstance:std::enable_shared_from_this<PackageInstance>
	{
	public:
		PackageInstance(std::shared_ptr<System> sys,const std::string &name)\
			:sys_call_(sys),name_(name),flag_(PACKAGE_INSTANCE_RUN_FLAG::UNSTART){}
		virtual ~PackageInstance() {};

		//运行这个实例
		virtual bool start()=0;

		//停止这个实例
		virtual bool stop() = 0;

		//获取设置标志位
		virtual PACKAGE_INSTANCE_RUN_FLAG get_flag() { return flag_.load(); };
		virtual PACKAGE_INSTANCE_RUN_FLAG set_flag(PACKAGE_INSTANCE_RUN_FLAG flag) \
		{ flag_ = flag; return get_flag(); };

		//获取基本的插件参数
		virtual Basic_info get_basic_info()=0;

		//获取插件依赖
		virtual const std::map<std::string, Ver>& get_depends() = 0;

		//获取系统调用
		virtual std::shared_ptr<System> get_system_call(){ return sys_call_;}
		
		//获取实例名称
		virtual std::string get_name() { return name_; }
	protected:
		std::shared_ptr<System> sys_call_;
	private:
		PackageRunFlag flag_;
		//实例名称
		std::string name_;
	};

	//加载信息
	struct Package_info
	{
		//初始化
		std::function<bool(std::shared_ptr<System> sys)> init;
		
		//产生一个实例 失败返回空
		std::function<std::shared_ptr<PackageInstance> \
			(std::shared_ptr<System> sys,const std::string &name)> generate;
		
		//反初始化
		std::function<bool(std::shared_ptr<System> sys)> uninit;

		Basic_info info;
	};

	//插件加载函数
	typedef bool(*Package_loader_fn)(std::shared_ptr<System> sys,
		std::shared_ptr<Package_info> info);

	//必须有这个函数命名
#define PACKAGE_INIT "Package_loader"
}

#ifdef _WIN32
#define PACKAGE_EXPORT_SYMBOLS extern "C" __declspec(dllexport)
#elif defined(CJSON_EXPORT_SYMBOLS)
#define PACKAGE_EXPORT_SYMBOLS 
#endif

#endif
