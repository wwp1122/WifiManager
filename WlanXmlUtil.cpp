#include "WlanXmlUtil.h"

#pragma comment(lib, "wlanapi.lib")
std::string WlanXmlUtil::WstringToString(std::wstring orig) {
  char output[2048];
  sprintf_s(output, "%ws", orig.c_str());
  return output;
}

std::wstring WlanXmlUtil::StringToWstring(std::string orig) {
  wchar_t ws[8192];
  swprintf(ws, 8192, L"%hs", orig.c_str());
  return ws;
}

std::string WlanXmlUtil::GetProfileXml(const PWLAN_AVAILABLE_NETWORK pNet) {
  WLANParam param;
  if (!GetWlanParam(pNet, &param)) {
    return "";
  }

  return R"(
<WLANProfile
    xmlns="http://www.microsoft.com/networking/WLAN/profile/v1">
    <name>)" + param.ssid + R"(</name>
    <SSIDConfig>
        <SSID>
            <name>)" + param.ssid +R"(</name>
        </SSID>
    </SSIDConfig>
    <connectionType>)" + param.connection_type + R"(</connectionType>
    <connectionMode>auto</connectionMode>
    <MSM>
        <security>
            <authEncryption>
                <authentication>)" + param.authentication + R"(</authentication>
                <encryption>)" + param.encryption + R"(</encryption>
                <useOneX>true</useOneX>
            </authEncryption>
            <PMKCacheMode>enabled</PMKCacheMode>
            <PMKCacheTTL>720</PMKCacheTTL>
            <PMKCacheSize>128</PMKCacheSize>
            <preAuthMode>disabled</preAuthMode>
            <OneX
                xmlns="http://www.microsoft.com/networking/OneX/v1">
                <authMode>user</authMode>
                <EAPConfig>
                    <EapHostConfig
                        xmlns="http://www.microsoft.com/provisioning/EapHostConfig">
                        <EapMethod>
                            <Type
                                xmlns="http://www.microsoft.com/provisioning/EapCommon">25
                            </Type>
                            <VendorId
                                xmlns="http://www.microsoft.com/provisioning/EapCommon">0
                            </VendorId>
                            <VendorType
                                xmlns="http://www.microsoft.com/provisioning/EapCommon">0
                            </VendorType>
                            <AuthorId
                                xmlns="http://www.microsoft.com/provisioning/EapCommon">0
                            </AuthorId>
                        </EapMethod>
                        <Config
                            xmlns="http://www.microsoft.com/provisioning/EapHostConfig">
                            <Eap
                                xmlns="http://www.microsoft.com/provisioning/BaseEapConnectionPropertiesV1">
                                <Type>25</Type>
                                <EapType
                                    xmlns="http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV1">
                                    <ServerValidation>
                                        <DisableUserPromptForServerValidation>false</DisableUserPromptForServerValidation>
                                        <ServerNames></ServerNames>
                                        <TrustedRootCA>74 84 7a 59 ee 86 49 de b8 50 23 fc 90 15 01 3f ab 12 f7 13 </TrustedRootCA>
                                    </ServerValidation>
                                    <FastReconnect>true</FastReconnect>
                                    <InnerEapOptional>false</InnerEapOptional>
                                    <Eap
                                        xmlns="http://www.microsoft.com/provisioning/BaseEapConnectionPropertiesV1">
                                        <Type>26</Type>
                                        <EapType
                                            xmlns="http://www.microsoft.com/provisioning/MsChapV2ConnectionPropertiesV1">
                                            <UseWinLogonCredentials>false</UseWinLogonCredentials>
                                        </EapType>
                                    </Eap>
                                    <EnableQuarantineChecks>false</EnableQuarantineChecks>
                                    <RequireCryptoBinding>false</RequireCryptoBinding>
                                    <PeapExtensions>
                                        <PerformServerValidation
                                            xmlns="http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV2">true
                                        </PerformServerValidation>
                                        <AcceptServerName
                                            xmlns="http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV2">true
                                        </AcceptServerName>
                                        <PeapExtensionsV2
                                            xmlns="http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV2">
                                            <AllowPromptingWhenServerCANotFound
                                                xmlns="http://www.microsoft.com/provisioning/MsPeapConnectionPropertiesV3">true
                                            </AllowPromptingWhenServerCANotFound>
                                        </PeapExtensionsV2>
                                    </PeapExtensions>
                                </EapType>
                            </Eap>
                        </Config>
                    </EapHostConfig>
                </EAPConfig>
            </OneX>
        </security>
    </MSM>
    <MacRandomization
        xmlns="http://www.microsoft.com/networking/WLAN/profile/v3">
        <enableRandomization>false</enableRandomization>
        <randomizationSeed>991743173</randomizationSeed>
    </MacRandomization>
</WLANProfile>
)";
}

