/** Beispiel Abfrage Cloud Dienst Sunrise / Sunset
 */
#include "mbed.h"
#include <string>
#include "OLEDDisplay.h"
#include "http_request.h"
#include "MbedJSONValue.h"
#include "MFRC522.h"

#include <MQTTClientMbedOs.h>
#include <MQTTNetwork.h>
#include <MQTTClient.h>
#include <MQTTmbed.h>

char* topicActors = (char*) "iotkit/#";


char* topicRFID = (char*) "iotkit/rfid";
// MQTT Message
MQTT::Message message;
// I/O Buffer
char buf[100];
char cls[3][10] = { "low", "middle", "high" };
int type = 0;
int port = 1883;
// MQTT Brocker
char* hostname = (char*) "broker.mqttdashboard.com";


OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );
MFRC522    rfidReader( MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST ); 


void publish( MQTTNetwork &mqttNetwork, MQTT::Client<MQTTNetwork, Countdown> &client, char* topic )
{
    printf("Doing something:::::::: \n");
    MQTT::Message message;    
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buf;
    message.payloadlen = strlen(buf)+1;
    client.publish( topic, message);  
    printf("MEssage sent::::::::::\n");
}

void messageArrived( MQTT::MessageData& md )
{
    float value;
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("Topic %.*s, ", md.topicName.lenstring.len, (char*) md.topicName.lenstring.data );            
}

WiFiInterface* wifi;

bool connectWifi() {
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        return false;
    }
    int status = wifi->connect("Unicorn-Island", "un1c0rnB4n4n4s", NSAPI_SECURITY_WPA_WPA2);
    if (status != 0) {
        return false;
    }
    return true;
}


int main()
{   
    if (!connectWifi()) {
        printf("Could not connect to WIFI, system shutting down");
        return -1;
    }
    printf("---------------------------------------------------\n");
    printf("---------------| Connected to Wifi |---------------\n");
    printf("---------------------------------------------------\n");
    
    MQTTNetwork mqttNetwork( wifi );
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    
    rfidReader.PCD_Init();


    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*) wifi->get_mac_address(); // muss Eindeutig sein, ansonsten ist nur 1ne Connection moeglich
    data.username.cstring = (char*) wifi->get_mac_address(); // User und Password ohne Funktion
    data.password.cstring = (char*) "password";

    client.subscribe( topicActors, MQTT::QOS0, messageArrived );
    printf("MQTT subscribe %s\n", topicActors );
    
    printf("----------------| PCD Initialized |----------------\n");
    printf("---------------------------------------------------\n");
    while   ( 1 ) 
    {
        oled.clear();
        oled.cursor(1, 0);
        oled.printf("Ready");
        // RFID Reader
        if ( rfidReader.PICC_IsNewCardPresent()) {
            if ( rfidReader.PICC_ReadCardSerial()) 
            {
                sprintf( buf, "%02X:%02X:%02X:%02X:", rfidReader.uid.uidByte[0], rfidReader.uid.uidByte[1], rfidReader.uid.uidByte[2], rfidReader.uid.uidByte[3] );
                publish( mqttNetwork, client, topicRFID );       

            }
        }
        thread_sleep_for( 200 );
    }
}
