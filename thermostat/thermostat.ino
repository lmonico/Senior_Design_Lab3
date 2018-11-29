/***************************************************
Arduino Thermostat
****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>
#include <OneWire.h>   //for thermo
#include <DallasTemperature.h> //for thermo
#include <EEPROM.h> //for mem

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF
#define ONE_WIRE_BUS 5 //pin for thermo
// mem addresses of set points in eeprom
#define eeAddress0 = 0
#define eeAddress1 = sizeof(setPoint)
#define eeAddress2 = 2*sizeof(setPoint)
#define eeAddress3 = 3*sizeof(setPoint)
#define eeAddress4 = 4*sizeof(setPoint)
#define eeAddress5 = 5*sizeof(setPoint)
#define eeAddress6 = 6*sizeof(setPoint)
#define eeAddress7 = 7*sizeof(setPoint)

/* touch zone sizes */
uint16_t arrowSize3[] = {15, 21}; //width, height of a size 3 arrow
uint16_t bitmapLogoSize[] = {30, 30};
uint16_t settingsButtonSize[] = {180, 40};
uint16_t setPointButtonSize[] = {220, 20};

/* touch zone origins (top left corner) */
uint16_t home_setLeftArrow[] = {60, 176}; uint16_t home_setRightArrow[] = {138, 176};
uint16_t home_modeLeftArrow[] = {49, 240}; uint16_t home_modeRightArrow[] = {153, 240};
uint16_t home_holdButton[] = {0,0};

uint16_t settings_setPoints[] = {30, 96};
uint16_t settings_editTime[] = {30, 160};

uint16_t setPoints_weekdays[] = {30, 96};
uint16_t setPoints_weekends[] = {30, 160};

uint16_t edit_HourArrows[][2] = {{120, 61}, {177, 61}};
uint16_t edit_MinArrows[][2] = {{120, 105}, {177, 105}};
uint16_t edit_HalfArrows[][2] = {{120, 149}, {177, 149}};
uint16_t edit_TempArrows[][2] = {{120, 193}, {177, 193}};
uint16_t edit_StatusArrows[][2] = {{120, 237}, {177, 237}};

//first index is which button, second index is which x/y coord
uint16_t setPointPos[][2] = {{8, 118}, {8, 158}, {8, 198}, {8, 238}};

uint16_t br_cornerButton[] = {206, 286};

/* global information variables */
uint16_t backgroundColor = WHITE;
String screenState = "";
int setTemp = 75;
int currentTemp = 0;
int oldTemp = 0;
String dateTime = "Wed Oct 31 10:46 AM";
String modes[] = {"A/C", "HEAT", "AUTO", "OFF"};
int currentModeIndex = 0;

//mem variables
struct setPoint {
 int hour;
 int minute;
 bool half;
 int temp;
 bool status;
};

setPoint setPoints[8];

//variabled for thermo
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// home button bitmap
const unsigned char homeButton [] PROGMEM = {
// 'dynnamitt_home, 30x30px
0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x07, 0x0f, 0xc0, 0x00, 0x0f, 0x1f, 0xe0, 0x00,
0x0f, 0x3c, 0xf0, 0x00, 0x0e, 0x78, 0x78, 0x00, 0x0c, 0xf3, 0x1c, 0x00, 0x01, 0xe7, 0x8e, 0x00,
0x03, 0x8f, 0xc7, 0x00, 0x07, 0x1f, 0xe3, 0x80, 0x0e, 0x3f, 0xf1, 0xc0, 0x1c, 0xff, 0xfc, 0xe0,
0x39, 0xff, 0xfe, 0x70, 0x73, 0xff, 0xff, 0x38, 0xe7, 0xff, 0xff, 0x9c, 0x47, 0xff, 0xff, 0xc8,
0x0f, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xc0,
0x0f, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xc0, 0x0f, 0xf0, 0x3f, 0xc0,
0x0f, 0xf0, 0x3f, 0xc0, 0x0f, 0xf0, 0x3f, 0xc0, 0x0f, 0xf0, 0x3f, 0xc0, 0x0f, 0xf0, 0x3f, 0xc0,
0x0f, 0xf0, 0x3f, 0xc0, 0x07, 0xf0, 0x3f, 0x80
};


