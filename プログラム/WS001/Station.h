#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLE2902.h>

static const char* rootCACertificate = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
  "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
  "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
  "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
  "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
  "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
  "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
  "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
  "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
  "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
  "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
  "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
  "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
  "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
  "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
  "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
  "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
  "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
  "-----END CERTIFICATE-----\n";

#define CONFIG_FILE "/config.json"

static bool dark_mode = false;
static uint16_t front_color;
static uint16_t back_color;

static char ssid[100];
static char password[100];

static char area_code[100];
static char area_code_sub[100];
static char spot_code[100];
static char area_code_week[100];

static String payload;

#define NO_DATA -999
struct WeatherInfo{
  uint16_t code; // today
  int max_temp; // today
  int min_temp; // today
  int pops[4]; // 00,06,12,18
  String codes_date[7];  
  uint16_t codes[7];
};
static WeatherInfo weather_info;

class Station{
  public:
    Station();
    void begin();
    bool loadConfig();
    bool saveConfig();
    void getConfig();
    void setColorMode();
    bool tryWLAN();
    bool tryNTP();
    bool getWeather();
    void disconnectWLAN();
    bool parseWeather();
    void dispWeather();
    void dispError(String text);
    void sleep(bool enableNTP);
  private:
    void dispIcon(int x, int y, uint16_t code);
    void dispText(int x, int y, uint16_t width, uint16_t height, String text);
    String cnvNum(int num);
};

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define INPUT_MODE_TITLE    "Mode:Input Config"

static bool is_disconnect_ble,is_recieved_ble;
static uint16_t rcv_count;

class ServerCallbacks : public BLEServerCallbacks {
  public:
    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);
};

class HandlerBLE : public BLECharacteristicCallbacks {
  public:
    void onWrite(BLECharacteristic *pCharacteristic);
  private:
    void dispText(String text);
};

class ServerBLE {
  public:
    ServerBLE();
    void begin();
};
