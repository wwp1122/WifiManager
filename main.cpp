#include "WlanManager.h"


int main(int argc, char* argv[]) {
  if (argc == 4) {
    WlanManager ma;
    ma.Connect(argv[1], argv[2], argv[3]);
  }
  else {
    std::string useage = R"(useage:
      WifiManager.exe <Wlan name> <username> <password>)";
    printf("%s\n", useage.c_str());
  }
  getchar();
  return 0;
}