// settings button
const unsigned char settingsButton [] PROGMEM = {
// 'Settings_black-512, 30x30px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x60, 0x00, 0x00, 0x78, 0x78, 0x00,
0x00, 0xfc, 0xfc, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xfc, 0x00,
0x0f, 0xff, 0xff, 0xc0, 0x1f, 0xfc, 0xff, 0xe0, 0x1f, 0xf0, 0x3f, 0xe0, 0x3f, 0xe0, 0x1f, 0xf0,
0x3f, 0xc0, 0x0f, 0xf0, 0x0f, 0xc0, 0x07, 0xe0, 0x07, 0x80, 0x07, 0x80, 0x07, 0x80, 0x07, 0x80,
0x0f, 0xc0, 0x07, 0xc0, 0x3f, 0xc0, 0x0f, 0xf0, 0x3f, 0xe0, 0x0f, 0xf0, 0x1f, 0xf0, 0x1f, 0xe0,
0x1f, 0xf8, 0x7f, 0xe0, 0x0f, 0xff, 0xff, 0xc0, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xfc, 0x00,
0x00, 0xff, 0xfc, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x00, 0x7c, 0x78, 0x00, 0x00, 0x18, 0x60, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/*
* Setup
*/
void setup(void) {
while (!Serial);     // used for leonardo debugging

Serial.begin(115200);

tft.begin();

if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
 Serial.println("Couldn't start FT6206 touchscreen controller");
 while (1);
}

int memAdd = 0;
for(int i = 0; i < 8; i++)
{
  EEPROM.get(memAdd, setPoints[i]);
  memAdd += sizeof(setPoint);
}
//if cooling
backgroundColor = WHITE;

tft.setRotation(2);
homePage();
}

