#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <RTClib.h>
#include <AHTxx.h>

// Piny dla DS18B20 i wyjścia PWM
#define ONE_WIRE_BUS 2
#define RedLight 5
#define BlueLight 6
#define TFT_PIN 3
#define TFT_DIM 7

volatile int pwmValue = 0;

// Inicjalizacja modułów DS18B20, ST7735 i DS3231
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_ST7735 tft = Adafruit_ST7735(10, 9, 8);
RTC_DS3231 rtc;

float ahtValue;                               //to store T/RH result
int ekran = 0;

AHTxx aht10(AHTXX_ADDRESS_X38, AHT1x_SENSOR); //sensor address, sensor type
int Time = 30;
const unsigned long zmianaR = map(Time,0,255,0,Time*60*1000);
const unsigned long zmianaB = map(Time,0,200,0,Time*60*1000);
unsigned long aktualnyCzasR = 0;
unsigned long aktualnyCzasB = 0;
unsigned long zapamietanyCzasR = 0;
unsigned long zapamietanyCzasB = 0;
int wakeHour = 15;
int wakeMin = 5;

int sleepHour = 15;
int sleepMin = 45;
int fadeR = 1;
int fadeB = 1;
int swiatloR = 0;
int swiatloB = 0;

void setup() {

Serial.begin(9600);
  // Inicjalizacja wyjść PWM
  pinMode(RedLight, OUTPUT);
  pinMode(BlueLight, OUTPUT);
  pinMode(TFT_PIN, OUTPUT);
  pinMode(TFT_DIM, INPUT);

  // Inicjalizacja modułów DS18B20, ST7735 i DS3231
  sensors.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
  initPartialDisplay();
  rtc.begin();
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  analogWrite(RedLight, 0);
  analogWrite(BlueLight, 0);
  aht10.begin();

}


void loop() {
  
// odświeżenie ekranu
  updateTime();
  updateTemp();
  updateAHT ();
  updateLight ();
  SunMoon();

if(digitalRead(TFT_DIM) == HIGH){
  analogWrite(TFT_PIN, 255);}
else{analogWrite(TFT_PIN, 0);}

DateTime now = rtc.now();

if (now.hour() == wakeHour && now.minute() >= wakeMin && now.minute() <= wakeMin + Time)
{sunrise();}
else if(now.hour() >= wakeHour && now.hour() <= sleepHour)
{swiatloR=243;
swiatloB=222;
  analogWrite(RedLight,swiatloR);
  analogWrite(BlueLight,swiatloB);}
 else if(now.hour() == sleepHour && now.minute() >= sleepMin && now.minute() <= sleepMin + Time) 
 {sunset();}
 else {swiatloR = 0;
 swiatloB=0;
  analogWrite (RedLight,swiatloR);
  analogWrite(BlueLight,swiatloB);}

Serial.println(swiatloR);
Serial.println(zmianaR);
Serial.println(swiatloB);
Serial.println(zmianaB);
Serial.println("");
}

// inicjalizacja trybu "Partial Display"
void initPartialDisplay() {
  
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
  tft.setAddrWindow(0, 0, 127, 31);
  //tft.writeCommand(ST7735_RAMWR);
}

  //wyświetlanie aktualnej godziny
void updateTime() {

  DateTime now = rtc.now();
  static int lastHour = -1;
  static int lastMinute = -1;
  static int lastSecond = -1;

  if (now.hour() != lastHour) {
    lastHour = now.hour();
    tft.setCursor(20, 10);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(2);
   if (lastHour < 10) tft.print('0');
    tft.print(lastHour, DEC);
    tft.print(':');
  }

  if (now.minute() != lastMinute) {
    lastMinute = now.minute();
    tft.setCursor(55, 10);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(2);
    if (lastMinute < 10) tft.print('0');
    tft.print(lastMinute, DEC);
    tft.print(':');
  }

  if (now.second() != lastSecond) {
    lastSecond = now.second();
    tft.setCursor(90, 10);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(2);
    if (lastSecond < 10) tft.print('0');
    tft.print(lastSecond, DEC);
  }}

