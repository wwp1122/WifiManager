#pragma once
#include <string>
#include <windows.h>
#include <wlanapi.h>
class WlanXmlUtil
{
public:
  static std::string WstringToString(std::wstring orig);
  static std::wstring StringToWstring(std::string orig);

  static std::string GetProfileXml(const PWLAN_AVAILABLE_NETWORK pNet);
  static std::string GetCredentialsXml(const std::string& username, const std::string& password);

  struct WLANParam {
    std::string name;
    std::string ssid;
    std::string connection_type;
    std::string authentication;
    std::string encryption;
  };
private:
  static bool GetWlanParam(const PWLAN_AVAILABLE_NETWORK pNet, WLANParam* param);
};

