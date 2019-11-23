# TheSeed
游戏之作

1.分布式系统
2.通信协议
	tcp 协议，http协议
	服务端、客户端
3.模块化 根据输入的脚本执行不同的功能 统一接口
4.加密校验

1.如包管理一样 分布式 管理插件
2.插件运行统一接口 exec(std::string json ) 通过json传输交互 event 执行
3.服务端和客户端之间通过json交互数据
	
1.异步事件模块
	event.hpp
//2.log模块
3.数据传输模块 tcp asio 即可
	根据http 改良协议
三个服务器
 接口:指令通道服务器  心跳 KL 指令传输 ORDER 文件传输 TRAN 数据传输通道服务器
4.指令系统 下载 上传 执行 json 表达顺序执行 
[
	{
		"cmd":"down"
		"params":
		{
			"type":"file"
			"value":"...."
		}
	},

]
	
5.插件系统 插件 接口 传入json 字符串 和系统调用 包管理 
	包管理：每个包有唯一的名字 和版本号 及 依赖列表 
		只有一个执行接口 int exec(std::string&in_json,const std::string &msg,const system_call*call);
		获取包名 std::string name();
		获取依赖项 vector<std::string> depends();
	

数据传输 
客户端 
	只发送数据 可以是请求 也可以是 回复 
	请求 带有 token  和 src 源节点  没有两个中任意一个字段则是此请求无需回复 
	回复必须带有 token 来自 请求 发往 src 来自请求
服务端 只接收数据 
	收到请求 根据 cmd 字段 选择不同的执行函数，创建一个客户端进行回复。
	收到回复 根据 token字段 触发发送时设置的回调 根据 close true/false 字段关闭回调