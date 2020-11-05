#include "COpcClient.h"
#include <atlconv.h>
#include <string>
#include <iostream>

COpcClient::COpcClient()
{
	HRESULT result = ::CoInitialize(nullptr);//创建com对象
	if (FAILED(result))
	{
		std::cout << "未开启COM组件注册..." << std::endl;
		return;
	}
	std::cout << "已开启COM组件注册..." << std::endl;
	std::cout << "已登陆COM组件..." << std::endl;

	ZeroMemory(&m_serverInfo, sizeof(m_serverInfo));
	ZeroMemory(&m_mQi, sizeof(m_mQi));
	m_pIServerList = NULL;
	m_pIServer = NULL;
	m_pIItemMgt = NULL;
	m_pSyncIO = NULL;
}

COpcClient::~COpcClient()
{
	::CoUninitialize();
	std::cout << "退出COM组件..." << std::endl;
	ClearMemory();
}

bool COpcClient::ConnectServer(const char * szHostName, const char* szProgID)
{
	ClearMemory();

	USES_CONVERSION;
	m_serverInfo.pwszName = A2W(szHostName);
	m_mQi.pIID = &IID_IOPCServerList;
	HRESULT hr = ::CoCreateInstanceEx(CLSID_OpcServerList, NULL, CLSCTX_ALL, &m_serverInfo, 1, &m_mQi);
	if (FAILED(hr))
	{
		std::cout << "获取目标服务器端OPC服务器列表失败..." << std::endl;
		ClearMemory();
		return false;
	}
	std::cout << "获取目标服务器端OPC服务器列表成功" << std::endl;

	m_pIServerList = (IOPCServerList*)m_mQi.pItf;

	//根据PROGID得到CLSID
	CLSID clsID;
	hr = m_pIServerList->CLSIDFromProgID(A2COLE(szProgID), &clsID);
	if (FAILED(hr))
	{
		std::cout << "未找到需要连接的OPC服务器..." << std::endl;
		ClearMemory();
		return false;
	}
	std::cout << "已找到需要连接的OPC服务器..." << std::endl;

	//连接OPC服务器
	ZeroMemory(&m_mQi, sizeof(m_mQi));
	m_mQi.pIID = &IID_IOPCServer;
	hr = CoCreateInstanceEx(clsID, NULL, CLSCTX_ALL, &m_serverInfo, 1, &m_mQi);
	if (FAILED(hr))
	{
		std::cout << "未连接上需要连接的OPC服务器..." << std::endl;
		ClearMemory();
		return false;
	}
	std::cout << "已连接上需要连接的OPC服务器..." << std::endl;

	hr = ((IUnknown*)m_mQi.pItf)->QueryInterface(IID_IOPCServer, (void**)&m_pIServer);
	if (FAILED(hr))
	{
		ClearMemory();
		return false;
	}
	AddGroup();//添加组

	return true;
}

void COpcClient::MoniterItem(const char * szItemName, ITEMDATATYPE eDataType)
{
	if (!m_pIItemMgt) return;
	USES_CONVERSION;
	OPCITEMDEF item;
	ZeroMemory(&item, sizeof(item));
	item.szItemID = A2W(szItemName);
	switch (eDataType)
	{
	case Data_bool:
		item.vtRequestedDataType = VT_BOOL;
		break;
	case Data_int:
		item.vtRequestedDataType = VT_I4;
		break;
	case Data_float:
		item.vtRequestedDataType = VT_R4;
		break;
	case Data_double:
		item.vtRequestedDataType = VT_R8;
		break;
	case Data_string:
		item.vtRequestedDataType = VT_BSTR;
		break;
	default:
		item.vtRequestedDataType = VT_R4;
		break;
	}
	OPCITEMRESULT *pItemResult = NULL;
	HRESULT *pErrors = NULL;
	HRESULT hr = m_pIItemMgt->AddItems(1, &item, &pItemResult, (HRESULT**)&pErrors);
	if(SUCCEEDED(hr))
		m_mapItemHandle[szItemName] = pItemResult;
	CoTaskMemFree(pErrors);
}

