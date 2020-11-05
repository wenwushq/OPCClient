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

	bool ConnectServer(const char* szHostName = "127.0.0.1", const char* szProgID = "Matrikon.OPC.Simulation.1");//���ӷ�����
	void MoniterItem(const char* szItemName, ITEMDATATYPE eDataType);//��Ҫ���ӵ�ITEM���Լ��������ֵ����
	void EnumAllItemName();//�������е�ITEM���ʾ���������Ի������
	VARIANT ReadItemValueSync(const char* szItemName);//ͬ����ȡ��ֵ
	bool WriteItemValueSync(const char* szItemName, VARIANT varValue);//ͬ��д��ֵ
	const std::list<const char*>* GetItemNames() { return &m_lstItemNames; }
private:
	void ClearMemory();//�����ڴ�
	void AddGroup(const char* szGroupName = "Group");//�����

private:
	COSERVERINFO m_serverInfo;//��������Ϣ
	MULTI_QI m_mQi;//������������
	IOPCServerList *m_pIServerList;//�������б�ָ��
	IOPCServer *m_pIServer;//������ָ��
	IOPCItemMgt *m_pIItemMgt;//���ָ�룬����������
	IOPCSyncIO *m_pSyncIO;//ͬ����ȡָ��
	std::map<const char*, OPCITEMRESULT*> m_mapItemHandle;//ÿһ���Ӧ��handle��д��ʱ����Ҫ�õ�
	std::list<const char*> m_lstItemNames;
};

