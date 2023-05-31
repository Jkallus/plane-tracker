
// Example sketch which shows how to display some patterns
// on a 64x32 LED matrix
//

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Fonts/Picopixel.h>
#include <ArduinoJson.h>

const char* ssid = "Josh's Wi-Fi Network";
const char* password = "FFFFFFFFFF";
const char* mqtt_server = "192.168.8.198";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[1024];

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another
 
//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("FlightTrackerClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("flight_data_json");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {

  // Module configuration
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(1024);

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255
  dma_display->clearScreen();
  dma_display->fillScreen(myWHITE);
  
  // fix the screen with green
  dma_display->fillRect(0, 0, dma_display->width(), dma_display->height(), dma_display->color444(0, 15, 0));
  delay(500);

  // fill the screen with 'black'
  dma_display->fillScreen(dma_display->color444(0, 0, 0));

  dma_display->setTextWrap(false);
  dma_display->setFont(&Picopixel);
  dma_display->setTextSize(0);
}

void callback(char* topic, byte* message, unsigned int length)
{
  Serial.print("Message of length " + String(length) + " arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, message);

  if(error){
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  Serial.println("Data");
  Serial.println("----------------------" + String(millis()) + "-------------------");

  dma_display->clearScreen();

  int y = 4;
  dma_display->setCursor(0, y);

  for(JsonObject item: doc.as<JsonArray>()){
    String Aircraft = item["Aircraft"].as<String>();
    int speed = item["Speed"];
    double Distance = item["Distance"];
    String DistanceStr = String(Distance, 1);
    String FlightNumber = item["FlightNumber"].as<String>();
    String Direction = item["Direction"].as<String>();
    Serial.println(Aircraft + ", " + String(speed) + ", " + DistanceStr + ", " + FlightNumber + ", " + Direction);

    if(Direction == "In"){
      dma_display->setTextColor(dma_display->color565(255, 0, 0));
    }
    else{
      dma_display->setTextColor(dma_display->color565(0, 255, 0));
    }

    dma_display->print(FlightNumber);
    dma_display->print(" ");
    dma_display->print(Aircraft);
    dma_display->print(" ");
    dma_display->print(DistanceStr);
    y += 6;
    dma_display->setCursor(0, y);
  }

  Serial.println();
}

uint8_t wheelval = 0;
int i = 0;
void loop() {

    if (!client.connected()) 
    {
      reconnect();
    }
    client.loop();
}