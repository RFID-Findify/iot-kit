/** Beispiel Senden von Sensordaten an ThingSpeak
 */
#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include "MFRC522.h"
#include "OLEDDisplay.h"
#include "http_request.h"


using namespace std;
// UI
OLEDDisplay oled(MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA,
                 MBED_CONF_IOTKIT_OLED_SCL);

// NFC/RFID Reader (SPI)
MFRC522 rfidReader(MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO,
                   MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS,
                   MBED_CONF_IOTKIT_RFID_RST);


extern "C" void mbed_mac_address(char *mac);


// I/O Buffer
char UUIDBytes[64];
char body[1024];


int main() {

    printf("\n");
  // Initialize the display
    oled.clear();
    //set oled cursor to 0,0
    oled.cursor(0, 0);
    //print oled starting message
    oled.printf("RFID Reader - Findify\n");
    printf("RFID Reader - Findify\n");
  uint8_t id;
  float value1, value2;
  /* Init all sensors with default params */
  rfidReader.PCD_Init();
  // Connect to the network with the default networking interface
  // if you use WiFi: see mbed_app.json for the credentials
  WiFiInterface *network = WiFiInterface::get_default_instance();
  if (!network) {
    printf("ERROR: No WiFiInterface found.\n");
    oled.printf("ERROR: No WiFiInterface found.\n");
    return -1;
  }
  // Connect to the network
  printf("Connecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
  oled.printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
  int ret =
      network->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD,
                       NSAPI_SECURITY_WPA_WPA2);

  // Wait for connection
  if (ret != 0) {
    // Connection error
    oled.printf("\nConnection error: %d\n", ret);
    oled.printf("press the black button to reset\n");

    printf("\nConnection error: %d\n", ret);
    printf("press the black button to reset\n");
    return -1;
  }

  oled.clear();
  oled.cursor(0, 0);

  printf("Success\n");
  printf("Mac-address: %s\n", network->get_mac_address());

  oled.printf("Success\n");
  oled.printf("Mac-address: %s\n", network->get_mac_address());
  SocketAddress a;
  network->get_ip_address(&a);

  //print IP address
  oled.printf("IP: %s\n", a.get_ip_address());
    //print IP address
  printf("IP: %s\n", a.get_ip_address());
  while (1) {
    if (rfidReader.PICC_IsNewCardPresent())
      if (rfidReader.PICC_ReadCardSerial()) {

        // read UID (Serial number) from RFID chip
         for ( int i = 0; i < rfidReader.uid.size; i++ ){
                    char buff[5];
                    // convert UID to string with Hex format
                    sprintf(buff, "%02X:", rfidReader.uid.uidByte[i]);
                    //add UID to UUIDBytes
                    strcat(UUIDBytes, buff);
                }

        //remove last char (:)
        UUIDBytes[strlen(UUIDBytes) -1] = '\0';

        if (!rfidReader.PICC_IsNewCardPresent()) {
        }

        // Select one of the cards
        if (!rfidReader.PICC_ReadCardSerial()) {
        }
        //clear oled display
        oled.clear();
        //put cursor at 0,0
        oled.cursor(0, 0);
        //create mac-Adress
        char mac[6];
        mbed_mac_address(mac);

        //char array for the full mac-Adress
        char finalmac[64];
        //copy mac-Adress to finalmac
        sprintf(finalmac, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); 

        //create post_request, with
        HttpRequest* post_req = new HttpRequest( network, HTTP_POST, "http://lenserver.one:500/api/v1/iot-handler");
        //set header to json
        post_req->set_header("Content-Type", "application/json");
        //create json string
        sprintf( body, "{ \"serial_number\": \"%s\", \"mac_address\": \"%s\" }", UUIDBytes, finalmac);
        //print body to console
        printf("body: %s \n", body);
        //print on oled, that he is sending data
        printf("Request: %s\n", UUIDBytes);
        oled.printf("Request: %s\n", UUIDBytes);
        //get response by sending the request.
        HttpResponse* post_res = post_req->send(body, strlen(body));
        //print response to oled display
        printf("\n");
        oled.clear();
        
        // put cursor at 0,0
        oled.cursor(0, 0);
        printf("Code: %i\n", post_res->get_status_code());
        oled.printf("Code: %i\n", post_res->get_status_code());
        printf("Response: %s\n", post_res->get_body());
        oled.printf("Response: %s", post_res->get_body());

        //delete request and response
        memset(body, 0, strlen(body));
        memset(UUIDBytes, 0, sizeof UUIDBytes);
      }
      //wait for 100 ms
    thread_sleep_for(100);
  }
}