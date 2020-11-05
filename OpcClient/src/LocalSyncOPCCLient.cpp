#include "LocalSyncOPCCLient.h"
#include <Winsvc.h>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

URUTIL_BEGIN_NAMESPACE
LocalSyncOPCCLient::LocalSyncOPCCLient()
{
	p_group_ = nullptr;
	refresh_rate_ = 0;
	p_host_ = nullptr;
	p_opc_server_ = nullptr;
	is_env_ready = false;
}

LocalSyncOPCCLient::~LocalSyncOPCCLient()
{
	DisConnect();
	Stop();
}

bool LocalSyncOPCCLient::Init()
{
	COPCClient::init();
	return true;
}

bool LocalSyncOPCCLient::Stop()
{
	COPCClient::stop();
	return true;
}

bool LocalSyncOPCCLient::Connect(const std::string& serverName,const std::string& groupName,const std::list<std::string>& itemNames)
{
	// check if opc service runing
	if (!DetectService((char*)serverName.data()))
	{
		// can do it when run at GuiYang! [10/24/2017 WQ]
		//return false;
	}
	is_env_ready = true;

	// create local host
	p_host_ = COPCClient::makeHost("");

	return MakeGroupAndAddItems(serverName,groupName,itemNames);
}

bool LocalSyncOPCCLient::Connect(const std::string& remoteHostName,const std::string& serverName,const std::string& groupName,const std::list<std::string>& itemNames)
{
	// check if opc service runing
	if (!DetectService((char*)remoteHostName.data(),(char*)serverName.data()))
	{
		// can do it when run at GuiYang! [10/24/2017 WQ]
		//return false;
	}
	is_env_ready = true;

	// create remote host
	//WSADATA wsData;
	//::WSAStartup(MAKEWORD(2, 2), &wsData);
	//struct hostent *hp;
	//hp = gethostbyname(remoteHostName.data());

	p_host_ = COPCClient::makeHost(remoteHostName);
	return MakeGroupAndAddItems(serverName,groupName,itemNames);
}

bool LocalSyncOPCCLient::IsConnected()
{
	if (!is_env_ready)
	{
		return false;
	}
	// check is opc server runing
	if (IsOPCRuning())
	{
		if (IsOPCConnectedPLC())
		{
			return true;
		}
	}
	return false;
}

bool LocalSyncOPCCLient::DisConnect()
{
	if (!is_env_ready)
	{
		return true;
	}

	CleanOPCMember();
	return true;
}

bool LocalSyncOPCCLient::IsOPCRuning()
{
	if (p_opc_server_ == nullptr)
	{
		return false;
	}
	ServerStatus status;
	p_opc_server_->getStatus(status);
	if (status.dwServerState != OPC_STATUS_RUNNING)
	{
		return false;
	}
	return true;
}

bool LocalSyncOPCCLient::IsOPCConnectedPLC()
{
	return true;
}

//make group and add items
bool LocalSyncOPCCLient::MakeGroupAndAddItems(const std::string& ServerName,const std::string& GroupName,const std::list<std::string>& ItemNames)
{
	//  connect to server and get item names
	std::vector<std::string> serverList;
	p_host_->getListOfDAServers(IID_CATID_OPCDAServer20, serverList);
	if (serverList.size() == 0)
		//		|| std::find(localServerList.begin(),localServerList.end(),serverName) == localServerList.end())
	{
		return false;
	}

	p_opc_server_ = p_host_->connectDAServer(ServerName);

	p_opc_server_->getItemNames(item_name_vector_);

	// make group
	p_group_ = p_opc_server_->makeGroup(GroupName, true, 1000, refresh_rate_, 0.0);

	// add items
	std::string itemName;
	for (std::list<std::string>::const_iterator it = ItemNames.begin();it != ItemNames.end();it++)
	{
		itemName = *it;
		if (ItemNameFilter(itemName))
			name_item_map_[itemName] = p_group_->addItem(itemName, true);
	}
	if (!IsConnected())
	{
		CleanOPCMember();
		return false;
	}
	VariantInit(&read_tmp_variant_);
	return true;
}

