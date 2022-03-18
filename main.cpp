/** Beispiel Senden von Sensordaten an ThingSpeak
 */
#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#if MBED_CONF_IOTKIT_HTS221_SENSOR == true
#include "HTS221Sensor.h"
#endif
#if MBED_CONF_IOTKIT_BMP180_SENSOR == true
#include "BMP180Wrapper.h"
#endif
#include "MFRC522.h"
#include "OLEDDisplay.h"
#include "http_request.h"

using std::cout;

using namespace std;
// UI
OLEDDisplay oled(MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA,
                 MBED_CONF_IOTKIT_OLED_SCL);
static DevI2C devI2c(MBED_CONF_IOTKIT_I2C_SDA, MBED_CONF_IOTKIT_I2C_SCL);
#if MBED_CONF_IOTKIT_HTS221_SENSOR == true
static HTS221Sensor hum_temp(&devI2c);
#endif
#if MBED_CONF_IOTKIT_BMP180_SENSOR == true
static BMP180Wrapper hum_temp(&devI2c);
#endif
// NFC/RFID Reader (SPI)
MFRC522 rfidReader(MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO,
                   MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS,
                   MBED_CONF_IOTKIT_RFID_RST);


extern "C" void mbed_mac_address(char *mac);

/** ThingSpeak URL und API Key ggf. anpassen */
char host[] = "http://api.thingspeak.com/update";
char key[] = "A2ABBMDJYRAMA6JM";
// I/O Buffer
char UUIDBytes[64];
char body[1024];
// DigitalOut myled(MBED_CONF_IOTKIT_LED1);
int main() {
  uint8_t id;
  float value1, value2;
  printf("\tThingSpeak\n");
  /* Init all sensors with default params */
  hum_temp.init(NULL);
  hum_temp.enable();
  hum_temp.read_id(&id);
  printf("HTS221  humidity & temperature    = 0x%X\r\n", id);
  printf("RFID Reader MFRC522 Test V3\n");
  rfidReader.PCD_Init();
  // Connect to the network with the default networking interface
  // if you use WiFi: see mbed_app.json for the credentials
  WiFiInterface *network = WiFiInterface::get_default_instance();
  if (!network) {
    printf("ERROR: No WiFiInterface found.\n");
    return -1;
  }
  printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
  int ret =
      network->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD,
                       NSAPI_SECURITY_WPA_WPA2);
  if (ret != 0) {
    printf("\nConnection error: %d\n", ret);
    return -1;
  }
  printf("Success\n\n");
  printf("MAC: %s\n", network->get_mac_address());
  SocketAddress a;
  network->get_ip_address(&a);
  printf("IP: %s\n", a.get_ip_address());
  while (1) {
    if (rfidReader.PICC_IsNewCardPresent())
      if (rfidReader.PICC_ReadCardSerial()) {
          sprintf(UUIDBytes, "%02X:%02X:%02X:%02X", rfidReader.uid.uidByte[0],  rfidReader.uid.uidByte[1],  rfidReader.uid.uidByte[2],  rfidReader.uid.uidByte[3]);
    
        //std::cout << std::endl;
        std::cout << UUIDBytes << std::endl;


        if (!rfidReader.PICC_IsNewCardPresent()) {
        }

        // Select one of the cards
        if (!rfidReader.PICC_ReadCardSerial()) {
        }
        char mac[6];
        mbed_mac_address(mac);
        char finalmac[64];
        sprintf(finalmac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); 
        HttpRequest* post_req = new HttpRequest( network, HTTP_POST, "http://findify.lopo.dev:8000/");//todo, send thing to backend
        post_req->set_header("Content-Type", "application/json");
        sprintf( body, "{ \"UUID\": \"%s\", \"mac-address\": \"%s\" }", UUIDBytes, finalmac);
        cout << "body:" << body <<endl;
        HttpResponse* post_res = post_req->send(body, strlen(body));
        cout << post_res->get_status_code() << endl;
        memset(body, 0, strlen(body));

      }
    // hum_temp.get_temperature(&value1);
    // hum_temp.get_humidity(&value2);
    // sprintf( UUIDBytes, "%s?key=%s&field1=%f&field2=%f", host, key, value1,
    // value2 ); printf( "%s\n", UUIDBytes ); oled.cursor( 1, 0 ); oled.printf(
    // "temp: %3.2f\nhum : %3.2f", value1, value2 );
    ////myled = 1;
    // HttpRequest* get_req = new HttpRequest( network, HTTP_POST, UUIDBytes );
    // HttpResponse* get_res = get_req->send();
    // if (!get_res)
    //{
    //    printf("HttpRequest failed (error code %d)\n", get_req->get_error());
    //    return 1;
    //}
    // delete get_req;
    ////myled = 0;
    // thread_sleep_for( 10000 );
    thread_sleep_for(200);
  }
}