/*
* Main Loop
*/
void loop() {
 oldTemp = currentTemp;
 sensors.requestTemperatures();
 currentTemp = (int)((sensors.getTempCByIndex(0) * 1.8) + 32);

// Retrieve a point
TS_Point p = ctp.getPoint();


if(screenState == "HOME"){

 if(oldTemp != currentTemp)
 {
   homePage();
 }

 //left set temp arrow
 if(isInTouchZone(home_setLeftArrow, arrowSize3[0], arrowSize3[1], p.x, p.y)){
   setTemp = setTemp - 1;
   homePage();
 }

 //right set temp arrow
 if(isInTouchZone(home_setRightArrow, arrowSize3[0], arrowSize3[1], p.x, p.y)){
   setTemp = setTemp + 1;
   homePage();
 }

 //left mode arrow
 if(isInTouchZone(home_modeLeftArrow, arrowSize3[0], arrowSize3[1], p.x, p.y)){

   if (currentModeIndex > 0){
     currentModeIndex -= 1;
   }
   else {
     currentModeIndex = 3;
   }
   homePage();
 }

 //right mode arrow
 if(isInTouchZone(home_modeRightArrow, arrowSize3[0], arrowSize3[1], p.x, p.y)){

   if (currentModeIndex < 3){
     currentModeIndex += 1;
   }
   else {
     currentModeIndex = 0;
   }
   homePage();
 }


 //settings button
 if(isInTouchZone(br_cornerButton, bitmapLogoSize[0], bitmapLogoSize[1], p.x, p.y)){

   settingsPage();
   return;
 }

}


if(screenState == "SETTINGS"){

 //Set points navigation button
 if(isInTouchZone(settings_setPoints, settingsButtonSize[0], settingsButtonSize[1], p.x, p.y)){

   setPointsChoicePage();
   return;
 }

 //edit date/time navigation button
 if(isInTouchZone(settings_editTime, settingsButtonSize[0], settingsButtonSize[1], p.x, p.y)){

   editDateTimePage();
   return;
 }

 //home button
 if(isInTouchZone(br_cornerButton, bitmapLogoSize[0], bitmapLogoSize[1], p.x, p.y)){

   homePage();
   return;
 }

}

if(screenState == "SET_POINTS_CHOICE"){

 //Weekday set points button
 if(isInTouchZone(setPoints_weekdays, settingsButtonSize[0], settingsButtonSize[1], p.x, p.y)){

   setPointsWeekdaysPage();
   return;
 }

 //Weekend set points button
 if(isInTouchZone(setPoints_weekends, settingsButtonSize[0], settingsButtonSize[1], p.x, p.y)){

   setPointsWeekendsPage();
   return;
 }

 //home button
 if(isInTouchZone(br_cornerButton, bitmapLogoSize[0], bitmapLogoSize[1], p.x, p.y)){

   homePage();
   return;
 }
}



if(screenState == "SETPOINTS_WEEKDAY" || screenState == "SETPOINTS_WEEKEND"){

 //Set point button position 1
 if(isInTouchZone(setPointPos[0], setPointButtonSize[0], setPointButtonSize[1], p.x, p.y)){

   if(screenState == "SETPOINTS_WEEKDAY"){
     editSetPoint(0);
   }
   else if(screenState == "SETPOINTS_WEEKEND"){
     editSetPoint(4);
   }
   return;
 }

  //Set point button position 2
 if(isInTouchZone(setPointPos[1], setPointButtonSize[0], setPointButtonSize[1], p.x, p.y)){

   if(screenState == "SETPOINTS_WEEKDAY"){
     editSetPoint(1);
   }
   else if(screenState == "SETPOINTS_WEEKEND"){
     editSetPoint(5);
   }
   return;
 }

  //Set point button position 3
 if(isInTouchZone(setPointPos[2], setPointButtonSize[0], setPointButtonSize[1], p.x, p.y)){

   if(screenState == "SETPOINTS_WEEKDAY"){
     editSetPoint(2);
   }
   else if(screenState == "SETPOINTS_WEEKEND"){
     editSetPoint(6);
   }
   return;
 }

  //Set point button position 4
 if(isInTouchZone(setPointPos[3], setPointButtonSize[0], setPointButtonSize[1], p.x, p.y)){

   if(screenState == "SETPOINTS_WEEKDAY"){
     editSetPoint(3);
   }
   else if(screenState == "SETPOINTS_WEEKEND"){
     editSetPoint(7);
   }
   return;
 }


 //home button
 if(isInTouchZone(br_cornerButton, bitmapLogoSize[0], bitmapLogoSize[1], p.x, p.y)){

   homePage();
   return;
 }
}

if(screenState == "EDIT_SETPOINT"){

}



if(screenState == "EDIT_DATETIME"){

 //home button
 if(isInTouchZone(br_cornerButton, bitmapLogoSize[0], bitmapLogoSize[1], p.x, p.y)){

   homePage();
   return;
 }
}


// Wait for a touch
//if (! ctp.touched()) {
 //return;
//}

//delay(250);

}


/*
* Home Page
*/
void homePage(){

screenState = "HOME";

header();
//display current temp
tft.setTextSize(11); tft.setCursor(55, 48); tft.print(currentTemp);
tft.setTextSize(5); tft.print((char)247);

//display set to block
tft.setTextSize(2); tft.setCursor(80, 152); tft.print("Set To");
tft.setTextSize(3); tft.setCursor(home_setLeftArrow[0], home_setLeftArrow[1]); tft.print("< "); tft.print(setTemp);
tft.setTextSize(2); tft.print((char)247);
tft.setTextSize(3); tft.setCursor(home_setRightArrow[0], home_setRightArrow[1]); tft.print(" >");

//display mode block
tft.setTextSize(2); tft.setCursor(92, 216); tft.print("Mode");
tft.setTextSize(3); tft.setCursor(home_modeLeftArrow[0], home_modeLeftArrow[1]); tft.print("< "); tft.print(modes[currentModeIndex]);
tft.setCursor(home_modeRightArrow[0], home_modeRightArrow[1]); tft.print(" >");

//display hold button
tft.drawRect(6, 286, 75, 29, BLACK);
tft.setTextSize(3); tft.setCursor(9, 291); tft.print("HOLD");

//display settings button
tft.drawBitmap(br_cornerButton[0], br_cornerButton[1], settingsButton, 30, 30, BLACK);

}