// check service status 
bool LocalSyncOPCCLient::DetectService(char* ServiceName)
{
	// open manager
	SC_HANDLE hSC = ::OpenSCManager(NULL, NULL, GENERIC_EXECUTE);
	if (hSC == NULL)
	{
		return false;
	}
	// open service
	WCHAR wszServiceName[256];  
	memset(wszServiceName,0,sizeof(wszServiceName));  
	MultiByteToWideChar(CP_ACP,0,ServiceName,strlen(ServiceName)+1,wszServiceName,  
		sizeof(wszServiceName)/sizeof(wszServiceName[0]));

	SC_HANDLE hSvc = ::OpenService(hSC, wszServiceName,
	                               SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);
	if (hSvc == NULL)
	{
		return false;
	}
	// read status
	SERVICE_STATUS status;
	if (::QueryServiceStatus(hSvc, &status) != FALSE)
	{
		return true;
	}


	return false;
}

// check remote service status 
bool LocalSyncOPCCLient::DetectService(char* RemoteHostName,char* ServiceName)
{
	// convert char* to lpcwstr
	WCHAR wszRemoteHost[256];  
	memset(wszRemoteHost,0,sizeof(wszRemoteHost));  
	MultiByteToWideChar(CP_ACP,0,RemoteHostName,strlen(RemoteHostName)+1,wszRemoteHost,
		sizeof(wszRemoteHost)/sizeof(wszRemoteHost[0]));

	// open manager
	SC_HANDLE hSC = ::OpenSCManager(wszRemoteHost, NULL, GENERIC_EXECUTE);
	if (hSC == NULL)
	{
		return false;
	}
	// open service
	WCHAR wszServiceName[256];  
	memset(wszServiceName,0,sizeof(wszServiceName));  
	MultiByteToWideChar(CP_ACP,0,ServiceName,strlen(ServiceName)+1,wszServiceName,  
		sizeof(wszServiceName)/sizeof(wszServiceName[0]));

	SC_HANDLE hSvc = ::OpenService(hSC, wszServiceName,
		SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);
	if (hSvc == NULL)
	{
		return false;
	}
	// read status
	SERVICE_STATUS status;
	if (::QueryServiceStatus(hSvc, &status) != FALSE)
	{
		return true;
	}

	return false;
}


bool LocalSyncOPCCLient::ItemNameFilter(std::string item_name)
{
	std::vector<std::string>::iterator iter = find(item_name_vector_.begin(),item_name_vector_.end(),item_name);
	if (iter == item_name_vector_.end())
		return false;
	return true;
}

bool LocalSyncOPCCLient::SyncWriteItem(std::string item_name, VARIANT* var)
{
	name_item_map_[item_name]->writeSync(*var);
	return true;
}

bool LocalSyncOPCCLient::SyncReadItem(std::string item_name, VARIANT* var)
{
	static OPCItemData data;
	if (name_item_map_.find(item_name) == name_item_map_.end())
		return false;

	name_item_map_[item_name]->readSync(data, OPC_DS_DEVICE);
	VariantCopy(var, &data.vDataValue);
	return true;
}

bool LocalSyncOPCCLient::SyncReadItems(const std::list<std::string>& item_names, std::list<VARIANT>& varList)
{
	std::list<OPCItemData> dataList;
	std::vector<COPCItem *> items;
	COPCItem_DataMap opcData;

	for (std::list<std::string>::const_iterator it = item_names.begin();it != item_names.end();it++)
	{
		if (name_item_map_.find(*it) == name_item_map_.end())
			items.push_back(0);
		else
			items.push_back(name_item_map_[*it]);
	}

	p_group_->readSync(items,opcData,OPC_DS_DEVICE);

	COPCItem_DataMap::CPair* pos;
	OPCItemData* readData;
	for (int num = 0;num < items.size();num++)
	{
		pos = opcData.Lookup(items[num]);
		if (pos){
			readData = opcData.GetValueAt(pos);
			if (readData && !FAILED(readData->error)){
				dataList.push_back(*readData);
			}
			else
			{
				qDebug() << QString("read data of item:%1 failed!").arg(QString::fromLocal8Bit(items[num]->getName().c_str()));
				dataList.push_back(OPCItemData());
			}
		}
		else
			dataList.push_back(OPCItemData());
	}

	VARIANT var;
	for (std::list<OPCItemData>::const_iterator it = dataList.begin();it != dataList.end();it++)
	{
		VariantCopy(&var, &(*it).vDataValue);
		varList.push_back(var);
	}

	return true;
}