bool WlanXmlUtil::GetWlanParam(const PWLAN_AVAILABLE_NETWORK pNet, WLANParam* param) {
  param->name = WstringToString(pNet->strProfileName);
  param->ssid = (const char*)pNet->dot11Ssid.ucSSID;
  switch (pNet->dot11BssType) {
  case dot11_BSS_type_infrastructure:
    param->connection_type = "ESS";
    break;
  case dot11_BSS_type_independent:
    param->connection_type = "IBSS";
    break;
  case dot11_BSS_type_any:
    param->connection_type = "Any";
    break;
  default:
    printf("Unknown BSS type\n");
    return false;
  }

  switch (pNet->dot11DefaultAuthAlgorithm) {
  case DOT11_AUTH_ALGO_80211_OPEN:
    param->authentication = "open";
    break;
  case DOT11_AUTH_ALGO_80211_SHARED_KEY:
    param->authentication = "shared";
    break;
  case DOT11_AUTH_ALGO_WPA:
    param->authentication = "WPA";
    break;
  case DOT11_AUTH_ALGO_WPA_PSK:
    param->authentication = "WPAPSK";
    break;
  case DOT11_AUTH_ALGO_WPA_NONE:
    param->authentication = "none";
    break;
  case DOT11_AUTH_ALGO_RSNA:
    param->authentication = "WPA2";
    break;
  case DOT11_AUTH_ALGO_RSNA_PSK:
    param->authentication = "WPA2PSK";
    break;
  default:
    return false;
  }

  switch (pNet->dot11DefaultCipherAlgorithm) {
  case DOT11_CIPHER_ALGO_NONE:
    param->encryption = "none";
    break;
  case DOT11_CIPHER_ALGO_WEP40:
    param->encryption = "WEP";
    break;
  case DOT11_CIPHER_ALGO_TKIP:
    param->encryption = "TKIP";
    break;
  case DOT11_CIPHER_ALGO_CCMP:
    param->encryption = "AES";
    break;
  case DOT11_CIPHER_ALGO_WEP104:
    param->encryption = "WEP";
    break;
  case DOT11_CIPHER_ALGO_WEP:
    param->encryption = "WEP";
    break;
  case DOT11_CIPHER_ALGO_WPA_USE_GROUP:
    wprintf(L"USE-GROUP not exist in MSDN");
  default:
    return false;
  }

  return true;
}

std::string WlanXmlUtil::GetCredentialsXml(const std::string& username, const std::string& password) {
  return R"(
<EapHostUserCredentials
    xmlns="http://www.microsoft.com/provisioning/EapHostUserCredentials"
    xmlns:eapCommon="http://www.microsoft.com/provisioning/EapCommon"
    xmlns:baseEap="http://www.microsoft.com/provisioning/BaseEapMethodUserCredentials">
    <EapMethod>
        <eapCommon:Type>25</eapCommon:Type>
        <eapCommon:AuthorId>0</eapCommon:AuthorId>
    </EapMethod>
    <Credentials
        xmlns:eapUser="http://www.microsoft.com/provisioning/EapUserPropertiesV1"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
        xmlns:baseEap="http://www.microsoft.com/provisioning/BaseEapUserPropertiesV1"
        xmlns:MsPeap="http://www.microsoft.com/provisioning/MsPeapUserPropertiesV1"
        xmlns:MsChapV2="http://www.microsoft.com/provisioning/MsChapV2UserPropertiesV1">
        <baseEap:Eap>
            <baseEap:Type>25</baseEap:Type>
            <MsPeap:EapType>
                <MsPeap:RoutingIdentity>test1</MsPeap:RoutingIdentity>
                <baseEap:Eap>
                    <baseEap:Type>26</baseEap:Type>
                    <MsChapV2:EapType>
                        <MsChapV2:Username>)" + username + R"(</MsChapV2:Username>
                        <MsChapV2:Password>)" + password + R"(</MsChapV2:Password>
                    </MsChapV2:EapType>
                </baseEap:Eap>
            </MsPeap:EapType>
        </baseEap:Eap>
    </Credentials>
</EapHostUserCredentials>
)";
}