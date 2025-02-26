#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLE2902.h>

static const char* rootCACertificate = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\n" \
  "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\n" \
  "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\n" \
  "MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\n" \
  "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
  "hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\n" \
  "RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\n" \
  "gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\n" \
  "KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\n" \
  "QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\n" \
  "XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\n" \
  "DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\n" \
  "LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\n" \
  "RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\n" \
  "jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\n" \
  "6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\n" \
  "mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\n" \
  "Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\n" \
  "WD9f\n" \
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