bool LocalSyncOPCCLient::ReadBool(std::string item_name)
{
	SyncReadItem(item_name, &read_tmp_variant_);
	return read_tmp_variant_.boolVal;
}

bool LocalSyncOPCCLient::WriteBool(std::string item_name, bool item_value)
{
	static VARIANT var;
	static VARTYPE a = (var.vt = VT_BOOL);
	var.boolVal = item_value;
	SyncWriteItem(item_name, &var);
	return true;
}

float LocalSyncOPCCLient::ReadFloat(std::string item_name)
{
	SyncReadItem(item_name, &read_tmp_variant_);
	return read_tmp_variant_.fltVal;
}

bool LocalSyncOPCCLient::WirteFloat(std::string item_name, float item_value)
{
	static VARIANT var;
	static VARTYPE a = (var.vt = VT_R4);
	var.fltVal = item_value;
	SyncWriteItem(item_name, &var);
	return true;
}


uint16_t LocalSyncOPCCLient::ReadUint16(std::string item_name)
{
	SyncReadItem(item_name, &read_tmp_variant_);
	return read_tmp_variant_.uiVal;
}


bool LocalSyncOPCCLient::WriteUint16(std::string item_name, uint16_t item_value)
{
	static VARIANT var;
	static VARTYPE a = (var.vt = VT_UI2);
	var.uiVal = item_value;
	SyncWriteItem(item_name, &var);
	return true;
}


bool LocalSyncOPCCLient::ReadUint16Array(std::string item_name, uint16_t* item_value_array, int array_size)
{
	VARIANT array_variant;
	VariantInit(&array_variant);
	SyncReadItem(item_name, &array_variant);

	uint16_t* buf;
	SafeArrayAccessData(array_variant.parray, (void **)&buf);
	for (int i = 0; i < array_size; ++i)
	{
		item_value_array[i] = buf[i];
	}
	SafeArrayUnaccessData(array_variant.parray);
	VariantClear(&array_variant);
	return true;
}

bool LocalSyncOPCCLient::WriteUint16Array(std::string item_name, uint16_t* item_value_array, int array_size)
{
	// create variant and safe array
	VARIANT array_variant;
	VariantInit(&array_variant);
	VARTYPE a = (array_variant.vt = VT_ARRAY | VT_UI2);
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].cElements = array_size;
	rgsabound[0].lLbound = 0;
	array_variant.parray = SafeArrayCreate(VT_UI2, 1, rgsabound);
	// copy data
	uint16_t* buf;
	SafeArrayAccessData(array_variant.parray, (void **)&buf);
	for (int i = 0; i < array_size; ++i)
	{
		buf[i] = item_value_array[i];
	}
	SafeArrayUnaccessData(array_variant.parray);
	//write variant
	SyncWriteItem(item_name, &array_variant);
	VariantClear(&array_variant);
	return true;
}

bool LocalSyncOPCCLient::CleanOPCMember()
{
	if (IsOPCRuning()) // delete heap if connected
	{
		for(auto iter=name_item_map_.begin();iter!=name_item_map_.end();++iter)
		{
			delete iter->second;
		}
		name_item_map_.clear();

		if (p_group_)
		{
			delete p_group_;
			p_group_ = nullptr;
		}
		
		if (p_host_)
		{
			delete p_host_;
			p_host_ = nullptr;
		}
		
		if (p_opc_server_)
		{
			delete p_opc_server_;
			p_opc_server_ = nullptr;
		}
	}
	
	refresh_rate_ = 0;
	is_env_ready = false;
	return true;
}

URUTIL_END_NAMESPACE