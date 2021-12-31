#include "UbidotsESPMQTT.h"
#include "DHT.h"  
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
/*--------------------------*/
#define token "" 
#define ssid "" 
#define password ""  
#define device "dht11"
#define MQTTCLIENTNAME  "sdfcvdhjnbvxt2"
Ubidots client(token, MQTTCLIENTNAME);
/*--------------------------*/
#define DHTTYPE DHT11   
#define dht_dpin 0
DHT dht(dht_dpin, DHTTYPE); 
/*--------------------------*/
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 
#define OLED_RESET LED_BUILTIN 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/*--------------------------*/
int heater = 2;
int fan = 14;
int setpoint;
int deadband = 3;
bool hMessage = false;
bool fMessage = false;
bool fButtonON = false;
bool hButtonON = false;


void callback(char* topic, byte* payload, unsigned int length) 
{
  char str[2];
  
  for (int i=0; i<length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  str[0]= payload[0];
  str[1]= payload[1];
  str[2] = '\0';
 
  if (topic[20] == 's')
  {
    setpoint = atoi(str);
    memset(payload, 0, sizeof payload);
  }
    
  if (topic[20] == 'h')
  {
    if (str[0] == '1')
    {
      digitalWrite(heater, HIGH);
      digitalWrite(fan, LOW);
      display.clearDisplay();
      display.setTextSize(2);  
      display.setCursor(6,7);
      display.print("Heating On");
      display.display();
      delay(1500);
      while(str[0] != '0')
      {
      }
      hButtonON = true;
    }
    else if (str[0] == '0')
    {
      Serial.println("Heating button depressed");
      digitalWrite(heater, LOW);
      hButtonON = false;
    }
  }
  else if(topic[20] == 'f')
  {
    if (str[0] == '1')
    {
      digitalWrite(fan, HIGH);
      digitalWrite(heater, LOW);
      display.clearDisplay();
      display.setTextSize(2);  
      display.setCursor(20,7);
      display.print("Fan On");
      display.display();
      delay(1500);
      while(str[0] != '0')
      {
      }
      fButtonON = true;
    }
    else if (str[0] == '0')
    {
      digitalWrite(fan, LOW);
      fButtonON = false;
    }
  }
} 

/****************************************
 * Main Functions
 ****************************************/

void setup() 
{
  pinMode(heater, OUTPUT);
  pinMode(fan, OUTPUT);
  client.setDebug(false);
  Serial.begin(115200);
  client.wifiConnection(ssid, password);
  client.begin(callback);
  client.ubidotsSubscribe(device, "setpoint"); 
  client.ubidotsSubscribe(device, "heating");
  client.ubidotsSubscribe(device, "fan");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
}

void loop() 
{
	int humid = getHomeHumidity();
	int temp = getHomeTemp();
	display.clearDisplay();
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.setTextSize(1);
	display.println("Temperature");
	display.setCursor(80,0);
	display.println("Humidity");
	display.setCursor(54,9);
	display.println("SP");  
	display.setTextSize(2);
	display.setCursor(90,8);
	display.print(humid);
	display.setTextSize(1);   
	display.print("%");
	display.setTextSize(2);  
	display.setCursor(0,8);
	display.print(temp);
	display.setTextSize(1); 
	display.print((char)247); 
	display.println("C");
	display.setTextSize(2);  
	display.setCursor(36,17);
	display.print("[");
	display.print(setpoint);
	display.print("]");  
	display.display();
	delay(1000);
	display.clearDisplay();

	if (temp < (setpoint - deadband))
	{
	  fMessage = false;
	  digitalWrite(heater, HIGH);
	  digitalWrite(fan, LOW);
	  if (hMessage != true)
	  {
		display.clearDisplay();
		display.setTextSize(2);  
		display.setCursor(6,7);
		display.print("Heating On");
		display.display();
		delay(1500);
		hMessage = true;
	  }
	}

	else if (temp > (setpoint + deadband))
	{
	  hMessage= false;
	  digitalWrite(heater, LOW);
	  digitalWrite(fan, HIGH);
	  if (fMessage != true)
	  {
		display.clearDisplay();
		display.setTextSize(2);  
		display.setCursor(20,7);
		display.print("Fan On");
		display.display();
		delay(1500);
		fMessage = true;
	  }
	}
	else
	{
	  digitalWrite(heater, LOW);
	  digitalWrite(fan, LOW);
	  hMessage = fMessage = false;
	}

	if(!client.connected())
	{
	  client.reconnect();
	  client.ubidotsSubscribe(device, "setpoint");
	  client.ubidotsSubscribe(device, "heating");
	  client.ubidotsSubscribe(device, "fan");
	}
	client.add("Temperature", temp);
	client.add("Humidity", humid);

	client.ubidotsPublish(device);
	client.loop();
}

float getHomeTemp()
{
  int temp = dht.readTemperature();
  return temp;
}

float getHomeHumidity()
{
  int humid = dht.readHumidity();
  return humid;
}
