#pragma once

#include "WlanXmlUtil.h"

class WlanManager {
public:
  WlanManager();
  ~WlanManager();

  bool Connect(const std::string& ssid, const std::string& username, const std::string& password);

private:
  int DisconnectWlan(const std::string& ssid, const HANDLE client, const PWLAN_INTERFACE_INFO net_card);
  bool ConnectWlan(const std::string& username, const std::string& password,
    const HANDLE client, const PWLAN_INTERFACE_INFO net_card, const PWLAN_AVAILABLE_NETWORK wlan);

  bool SetProfile(const std::string& username, const std::string& password,
    const HANDLE client, const PWLAN_INTERFACE_INFO net_card, const PWLAN_AVAILABLE_NETWORK wlan);
};