//遍历所有的ITEM项并显示，用做调试或测试用
void COpcClient::EnumAllItemName()
{
	m_lstItemNames.clear();
	if (!m_pIServer) return;

	IOPCBrowseServerAddressSpace* pIopcAddressSpace;
	HRESULT hr = m_pIServer->QueryInterface(IID_IOPCBrowseServerAddressSpace, (void**)&pIopcAddressSpace);
	if (FAILED(hr))
		return;

	IEnumString *pIEnumStrings;
	hr = pIopcAddressSpace->BrowseOPCItemIDs(OPC_FLAT, L"", VT_EMPTY, 0, &pIEnumStrings);
	if (FAILED(hr))
		return;

	WCHAR * strName;
	ULONG nCount;
	USES_CONVERSION;
	while ((hr = pIEnumStrings->Next(1, &strName, &nCount)) == S_OK)
	{
		std::cout << W2A(strName) << std::endl;
		m_lstItemNames.push_back(W2A(strName));
	}

	if (pIopcAddressSpace) pIopcAddressSpace->Release();
	pIopcAddressSpace = NULL;
}

//同步读取数值
VARIANT COpcClient::ReadItemValueSync(const char * szItemName)
{
	VARIANT varRet = VARIANT();

	HRESULT *pErrors = NULL;
	OPCITEMSTATE *pItemValue = NULL;
	OPCITEMRESULT *pItemResult = m_mapItemHandle[szItemName];
	if (!pItemResult)
		return varRet;

	HRESULT hr = m_pSyncIO->Read(OPC_DS_CACHE, 1, &pItemResult->hServer, &pItemValue, &pErrors);

	if (SUCCEEDED(hr))
		varRet = pItemValue->vDataValue;

	CoTaskMemFree(pItemValue);
	CoTaskMemFree(pErrors);
	std::cout << V_I4(&varRet) << std::endl;
	return varRet;
}

//同步写数值
bool COpcClient::WriteItemValueSync(const char * szItemName, VARIANT varValue)
{
	bool bRet = false;
	HRESULT *pErrors = NULL;
	OPCITEMRESULT *pItemResult = m_mapItemHandle[szItemName];
	if (!pItemResult)
		return false;

	HRESULT hr = m_pSyncIO->Write(1, &pItemResult->hServer, &varValue, &pErrors);
	if (SUCCEEDED(hr))
		bRet = true;

	CoTaskMemFree(pErrors);

	return bRet;
}

//清理内存
void COpcClient::ClearMemory()
{
	if (m_pIServerList) m_pIServerList->Release();
	m_pIServerList = NULL;
	if (m_pIServer) m_pIServer->Release();
	m_pIServer = NULL;
	if (m_pIItemMgt) m_pIItemMgt->Release();
	m_pIItemMgt = NULL;
	if (m_pSyncIO) m_pSyncIO->Release();
	m_pSyncIO = NULL;
	std::map<const char*, OPCITEMRESULT*>::iterator iter;
	for (iter = m_mapItemHandle.begin(); iter != m_mapItemHandle.end(); iter++)
		CoTaskMemFree(iter->second);
	m_mapItemHandle.clear();
}

//添加组
void COpcClient::AddGroup(const char * szGroupName)
{
	USES_CONVERSION;
	long lTimeBias = 0;
	float fTemp = 0.0f;
	OPCHANDLE hOpcServer1;
	DWORD dwActualRate = 0;
	HRESULT hr = m_pIServer->AddGroup(A2W(szGroupName), TRUE, 10, 0, &lTimeBias, &fTemp, 0, &hOpcServer1,
		&dwActualRate, IID_IOPCItemMgt, (LPUNKNOWN*)&m_pIItemMgt);
	if (FAILED(hr))
	{
		std::cout << "未能添加组..." << std::endl;
		ClearMemory();
		return;
	}
	std::cout << "已添加组..." << std::endl;
	hr = m_pIItemMgt->QueryInterface(IID_IOPCSyncIO, (void**)&m_pSyncIO);
	if (FAILED(hr))
		ClearMemory();
}