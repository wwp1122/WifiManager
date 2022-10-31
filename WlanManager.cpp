#include "WlanManager.h"



WlanManager::WlanManager()
{
}


WlanManager::~WlanManager()
{
}

bool WlanManager::Connect(const std::string& ssid, const std::string& username, const std::string& password) {
  HANDLE client = NULL;
  DWORD result = ERROR_SUCCESS;
  DWORD service_version;
  if ((result = WlanOpenHandle(WLAN_API_VERSION, NULL, &service_version, &client)) != ERROR_SUCCESS) {
    printf("WlanOpenHandle failed, error :%d\n", result);
    return false;
  }

  PWLAN_INTERFACE_INFO_LIST net_card_list = NULL;
  PWLAN_AVAILABLE_NETWORK_LIST wlan_list = NULL;

  auto DeletePointer = [&](bool result) {
    if (wlan_list != NULL) {
      WlanFreeMemory(wlan_list);
      wlan_list = NULL;
    }
    if (net_card_list != NULL) {
      WlanFreeMemory(net_card_list);
      net_card_list = NULL;
    }
    if (client != NULL) {
      WlanCloseHandle(client, NULL);
      client = NULL;
    }
    return result;
  };

  if ((result = WlanEnumInterfaces(client, NULL, &net_card_list)) != ERROR_SUCCESS) {
    printf("WlanEnumInterfaces failed, error :%d\n", result);
    return DeletePointer(false);
  }

  PWLAN_INTERFACE_INFO net_card = NULL;
  PWLAN_AVAILABLE_NETWORK wlan = NULL;
  bool find_wlan = false;
  int i;
  for (i = 0; i < net_card_list->dwNumberOfItems; ++i) {
    net_card = (PWLAN_INTERFACE_INFO)&net_card_list->InterfaceInfo[i];

    if (ERROR_SUCCESS != WlanGetAvailableNetworkList(client, &net_card->InterfaceGuid, 0x00, NULL, &wlan_list)) {
      return DeletePointer(false);
    }
    //等待wlan查询结果
    int wait_time = 8;
    for (int s = 0; s < wait_time; ++s) {
      printf("%d\n", wait_time - s);
      ::Sleep(1000);
    }
    for (int j = 0; j < wlan_list->dwNumberOfItems; ++j) {
      wlan = (PWLAN_AVAILABLE_NETWORK)&wlan_list->Network[j];
      if (strcmp(ssid.c_str(), (char*)wlan->dot11Ssid.ucSSID) != 0) {
        continue;
      }
      find_wlan = true;
      break;
    }
    if (find_wlan) {
      break;
    }
  }

  if (!find_wlan) {
    printf("Does not find wlan\n");
    return false;
  }

  if (net_card->isState == wlan_interface_state_connected) {
    int ret = DisconnectWlan(ssid, client, net_card);
    if (ret != 0) {
      return DeletePointer(-1 == ret);
    }
    while (1) {
      WlanFreeMemory(net_card_list);
      if ((result = WlanEnumInterfaces(client, NULL, &net_card_list)) != ERROR_SUCCESS) {
        printf("WlanEnumInterfaces failed, error :%d\n", result);
        break;
      }

      net_card = (PWLAN_INTERFACE_INFO)&net_card_list->InterfaceInfo[i];
      if (net_card->isState == wlan_interface_state_disconnected) {
        break;
      }
      ::Sleep(100);
    }
  }


  if (net_card->isState == wlan_interface_state_disconnected) {
    if (!ConnectWlan(username, password, client, net_card, wlan)) {
      return DeletePointer(false);
    }

    DWORD dwTimeout = GetTickCount64();
    while (1) {
      WlanFreeMemory(net_card_list);
      if (ERROR_SUCCESS != WlanEnumInterfaces(client, NULL, &net_card_list)) {
        printf("WlanEnumInterfaces failed, error :%d\n", result);
        return DeletePointer(false);
      }

      net_card = (PWLAN_INTERFACE_INFO)&net_card_list->InterfaceInfo[i];
      if (net_card->isState == wlan_interface_state_connected) {
        printf("Wlan connected\n");
        return DeletePointer(true);
      }
      if ((GetTickCount64() - dwTimeout) >= 30000) {
        printf("Wait wlan coonect timeout\n");
        return DeletePointer(false);
      }
      ::Sleep(100);
    }
  }

  printf("Wlan connect timeout\n");
  return DeletePointer(false);
}

