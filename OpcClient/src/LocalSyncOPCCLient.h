#pragma once
#ifndef OPC_LOCAL_CLIENT_H
#define OPC_LOCAL_CLIENT_H

#include <vector>
#include <string>
#include <list>
#include <map>
#include <cstdint>

#include "./3rdparty/OPCClientToolKit/opcda.h"
#include "./3rdparty/OPCClientToolKit/OPCClient.h"
#include "./3rdparty/OPCClientToolKit/OPCHost.h"
#include "./3rdparty/OPCClientToolKit/OPCServer.h"
#include "./3rdparty/OPCClientToolKit/OPCGroup.h"
#include "./3rdparty/OPCClientToolKit/OPCItem.h"

#include "opcclient_global.h"

URUTIL_BEGIN_NAMESPACE
class OPCCLIENT_EXPORT LocalSyncOPCCLient
{
public:
	// constructor and destructor
	LocalSyncOPCCLient();
	virtual ~LocalSyncOPCCLient();

	// Controller actions
	bool Init();
	bool Connect(const std::string& serverName,const std::string& groupName,const std::list<std::string>& itemNames);// connect to local server
	bool Connect(const std::string& remoteHostName,const std::string& serverName,const std::string& groupName,const std::list<std::string>& itemNames); //connect to remote server
	bool DisConnect();
	bool IsConnected();
	bool Stop();

	// OPC API
	bool IsOPCRuning();
	virtual bool IsOPCConnectedPLC();// this function relis on the specific device
	virtual bool ItemNameFilter(std::string item_name);// if a item's name cannot pass the name fitler, it will not be added.

	// Basic Read Write API
	inline bool SyncWriteItem(std::string item_name, VARIANT* var);
	inline bool SyncReadItem(std::string item_name, VARIANT* var);
	bool SyncReadItems(const std::list<std::string>& item_names, std::list<VARIANT>& varList);

	bool ReadBool(std::string item_name);
	bool WriteBool(std::string item_name, bool item_value);

	float ReadFloat(std::string item_name);
	bool WirteFloat(std::string item_name, float item_value);

	uint16_t ReadUint16(std::string item_name);
	bool WriteUint16(std::string item_name, uint16_t item_value);

	bool ReadUint16Array(std::string item_name, uint16_t* item_value_array, int array_size);
	bool WriteUint16Array(std::string item_name, uint16_t* item_value_array, int array_size);

protected:
	std::map<std::string, COPCItem *> name_item_map_;
	COPCGroup* p_group_;
	unsigned long refresh_rate_;
	COPCHost* p_host_;
	COPCServer* p_opc_server_;
	bool is_env_ready;
	std::vector<std::string> item_name_vector_;

	bool MakeGroupAndAddItems(const std::string& ServerName,const std::string& groupName,const std::list<std::string>& itemNames);
	bool CleanOPCMember();
	bool DetectService(char* ServiceName);
	bool DetectService(char* RemoteHostName,char* ServiceName);
	VARIANT read_tmp_variant_;
};

URUTIL_END_NAMESPACE
#endif //OPCONTROLLER_H
