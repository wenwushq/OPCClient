#pragma once
#include "./opcfoundation/opccomn.h"
#include "./opcfoundation/opcda.h"
#include "./opcfoundation/OpcEnum.h"
#include <map>
#include <list>

enum ITEMDATATYPE
{
	Data_bool,
	Data_int,
	Data_float,
	Data_double,
	Data_string
};

class COpcClient
{
public:
	COpcClient();
	~COpcClient();

	bool ConnectServer(const char* szHostName = "127.0.0.1", const char* szProgID = "Matrikon.OPC.Simulation.1");//连接服务器
	void MoniterItem(const char* szItemName, ITEMDATATYPE eDataType);//需要监视的ITEM项以及这项的数值类型
	void EnumAllItemName();//遍历所有的ITEM项并显示，用做调试或测试用
	VARIANT ReadItemValueSync(const char* szItemName);//同步读取数值
	bool WriteItemValueSync(const char* szItemName, VARIANT varValue);//同步写数值
	const std::list<const char*>* GetItemNames() { return &m_lstItemNames; }
private:
	void ClearMemory();//清理内存
	void AddGroup(const char* szGroupName = "Group");//添加组

private:
	COSERVERINFO m_serverInfo;//服务器信息
	MULTI_QI m_mQi;//服务器的引用
	IOPCServerList *m_pIServerList;//服务器列表指针
	IOPCServer *m_pIServer;//服务器指针
	IOPCItemMgt *m_pIItemMgt;//项的指针，控制项的添加
	IOPCSyncIO *m_pSyncIO;//同步读取指针
	std::map<const char*, OPCITEMRESULT*> m_mapItemHandle;//每一项对应的handle读写的时候需要用到
	std::list<const char*> m_lstItemNames;
};

