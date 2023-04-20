#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Sounds.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32MX1508.h>

/* Free Ports Left side
25
32
*/

#define PINA 12
#define PINB 14
#define CH1 2                   // 16 Channels (0-15) are availible
#define CH2 1                  // Make sure each pin is a different channel and not in use by other PWM devices (servos, LED's, etc)

#define PWMSound 22
#define RelayHeatElement 26
#define NeopixelOut 27        // On Trinket or Gemma, suggest changing this to 1
#define ReadTemperature 33
#define ReadMainPower 34
#define ReadBatteryPower 35
#define NUMPIXELS 8

float desiredTemperature = 36.0;
float temperatureCelsius = -1;
int ScreenID[1] = {-1};

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ReadTemperature);
DallasTemperature temperatureSensor(&oneWire);

TFT_eSPI tft = TFT_eSPI();

Sounds UserSounds = Sounds(PWMSound);

Adafruit_NeoPixel pixels(NUMPIXELS, NeopixelOut, NEO_GRB + NEO_KHZ800);

MX1508 airPumpIn(PINA,PINB, CH1, CH2);  

void MatchUI(int background, int contrast, int screen)
{
  if (ScreenID[0] != screen)
  {
    tft.fillScreen(background);
    tft.textbgcolor = background;
    tft.textcolor = contrast;
    ScreenID[0] = {screen};
  }
}

void PixelsLight(int arr[3])
{
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.fill(pixels.Color(arr[0],arr[1],arr[2]));
  pixels.show(); // Send the updated pixel colors to the hardware.

}

void setup()
{
  Serial.begin(9600);

  pinMode(RelayHeatElement, OUTPUT);
  pixels.begin();
  int colors [3] = {255,250,205}; //lemonchiffon
  PixelsLight(colors); 
  tft.init();
  tft.setRotation(2);
  MatchUI(TFT_LIGHTGREY, TFT_BLACK, 1);
  tft.drawString("The Open Shower", 22, 100, 4);
  delay(200);
  UserSounds.start();
  delay(200);
  temperatureSensor.begin();
  delay(200);
  tft.setTextSize(3);
  pixels.clear();
  pixels.show();

}

void TemperatureMainPower(double temperatureCelsius)
{
  // Check if reading was successful
  if (temperatureCelsius != DEVICE_DISCONNECTED_C)
  {
    tft.drawString("Celsius", 60, 30);
    tft.drawString("Temp", 10, 80);
    tft.drawFloat(temperatureCelsius, 2, 85, 110);

    if (temperatureCelsius > desiredTemperature)
    {
      digitalWrite(RelayHeatElement, LOW); // Shutdown relay
      airPumpIn.motorStop(); // Stop water mixture 
      UserSounds.temperature();            // Alert the user that the water is ready
      int colors [3] = {255,255,0}; //yellow
      PixelsLight(colors);       
    }
    else
    {
      digitalWrite(RelayHeatElement, HIGH); // Start relay
      delay(500);
      airPumpIn.motorGo(250); // Start pump to mix water
      delay(700);
      int colors [3] = {173,255,47}; //green 
      PixelsLight(colors); 
    }
  }
  else
  {
    // Shutdown relay for security
    digitalWrite(RelayHeatElement, LOW);
    airPumpIn.motorStop(); // Stop water mixture 
    MatchUI(TFT_YELLOW, TFT_BLUE, 20);
    tft.drawString("ERT", 100, 100);
    int colors [3] = {255,0,0}; //red
    PixelsLight(colors); 

  }
}

void TemperatureBatteryPower(double temperatureCelsius)
{
  // Check if reading was successful
  if (temperatureCelsius != DEVICE_DISCONNECTED_C)
  {
    MatchUI(TFT_SKYBLUE, TFT_DARKGREEN, 3);
    tft.drawString("Temperature", 25, 80);
    tft.drawFloat(temperatureCelsius, 2, 85, 110);
    airPumpIn.motorGo(250); // Start pump to mix water
    int colors [3] = {30,144,255}; //blue
    PixelsLight(colors); 
  }
  else
  {
    MatchUI(TFT_SKYBLUE, TFT_DARKGREEN, 5);
    tft.drawString("ERT", 100, 100);
    airPumpIn.motorStop(); // Stop water mixture 
    int colors [3] = {255,0,0}; //red
    PixelsLight(colors); 
  }
}


void loop()
{

  temperatureSensor.requestTemperatures();
  float temperatureCelsius = temperatureSensor.getTempCByIndex(0);

  // If is connected to main power and temperature is below set value turn on relay

  if (analogRead(ReadMainPower) > 300)
  {
    MatchUI(TFT_YELLOW, TFT_BLUE, 2);
    tft.drawString("Target Temp", 10, 140);
    tft.drawFloat(desiredTemperature, 2, 80, 170);
    TemperatureMainPower(temperatureCelsius);
  }
  else if (analogRead(ReadBatteryPower) > 800)
  {
    tft.drawString("Battery", 60, 150);
    TemperatureBatteryPower(temperatureCelsius);
  }

  else
  {

    MatchUI(TFT_BLACK, TFT_LIGHTGREY, 4);
    tft.drawString("TEMP", 10, 80);
    tft.drawFloat(temperatureCelsius, 2, 80, 100);
    tft.drawString("EOR - 35 - 34", 10, 120);
    tft.drawFloat(analogRead(ReadBatteryPower), 2, 80, 140);
    tft.drawFloat(analogRead(ReadMainPower), 2, 80, 170);

    int colors[3] = {255,255,255};
    PixelsLight(colors);

    digitalWrite(RelayHeatElement, LOW); // Shutdown relay
    airPumpIn.motorStop(); //Stop water mixture

  }
}


// Task to be created.
void NeoPixelIndicator( void * pvParameters )
{
 for( ;; )
 {
     // Task code goes here.
 }
}

