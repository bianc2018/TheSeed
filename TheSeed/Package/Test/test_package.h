#include "system_define.h"
using namespace package;
//��ʵ��
class TestPackage:public package::PackageInstance
{
public:
	TestPackage(std::shared_ptr<System> sys, const std::string& name);
	~TestPackage();

	//�������ʵ�� ���뵱ǰʵ��������
	virtual bool start() override;

	//ֹͣ���ʵ��
	virtual bool stop()override;

	//��ȡ�����Ĳ������
	virtual Basic_info get_basic_info()override;

	//��ȡ�������
	virtual  std::map<std::string, Ver> get_depends()override;

	//�¼��ص�
	void event_cb(std::shared_ptr<SystemEvent> message);
};

PACKAGE_EXPORT_SYMBOLS bool Package_loader(std::shared_ptr<System> sys,
	std::shared_ptr<Package_info> info);