//wyświetlanie temperatury
  void updateTemp() {

sensors.requestTemperatures();
  float temp1 = sensors.getTempCByIndex(0);
  float temp2 = sensors.getTempCByIndex(1);
  float tempRTC = rtc.getTemperature();
  static float lastTemp1 = -999;
static float lastTemp2 = -999;
static float lastRTC = -999;

if (temp1 != lastTemp1) { // sprawdź czy temperatura się zmieniła
    lastTemp1 = temp1;
    tft.setCursor(10, 30);
    tft.setTextColor(ST7735_BLUE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("Woda: ");
    tft.print(lastTemp1, 1); // wyświetl temperaturę z jednym miejscem po przecinku
    tft.print(" C");}

if (tempRTC != lastRTC) { // sprawdź czy temperatura się zmieniła
    lastRTC = tempRTC;
    tft.setCursor(10, 40);
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("RTC: ");
    tft.print(tempRTC, 1); // wyświetl temperaturę z jednym miejscem po przecinku
    tft.print(" C");}


if (temp2 != lastTemp2) { // sprawdź czy temperatura się zmieniła
    lastTemp2 = temp2;
    tft.setCursor(10, 50);
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("Temp: ");
    tft.print(lastTemp2, 1); // wyświetl temperaturę z jednym miejscem po przecinku
    tft.print(" C");}
  }

//wyświetlanie wilgotności
void updateAHT () {
 float ahtValue = aht10.readHumidity();
float lastAHT = -1;

if(ahtValue != lastAHT){
  lastAHT = ahtValue;
  tft.setCursor(10, 60);
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("rH: ");
    tft.print(ahtValue, 0); // wyświetl wilgotność 
    tft.print(" %");}
}

void updateLight (){
  float last_swiatloR = 0;
  float last_swiatloB = 0;

  if(swiatloR!=last_swiatloR){
last_swiatloR=swiatloR;
tft.setCursor(10, 80);
    tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("Red Light: ");
    tft.print(map(swiatloR, 0, 255, 0, 100));
    tft.print("%");}

if(swiatloB!=last_swiatloB){
last_swiatloB=swiatloB;
tft.setCursor(10, 90);
    tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    tft.setTextSize(1);
    tft.print("Blue Light:");
    tft.print(map(swiatloB, 0, 255, 0, 100));
    tft.print("%");}
}

void SunMoon (){
  //Sun
  int x = 110;
  int y = 40;
tft.fillCircle(x, y, 4, ST7735_YELLOW);
tft.drawLine(x-7, y, x+7, y, ST7735_YELLOW);
tft.drawLine(x, y-7, x, y+7, ST7735_YELLOW);
tft.drawLine(x-4, y+4, x+4, y-4, ST7735_YELLOW);
tft.drawLine(x-4, y-4, x+4, y+4, ST7735_YELLOW);
tft.setCursor(x+9, y-1);
    tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    tft.setTextSize(1);
    if(wakeHour<10){
      tft.print("0");
      tft.print(wakeHour);}
      else {tft.print(wakeHour);}
    tft.print(":");
if(wakeMin<10){
      tft.print("0");
      tft.print(wakeMin);}
      else {tft.print(wakeMin);};

//Moon
tft.fillCircle(x, y+14, 4, ST7735_YELLOW);
tft.fillCircle(x+4, y+14, 4, ST7735_BLACK);
tft.setCursor(x+9, y+13);
tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    tft.setTextSize(1);
    if(sleepHour<10){
      tft.print("0");
      tft.print(sleepHour);}
      else {tft.print(sleepHour);}
    tft.print(":");
if(sleepMin<10){
      tft.print("0");
      tft.print(sleepMin);}
      else {tft.print(sleepMin);}

}


void sunrise()
{
  if (swiatloR<255 && swiatloB<255){
aktualnyCzasR = millis();
aktualnyCzasB = millis();
if (aktualnyCzasR - zapamietanyCzasR >= zmianaR)
{zapamietanyCzasR= aktualnyCzasR;
analogWrite(RedLight, swiatloR);
swiatloR=swiatloR+fadeR;}


if (aktualnyCzasB - zapamietanyCzasB >= zmianaB)
{zapamietanyCzasB = aktualnyCzasB;
analogWrite(BlueLight, swiatloB);
swiatloB=swiatloB+fadeB;}

}
else {analogWrite(RedLight, 255);
analogWrite(BlueLight, 255);}
}

void sunset()
{
  if (swiatloR>0 && swiatloB>0){
aktualnyCzasR = millis();
if (aktualnyCzasR - zapamietanyCzasR >= zmianaR)
{zapamietanyCzasR = aktualnyCzasR;
analogWrite(RedLight, swiatloR);
swiatloR=swiatloR-fadeR;}  

if (aktualnyCzasB - zapamietanyCzasB >= zmianaB)
{zapamietanyCzasB = aktualnyCzasB;
analogWrite(BlueLight, swiatloB);
swiatloB=swiatloB-fadeB;}  

}
else {analogWrite(RedLight, 0);
analogWrite(BlueLight, 0);}
}