/*
* Settings Page
*/
void settingsPage(){

screenState = "SETTINGS";

header();

tft.setTextSize(4); tft.setCursor(26, 40); tft.print("Settings");

tft.drawRect(settings_setPoints[0], settings_setPoints[1], settingsButtonSize[0], settingsButtonSize[1], BLACK);
tft.setTextSize(2); tft.setCursor(61, 108); tft.print("Set Points");

tft.drawRect(settings_editTime[0], settings_editTime[1], settingsButtonSize[0], settingsButtonSize[1], BLACK);
tft.setTextSize(2); tft.setCursor(37, 172); tft.print("Edit Date/Time");

tft.drawBitmap(206, 286, homeButton, 30, 30, BLACK);
}

/*
* Set points choice navigation
*/
void setPointsChoicePage(){

screenState = "SET_POINTS_CHOICE";
header();

tft.setTextSize(3); tft.setCursor(31, 40); tft.print("Set Points");

tft.drawRect(setPoints_weekdays[0], setPoints_weekdays[1], settingsButtonSize[0], settingsButtonSize[1], BLACK);
tft.setTextSize(2); tft.setCursor(61, 108); tft.print("Weekdays");

tft.drawRect(setPoints_weekends[0], setPoints_weekends[1], settingsButtonSize[0], settingsButtonSize[1], BLACK);
tft.setTextSize(2); tft.setCursor(61, 172); tft.print("Weekends");

tft.drawBitmap(206, 286, homeButton, 30, 30, BLACK);
}

/*
* Set points
*/
void setPointsWeekdaysPage(){

 screenState = "SETPOINTS_WEEKDAY";
header();

tft.setTextSize(2); tft.setCursor(10, 40); tft.print("Weekday Set Points");
tft.setTextSize(2); tft.setCursor(10, 80); tft.print("Click one to edit");

//set point 1

tft.drawRect(8, 118, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 120); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 120); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 120); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 120); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 120); tft.print("ON");

//set point 2
tft.drawRect(8, 158, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 160); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 160); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 160); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 160); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 160); tft.print("ON");

//set point 3
tft.drawRect(8, 198, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 200); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 200); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 200); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 200); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 200); tft.print("ON");


//set point 4
tft.drawRect(8, 238, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 240); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 240); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 240); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 240); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 240); tft.print("ON");


tft.drawBitmap(206, 286, homeButton, 30, 30, BLACK);
}

void setPointsWeekendsPage(){
 screenState = "SETPOINTS_WEEKEND";
header();

tft.setTextSize(2); tft.setCursor(10, 40); tft.print("Weekend Set Points");
tft.setTextSize(2); tft.setCursor(10, 80); tft.print("Click one to edit");

//set point 1

tft.drawRect(8, 118, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 120); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 120); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 120); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 120); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 120); tft.print("ON");

//set point 2
tft.drawRect(8, 158, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 160); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 160); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 160); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 160); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 160); tft.print("ON");

//set point 3
tft.drawRect(8, 198, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 200); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 200); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 200); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 200); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 200); tft.print("ON");