int WlanManager::DisconnectWlan(const std::string& ssid, const HANDLE client, const PWLAN_INTERFACE_INFO net_card) {
  PWLAN_CONNECTION_ATTRIBUTES connected_wlan = NULL;
  DWORD dwSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
  WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;
  DWORD result = WlanQueryInterface(client, &net_card->InterfaceGuid, wlan_intf_opcode_current_connection,
    NULL, &dwSize, (PVOID*)&connected_wlan, &opCode);

    if (ERROR_SUCCESS == result &&
      connected_wlan->wlanAssociationAttributes.dot11Ssid.uSSIDLength > 0 &&
      ssid == (const char*)connected_wlan->wlanAssociationAttributes.dot11Ssid.ucSSID) {
      printf("Wlan is connected\n");
      return -1;
  }
    result = WlanDisconnect(client, &net_card->InterfaceGuid, NULL);
    if (ERROR_SUCCESS == result) {
      return 0;
    }

  return result;
}

bool WlanManager::ConnectWlan(const std::string& username, const std::string& password,
  const HANDLE client, const PWLAN_INTERFACE_INFO net_card, const PWLAN_AVAILABLE_NETWORK wlan) {
  if (!SetProfile(username, password, client, net_card, wlan)) {
    return false;
  }

  WLAN_CONNECTION_PARAMETERS wlanConnPara;
  wlanConnPara.wlanConnectionMode = wlan_connection_mode_profile;
  wlanConnPara.strProfile = WlanXmlUtil::StringToWstring((const char*)wlan->dot11Ssid.ucSSID).c_str();			// 指定的用户文件
  wlanConnPara.pDot11Ssid = &wlan->dot11Ssid;		//指定的SSID
  wlanConnPara.dot11BssType = wlan->dot11BssType; //网络类型
  wlanConnPara.pDesiredBssidList = NULL;
  wlanConnPara.dwFlags = 0x00000000;
  if (ERROR_SUCCESS != WlanConnect(client, &net_card->InterfaceGuid, &wlanConnPara, NULL)) {
    printf("WlanConnect failed\n");
    return false;
  }

  return true;
}

bool WlanManager::SetProfile(const std::string& username, const std::string& password,
  const HANDLE client, const PWLAN_INTERFACE_INFO net_card, const PWLAN_AVAILABLE_NETWORK wlan) {
  std::wstring data = WlanXmlUtil::StringToWstring(WlanXmlUtil::GetProfileXml(wlan));
  DWORD reason;
  DWORD result = WlanSetProfile(client, &net_card->InterfaceGuid, 0x00,
    data.c_str(), NULL, TRUE, NULL, &reason);

  if (result != ERROR_SUCCESS && reason != ERROR_ALREADY_EXISTS) {
    printf("WlanSetProfile failed, error:%d, reason:%d\n", result, reason);
    return false;
  }

  data = WlanXmlUtil::StringToWstring(WlanXmlUtil::GetCredentialsXml(username, password));
  if (ERROR_SUCCESS != WlanSetProfileEapXmlUserData(client, &net_card->InterfaceGuid,
    WlanXmlUtil::StringToWstring((const char*)(wlan->dot11Ssid.ucSSID)).c_str(), 0, data.c_str(), 0)) {
    printf("WlanSetProfileEapXmlUserData failed\n");
    return false;
  }
  return true;
}