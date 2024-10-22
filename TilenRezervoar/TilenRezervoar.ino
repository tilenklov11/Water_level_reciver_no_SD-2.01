#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#define a_SSID "Wifi SSID"
#define a_PASSWORD "PASSWORD"

AsyncWebServer server(80);

// Pin definitions
#define SDA     21   // I2C data
#define SCL     22  // I2C clock
#define SS      18  // SPI slave select
#define RST     -1  // OLED reset
#define CONFIG_MOSI 27
#define CONFIG_MISO 19
#define CONFIG_CLK  5
#define CONFIG_NSS  18
#define CONFIG_RST  23
#define CONFIG_DIO0 26

// Images for water levels
#define WATER_0 ""
#define WATER_25 WATER_0
#define WATER_50 ""
#define WATER_75 WATER_50
#define WATER_100 ""

// Create SSD1306 object
SSD1306Wire display(0x3C, SDA, SCL);

char w_lvl;
String voltag = "0";

bool old_wifi_state = false;
String OldData;

String processor(const String& var){
  Serial.println (var);
	if(var == "TEMPLATE_PLACEHOLDERWATER_25"){
		if(w_lvl < 1){
			return "background-color:#bbb;";
		}
		return "";
	}
	if(var == "TEMPLATE_PLACEHOLDERWATER_50"){
		if(w_lvl < 2){
			return "background-color:#bbb;";
		}
		return "";
	}
	if(var == "TEMPLATE_PLACEHOLDERWATER_75"){
		if(w_lvl < 3){
			return "background-color:#bbb;";
		}
		return "";
	}
	if(var == "TEMPLATE_PLACEHOLDERWATER_100"){
		if(w_lvl < 4){
			return "background-color:#bbb;";
		}
		return "";
	}
	if(var == "TEMPLATE_PLACEHOLDERWATERLVL"){
		switch (w_lvl){
		  case 1:
			return "25%";
			break;
		  case 2:
			return "50%";
			break;
		  case 3:
			return "75%";
			break;
		  case 4:
			return "100%";
			break;
		  default:
			return "0%";
		}
	if(var == "TEMPLATE_PLACEHOLDERWATER_VOLTAGE"){
		return voltage;
	}

  return String();
}

String LoRa_parser(){
  String packet = "";
    
    // Read the packet into a string
    while (LoRa.available()) {
      packet += (char)LoRa.read();
    }

    Serial.println(packet);
    // Process the packet
    if (packet.length() > 0) {
      // Extract water level and voltage information
      String waterLevel = packet.substring(0,4);
      w_lvl = 0;
      for(int i = 0; i<4; i++){
        w_lvl += (int) waterLevel[i] - 48;
      }
      Serial.println(w_lvl,HEX);
      voltage = packet.substring(5);

    

    return "Water level: " + getWaterLevelString(w_lvl) + "\nVoltage: " + voltage;
    }
    else{
      return "";
    }
}

inline String getWaterLevelString(char level) {
  switch (level) {
    case 1:
      return "25%";
    case 2:
      return "50%";
    case 3:
      return "75%";
    case 4:
      return "100%";
    default:
      return "0%";
  }
  
}

void setup() {
	// Initialize serial communication
  Serial.begin(9600);
  
  // Initialize LoRa module
  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
  LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed. Check your connections!");
    while (1);
  }
   LoRa.setSpreadingFactor(12);
  // Initialize OLED display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Waiting for LoRa packets...");
  display.display();
  
  WiFi.begin(a_SSID, a_PASSWORD);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
  }
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();


} 

void loop() {
  // Check if a packet is available
  if (LoRa.parsePacket()) {
    Serial.println("Receaving packet...");
    String NewData;
    NewData = LoRa_parser();
    
    if (NewData != ""){
      // Display water level and voltage on the OLED display
      OldData = NewData;
      display.clear();
      display.drawString(0, 0, NewData);
      display.display();
    }
  }

  //Test for connection to the great all mighty wifi
  if ((WiFi.status() == WL_CONNECTED) != old_wifi_state){
    old_wifi_state = (WiFi.status() == WL_CONNECTED);
    Serial.println("Server:" + (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "Offline");
    display.drawString(0, 40, "Server:" + (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "Offline");
    display.display();
    //changeIP();
  }

}
  
