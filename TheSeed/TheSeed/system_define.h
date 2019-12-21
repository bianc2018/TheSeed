/*
    ���ϵͳ����
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
	/**************************************ϵͳ����*****************************************/
	enum class LogLevel
	{
		LERR,
		LWARN,
		LINFO,
		LDBG,
	};

	class PackageInstance;

	typedef boost::any value;

	//��Ϊ�������ݸ������ϵͳ���� ���麯��
	class System:boost::noncopyable
	{
		
	public:
		System() {};
		virtual ~System() {};

		//���ݱ������ƻ�ȡ�������� �����ڷ���nullptr
		virtual std::shared_ptr<value> ref(const std::string& name)=0;

		//�������� ��������򸲸�
		virtual bool struct_ref(const std::string& name, std::shared_ptr<value> obj)=0;

		//ɾ��
		virtual bool delete_ref(const std::string& name) = 0;

		//��ӡ��־
		virtual void print_log(LogLevel level,const std::string& msg)=0;

		//����һ��ʵ�� ʧ�ܷ��ؿ�
		virtual bool run_instance(\
			const std::string packename, std::string instance_name="")=0;

		//ֹͣһ��ʵ��������
		virtual bool stop_instance(const std::string& instance_name)=0;

		//ɨ���ӦĿ¼�µĲ����Ϣ�����
		virtual bool scan_package(const std::string& path)=0;

		//�鿴������Ӧ���Ƿ��Ѿ�����
		virtual bool is_loaded(const std::string& name) = 0;

		/********************************��Ϣ����******************************/
		//������Ϣ�ص�
		virtual bool set_message_callback(\
			std::shared_ptr<PackageInstance> self,\
			std::function<void(const std::string& from, const std::string & message)>) = 0;

		//������Ϣ to ��ʽ�� ip:port/instance_name
		virtual bool send_message(const std::string& to, const std::string& message) = 0;

	private:
		//std::hash_map<std::string, std::shared_ptr<value>> ctx_;
	};

	/**************************************�������*****************************************/

	//�汾
	typedef std::int64_t Ver;
	//��������
	struct Basic_info
	{
		//�汾
		Ver version;
		//����
		std::string name;
		//author
		std::string author;

		//������ name -> min version ���� ��С�汾
		std::map<std::string, Ver> depends;
	};

	//����״̬
	enum class PACKAGE_INSTANCE_RUN_FLAG
	{
		//�û����õ�
		//��������
		RUNNING =0,
		//δ����
		UNSTART,
		//�Ѿ�ֹͣ
		STOPED,
		//������� ����Ѿ�ֹͣ����
		PACK_ERROR,
		//ϵͳ���� �������ֹͣ����
		SYSTEM_ERROR,
		//ϵͳ�·��˳�ָ��
		QUIT,
	};
	typedef std::atomic<PACKAGE_INSTANCE_RUN_FLAG> PackageRunFlag;
	//���ʵ��
	class PackageInstance:std::enable_shared_from_this<PackageInstance>
	{
	public:
		PackageInstance(std::shared_ptr<System> sys,const std::string &name)\
			:sys_call_(sys),name_(name),flag_(PACKAGE_INSTANCE_RUN_FLAG::UNSTART){}
		virtual ~PackageInstance() {};

		//�������ʵ��
		virtual bool start()=0;

		//ֹͣ���ʵ��
		virtual bool stop() = 0;

		//��ȡ���ñ�־λ
		virtual PACKAGE_INSTANCE_RUN_FLAG get_flag() { return flag_.load(); };
		virtual PACKAGE_INSTANCE_RUN_FLAG set_flag(PACKAGE_INSTANCE_RUN_FLAG flag) \
		{ flag_ = flag; return get_flag(); };

		//��ȡ�����Ĳ������
		virtual Basic_info get_basic_info()=0;

		//��ȡ�������
		virtual const std::map<std::string, Ver>& get_depends() = 0;

		//��ȡϵͳ����
		virtual std::shared_ptr<System> get_system_call(){ return sys_call_;}
		
		//��ȡʵ������
		virtual std::string get_name() { return name_; }
	protected:
		std::shared_ptr<System> sys_call_;
	private:
		PackageRunFlag flag_;
		//ʵ������
		std::string name_;
	};

	//������Ϣ
	struct Package_info
	{
		//��ʼ��
		std::function<bool(std::shared_ptr<System> sys)> init;
		
		//����һ��ʵ�� ʧ�ܷ��ؿ�
		std::function<std::shared_ptr<PackageInstance> \
			(std::shared_ptr<System> sys,const std::string &name)> generate;
		
		//����ʼ��
		std::function<bool(std::shared_ptr<System> sys)> uninit;

		Basic_info info;
	};

	//������غ���
	typedef bool(*Package_loader_fn)(std::shared_ptr<System> sys,
		std::shared_ptr<Package_info> info);

	//�����������������
#define PACKAGE_INIT "Package_loader"
}

#ifdef _WIN32
#define PACKAGE_EXPORT_SYMBOLS extern "C" __declspec(dllexport)
#elif defined(CJSON_EXPORT_SYMBOLS)
#define PACKAGE_EXPORT_SYMBOLS 
#endif

#endif
