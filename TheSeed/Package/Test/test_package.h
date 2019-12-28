#include "system_define.h"
using namespace package;
//包实例
class TestPackage:public package::PackageInstance
{
public:
	TestPackage(std::shared_ptr<System> sys, const std::string& name);
	~TestPackage();

	//运行这个实例 输入当前实例的名称
	virtual bool start() override;

	//停止这个实例
	virtual bool stop()override;

	//获取基本的插件参数
	virtual Basic_info get_basic_info()override;

	//获取插件依赖
	virtual  std::map<std::string, Ver> get_depends()override;

	//事件回调
	void event_cb(std::shared_ptr<SystemEvent> message);
};

PACKAGE_EXPORT_SYMBOLS bool Package_loader(std::shared_ptr<System> sys,
	std::shared_ptr<Package_info> info);