//set point 4
tft.drawRect(8, 238, 220, 20, BLACK);
tft.setTextSize(2); tft.setCursor(10, 240); tft.print("01:");
tft.setTextSize(2); tft.setCursor(50, 240); tft.print("30");
tft.setTextSize(2); tft.setCursor(90, 240); tft.print("PM");
tft.setTextSize(2); tft.setCursor(140, 240); tft.print("70");tft.setTextSize(1); tft.print((char)247);
tft.setTextSize(2); tft.setCursor(200, 240); tft.print("ON");


tft.drawBitmap(206, 286, homeButton, 30, 30, BLACK);
}


int editSetPoint(int pointNum){

 screenState = "EDIT_SETPOINT";
 header();

 printCenteredText("Edit Set Point", 2, 32);

// hours
 tft.setTextSize(3); tft.setCursor(20, 61); tft.print("Hour:");
 tft.setTextSize(3); tft.setCursor(120, 61); tft.print("< ");
 tft.setTextSize(3); tft.setCursor(150, 61); tft.print("00");
 tft.setTextSize(3); tft.setCursor(177, 61); tft.print(" >");

  // minutes
 tft.setTextSize(3); tft.setCursor(20, 105); tft.print("Min:");
 tft.setTextSize(3); tft.setCursor(120, 105); tft.print("< ");
 tft.setTextSize(3); tft.setCursor(150, 105); tft.print("00");
 tft.setTextSize(3); tft.setCursor(177, 105); tft.print(" >");

  // AM/PM
 tft.setTextSize(3); tft.setCursor(120, 149); tft.print("< ");
 tft.setTextSize(3); tft.setCursor(150, 149); tft.print("AM");
 tft.setTextSize(3); tft.setCursor(177, 149); tft.print(" >");

  // degrees
 tft.setTextSize(3); tft.setCursor(20, 193); tft.print("Temp:");
 tft.setTextSize(3); tft.setCursor(120, 193); tft.print("< ");
 tft.setTextSize(3); tft.setCursor(150, 193); tft.print("70");tft.setTextSize(2); tft.print((char)247);
 tft.setTextSize(3); tft.setCursor(177, 193); tft.print(" >");

  // on/off
 tft.setTextSize(3); tft.setCursor(120, 237); tft.print("< ");
 tft.setTextSize(3); tft.setCursor(150, 237); tft.print("ON");
 tft.setTextSize(3); tft.setCursor(177, 237); tft.print(" >");

 tft.drawBitmap(206, 286, homeButton, 30, 30, BLACK);

 return 0;
}


void editDateTimePage(){

screenState = "EDIT_DATETIME";
header();

tft.setTextSize(3); tft.setCursor(26, 40); tft.print("Edit Date/Time");
tft.drawBitmap(206, 286, homeButton, 30, 30, BLACK);
}


void header(){
tft.fillScreen(backgroundColor);
tft.setTextColor(BLACK);
tft.setTextSize(2); tft.setCursor(6,8); tft.print("Wed Oct 31 10:46 AM");
}


bool isInTouchZone(uint16_t coord_arr[], int box_width, int box_height, int x_point, int y_point){
 //box_width/height are the bounds of the actual object (character/square/etc) we are making the zone for

 int margin = 4; //safe zone of 4 px
 int x_min = coord_arr[0] - margin;
 int y_min = coord_arr[1] - margin;
 int x_max = coord_arr[0] + box_width + (margin);
 int y_max = coord_arr[1] + box_height + (margin);

 tft.drawRect(x_min, y_min, (x_max-x_min), (y_max-y_min), RED);

 if((x_min < x_point) && (x_point < x_max)){

   if((y_min < y_point) && (y_point < y_max)){

     return true;
   }
 }
 else {
   return false;
 }
}

void printCenteredText(String text, uint16_t size, uint16_t y_pos){

 uint16_t x_pos = (240 - ((text.length()  * 5 * size) + ((text.length() - 1) * size))) / 2;

 Serial.println("print centered text info");
 Serial.println(text.length());
 Serial.println(x_pos);

 tft.setTextSize(size); tft.setCursor(x_pos, y_pos); tft.print(text);

}
