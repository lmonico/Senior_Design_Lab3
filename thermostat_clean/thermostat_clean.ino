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
#include <Wire.h> // for clock
#include "RTClib.h" //for clock

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define WHITE    0xFFFF
#define ONE_WIRE_BUS 5

//mem addresses of set points in eeprom
#define eeAddress0 = 0
#define eeAddress1 = sizeof(setPoint)
#define eeAddress2 = 2*sizeof(setPoint)
#define eeAddress3 = 3*sizeof(setPoint)
#define eeAddress4 = 4*sizeof(setPoint)
#define eeAddress5 = 5*sizeof(setPoint)
#define eeAddress6 = 6*sizeof(setPoint)
#define eeAddress7 = 7*sizeof(setPoint)

/* touch zone sizes */
int bitmapLogoSize[] = {30, 30};
int footerButtonSize[] = {75, 29};
int settingsButtonSize[] = {180, 40};
int setPointButtonSize[] = {220, 20};
int numpadCellSize[] = {74, 45};

/* touch zone origins (top left corner) */
int home_origins[][2] = {
  {-1, 48},   //0: current temperature
  {-1, 152},  //1: "set to" text
  {-1, 175},  //2: set to temperature
  {-1, 216},  //3: "mode" text
  {-1, 236}   //4: mode setting
};

int editSetPoint_origins[][2] = {

    {24, 70},   //0: hour
    {74, 70},   //1: colon
    {99, 70},   //2: minute
    {174, 70},  //3: half
    {-1, 128},  //4: temp
    {-1, 208},  //5: status
    {-1, 288}   //6: save
  };

int editDateTime_origins[][2] = {

  {24, 70},   //0: hour
  {74, 70},   //1: colon
  {99, 70},   //2: minute
  {174, 70},  //3: half
  {-1, 125},  //4: day
  {32, 208},  //5: month
  {144, 208},  //6: date
  {-1, 288}   //7: save
};

int footer_origins[][2] = {
  {6, 286},   //0: bottom left button
  {0,0},      //1: center button
  {206, 286}  //2: right button
};

/* global information variables */
uint16_t backgroundColor = WHITE;
String screenState = "";
String lastScreenState = "";
int setTemp = 75;
int currentTemp = 0;
int oldTemp = 0;

String currentMode = "A/C";
struct arrWrap {int arr[2];};

String halfArray[] = {"AM", "PM"};

String statusArray[] = {"ON", "OFF"};

String dayArray[] = {/*"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"*/};

String padNums[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "<", "0"};

String monthArray[] = {/*"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"*/};

String setPointsInfo[4];

struct setPoint {
  int hour;
  int minute;
  String half;
  int temp;
  String status;
};

struct myDateTime {
  int hour;
  int minute;
  String half;
  String day_text;
  String month;
  int day_num;
};

setPoint setPointsArr[8];

myDateTime dateTime = {11, 30, "AM", "Wed", "Oct", 30};

int heatingLEDPin = 2;
int coolingLEDPin = 3;

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

  pinMode(heatingLEDPin, OUTPUT);
  pinMode(coolingLEDPin, INPUT);

  setPointsArr[0] = {10,30,"AM",76,"ON"};
  setPointsArr[1] = {11,30,"AM",76,"ON"};
  setPointsArr[2] = {12,30,"AM",76,"ON"};
  setPointsArr[3] = {12,45,"AM",76,"ON"};
  setPointsArr[4] = {10,30,"AM",76,"ON"};
  setPointsArr[5] = {11,30,"AM",76,"ON"};
  setPointsArr[6] = {12,30,"AM",76,"ON"};
  setPointsArr[7] = {12,45,"AM",76,"ON"};



  backgroundColor = WHITE;

  tft.setRotation(2);
  screenState = "Home";
  homePage();

}

/*
 * Main Loop
 */
void loop() {
  oldTemp = currentTemp;
  sensors.requestTemperatures();
  currentTemp = (uint8_t)((sensors.getTempCByIndex(0) * 1.8) + 32);

  if(oldTemp != currentTemp && screenState == "Home") {
      printTemp(home_origins[0], currentTemp, 11);
  }
  
  if(setTemp != currentTemp && screenState == "Home")
  {
    if(setTemp < currentTemp){
      if(currentMode == "A/C" || currentMode == "AUTO"){
        //backgroundColor = CYAN;
        digitalWrite(coolingLEDPin, HIGH);
        digitalWrite(heatingLEDPin, LOW);
        //homePage();
      }
      else{
        digitalWrite(coolingLEDPin, LOW);
        digitalWrite(heatingLEDPin, LOW);
      }
    }
    else if(setTemp > currentTemp)
    { 
      if(currentMode == "HEAT" || currentMode == "AUTO"){
        //backgroundColor = RED;
        digitalWrite(coolingLEDPin, LOW);
        digitalWrite(heatingLEDPin, HIGH);
        //homePage();
      }
      else{
        digitalWrite(coolingLEDPin, LOW);
        digitalWrite(heatingLEDPin, LOW);
      }
    }
    else if(currentMode == "OFF")
    {
      //backgroundColor = WHITE;
      digitalWrite(coolingLEDPin, LOW);
      digitalWrite(heatingLEDPin, LOW);
      //homePage();
    }
  }
  else
  {
      //backgroundColor = WHITE;
      digitalWrite(coolingLEDPin, LOW);
      digitalWrite(heatingLEDPin, LOW);
      //homePage();
  }
  
  
  if(!ctp.touched()){
    return;
  }

  // Retrieve a point
  TS_Point p = ctp.getPoint();

  if(screenState == "Home"){

    String modes[] = {"A/C", "HEAT", "AUTO", "OFF"};

    if(isInTouchZone(home_origins[2], textSize(String(setTemp) + "  ", 2).arr, p.x, p.y, 4)){
      setTemp = numpad("Set Temp", setTemp);
      homePage();
    }

    if(isInTouchZone(home_origins[4], textSize(String(currentMode), 3).arr, p.x, p.y, 4)){
      currentMode = setUpButtons(modes, 4, "Select Mode", false, "");

      homePage();
    }

    //settings button
    if(isInTouchZone(footer_origins[2], bitmapLogoSize, p.x, p.y, 4)){

      screenState = "Settings";
    }

  }

  if(screenState == "Settings"){
    String settingsOptions[] = {"Set Points", "Edit Date/Time"};
    screenState = setUpButtons(settingsOptions, 2, "Settings", true, "Home");
  }

  if(screenState == "Set Points"){
    String setPointsOptions[] = {"Weekdays", "Weekends"};
    screenState = setUpButtons(setPointsOptions, 2, "Set Points", true, "Settings");
  }

  if(screenState == "Weekdays"){

   setPointsInfo[0] = setPointToString(setPointsArr[0]);
   setPointsInfo[1] = setPointToString(setPointsArr[1]);
   setPointsInfo[2] = setPointToString(setPointsArr[2]);
   setPointsInfo[3] = setPointToString(setPointsArr[3]);
   screenState = setUpButtons(setPointsInfo, 4, "Weekdays", true, "Set Points");

   for(int i = 0; i < 4; i++){
    if(screenState == setPointsInfo[i]){
      setPointsArr[i] = editSetPoint(setPointsArr[i]);
      screenState = "Weekdays";
    }
   }
  }

  if(screenState == "Weekends"){

    setPointsInfo[0] = setPointToString(setPointsArr[4]);
    setPointsInfo[1] = setPointToString(setPointsArr[5]);
    setPointsInfo[2] = setPointToString(setPointsArr[6]);
    setPointsInfo[3] = setPointToString(setPointsArr[7]);
    screenState = setUpButtons(setPointsInfo, 4, "Weekends", true, "Set Points");

    for(int i = 0; i < 4; i++){

      if(screenState == setPointsInfo[i]){
        setPointsArr[i+4] = editSetPoint(setPointsArr[i+4]);
        screenState = "Weekends";
      }
    }
  }

  if(screenState == "Edit Date/Time"){



     if(isInTouchZone(editDateTime_origins[0], textSize(String(dateTime.hour), 4).arr, p.x, p.y, 4)){
        dateTime.hour = numpad("Hour", dateTime.hour); editDateTimePage();
     }
     else if(isInTouchZone(editDateTime_origins[2], textSize(String(dateTime.minute), 4).arr, p.x, p.y, 4)){
        dateTime.minute = numpad("Minute", dateTime.minute); editDateTimePage();
     }
     else if(isInTouchZone(editDateTime_origins[3], textSize(dateTime.half, 4).arr, p.x, p.y, 4)){
        dateTime.half = setUpButtons(halfArray, 2, "Select AM/PM", false, ""); editDateTimePage();
     }
     else if(isInTouchZone(editDateTime_origins[4], textSize(dateTime.day_text, 4).arr, p.x, p.y, 4)){
        dateTime.day_text = setUpButtons(dayArray, 2, "Select Day", false, ""); editDateTimePage();
     }
     else if(isInTouchZone(editDateTime_origins[5], textSize(dateTime.month, 4).arr, p.x, p.y, 4)){
        dateTime.month = monthArray[numpad("Month", 12) - 1]; editDateTimePage();
     }
     else if(isInTouchZone(editDateTime_origins[6], textSize(String(dateTime.day_num), 4).arr, p.x, p.y, 4)){
        dateTime.day_num = numpad("Day", dateTime.day_num); editDateTimePage();
     }
     else if(isInTouchZone(editDateTime_origins[7], textSize("SAVE", 4).arr, p.x, p.y, 4)){
        screenState = "Settings";
     }

     editDateTimePage();
   }

}


/*
 * Home Page
 */
void homePage(){

  header();
  //display current temp
  printTemp( home_origins[0], currentTemp, 11);

  //display set to block
  printText( home_origins[1], "Set To", 2);
  printTemp(home_origins[2], setTemp, 3);

  //display mode block
  printText(home_origins[3], "Mode", 2);
  printText(home_origins[4], currentMode, 3);


  //display hold button
  tft.drawRect(6, 286, 75, 29, BLACK);
  tft.setTextSize(3); tft.setCursor(9, 291); tft.print("HOLD");

  //display settings button
  tft.drawBitmap(footer_origins[2][0], footer_origins[2][1], settingsButton, 30, 30, BLACK);
}

struct setPoint editSetPoint(struct setPoint setpoint){

  bool save = false;

  while(!save){

    header();

    int editSetPointText[] = {-1, 32};
    printText(editSetPointText, "Edit Set Point", 2);

    printText(editSetPoint_origins[0], String(setpoint.hour), 4);
    printText(editSetPoint_origins[1], ":", 4);
    printText(editSetPoint_origins[2], String(setpoint.minute), 4);
    printText(editSetPoint_origins[3], String(setpoint.half), 4);
    printTemp(editSetPoint_origins[4], setpoint.temp, 6);
    printText(editSetPoint_origins[5], setpoint.status, 6);
    printText(editSetPoint_origins[6], "SAVE", 3);

    while(!ctp.touched()) {} //wait for touch
    TS_Point newTouch = ctp.getPoint();  // Retrieve a point

     if(isInTouchZone(editSetPoint_origins[0], textSize(String(setpoint.hour), 4).arr, newTouch.x, newTouch.y, 4)){
        setpoint.hour = numpad("Hour", setpoint.hour);
     }
     else if(isInTouchZone(editSetPoint_origins[2], textSize(String(setpoint.minute), 4).arr, newTouch.x, newTouch.y, 4)){
        setpoint.minute = numpad("Minute", setpoint.minute);
     }
     else if(isInTouchZone(editSetPoint_origins[3], textSize(String(setpoint.half), 4).arr, newTouch.x, newTouch.y, 4)){
        setpoint.half = setUpButtons(halfArray, 2, "Select AM/PM", false, "");
     }
     else if(isInTouchZone(editSetPoint_origins[4], textSize(String(setpoint.temp), 6).arr, newTouch.x, newTouch.y, 4)){
        setpoint.temp = numpad("Temp", setpoint.temp);
     }
     else if(isInTouchZone(editSetPoint_origins[5], textSize(String(setpoint.status), 6).arr, newTouch.x, newTouch.y, 4)){
        setpoint.status = setUpButtons(statusArray, 2, "Select Status", false, "");
     }
     else if(isInTouchZone(editSetPoint_origins[6], textSize(String("SAVE"), 3).arr, newTouch.x, newTouch.y, 4)){


        return setpoint;
     }
   }
}


void editDateTimePage(){


  //screenState = "Edit Date/Time";
  header();

  int editDateTimeText[] = {-1, 32};
  printText(editDateTimeText, "Edit Date/Time", 2);

  printText(editDateTime_origins[0], String(dateTime.hour), 4);
  printText(editDateTime_origins[1], ":", 4);
  printText(editDateTime_origins[2], String(dateTime.minute), 4);
  printText(editDateTime_origins[3], dateTime.half, 4);
  printText(editDateTime_origins[4], dateTime.day_text, 4);
  printText(editDateTime_origins[5], dateTime.month, 4);
  printText(editDateTime_origins[6], String(dateTime.day_num), 4);
  printText(editDateTime_origins[7], "SAVE", 3);
}


void header(){
tft.fillScreen(backgroundColor);
tft.setTextColor(BLACK);
tft.setTextSize(2); tft.setCursor(6,8); tft.print("Wed Oct 31 10:46 AM");
}

void footer(){
  //display hold button
  tft.drawRect(footer_origins[0][0], footer_origins[0][1], footerButtonSize[0], footerButtonSize[1], BLACK);
  tft.setTextSize(3); tft.setCursor(9, 291); tft.print("BACK");

  //display settings button
  tft.drawBitmap(footer_origins[2][0], footer_origins[2][1], homeButton, bitmapLogoSize[0], bitmapLogoSize[1], BLACK);
}

int numpad(String editName, int prevVal){

  header();

  int numpadTitleOrigin[] = {-1, 30}; printText(numpadTitleOrigin, "Enter " + editName, 2); //print out title
  String prevValString = String(prevVal); int numOrigin[] = {-1, 52}; printText(numOrigin, prevValString, 9); //print out selected numbers (default is previous value)
  tft.drawLine(70, 125, 115, 125, BLACK); tft.drawLine(122, 125, 168, 125, BLACK); //selected number underlines

  String firstNumber = "0"; String secondNumber = "0"; //for the number selection mechanics
  bool enter = false;
  int origin[2];

  //print everything out first
  int numCounter = 0; //for iterating through the padNums array
  for(int y = 132; y < 300; y+=numpadCellSize[1]){
    for(int x = 8; x < 200; x+=numpadCellSize[0]){
      tft.drawRect(x, y, numpadCellSize[0], numpadCellSize[1], BLACK);
      int padNumOrigin[] = {27 + x, 8 + y}; printText(padNumOrigin, padNums[numCounter], 4);
      numCounter++;
    }
  }

  int enterOrigin[] = {164, 282}; printText(enterOrigin, "ENTER", 2); //print enter since different size

  while(!enter){

    while(!ctp.touched()) {} //wait for touch
    TS_Point newTouch = ctp.getPoint();  // Retrieve a point

    numCounter = 0;
    for(int y = 132; y < 300; y+=numpadCellSize[1]){
      for(int x = 8; x < 200; x+=numpadCellSize[0]){

        origin[0] = x;
        origin[1] = y;

        if(isInTouchZone(origin, numpadCellSize, newTouch.x, newTouch.y, 0)){

          if(numCounter == 9){ //backspace
            if(firstNumber == "0"){secondNumber = "0";}
            else {secondNumber = firstNumber; firstNumber = "0";}}
          else if(numCounter == 11){ //enter
            return (firstNumber + secondNumber).toInt();
            enter=true;}
          else if(firstNumber == "0"){ //anything else
            firstNumber = secondNumber; secondNumber = padNums[numCounter];}
          printText(numOrigin, firstNumber + secondNumber, 9);
          delay(200); //small delay is needed so multiple numbers don't register
        }
        numCounter++;
      }
    }
  }
}



String setUpButtons(String *option_arr, int arraySize, String title, bool isFooter, String lastScreenState){

  header();
  if(isFooter==true){footer();}

  int titleOrigin[] = {-1, 32}; printText(titleOrigin, title, 3);
  int buttonAreaHeight = 218;
  int buttonSpacing = 8;
  bool doubleColumn = false;
  bool buttonTouched = false;
  int buttonHeight = (buttonAreaHeight/arraySize) - buttonSpacing;
  int buttonWidth = 208;
  if(buttonHeight < 45){doubleColumn = true; buttonWidth = 88;}

  int optionCounter = 0;
  int x = 16;
  for(int y = 66; optionCounter < arraySize; y += (buttonHeight + buttonSpacing)){

    tft.drawRect(x, y, buttonWidth, buttonHeight, BLACK);
    int textOrigin[] = {x + 8, y + 8}; printText(textOrigin, option_arr[optionCounter], 2);

     optionCounter++;

  }

  while(!buttonTouched){

    while(!ctp.touched()) {} //wait for touch
    TS_Point newTouch = ctp.getPoint();  // Retrieve a point

    int optionCounter = 0;
    int x = 16;
    for(int y = 66; optionCounter < arraySize; y += (buttonHeight + buttonSpacing)){
      int origin[] = {x,y};
      int setupButtonSize[] = {buttonWidth, buttonHeight};
      if(isInTouchZone(origin, setupButtonSize, newTouch.x, newTouch.y, 0)){

        return option_arr[optionCounter];
      }

     optionCounter++;
    }

    if(isFooter == true){

       if(isInTouchZone(footer_origins[0], footerButtonSize, newTouch.x, newTouch.y, 0)){if(lastScreenState == "Home"){homePage();}return lastScreenState;} //back button

       if(isInTouchZone(footer_origins[2], bitmapLogoSize, newTouch.x, newTouch.y, 0)){homePage(); return "Home";} //home button

    }
  }
}

bool isInTouchZone(int origin[], int box_dim[], int x_point, int y_point, int margin){
  //box_dim is the bounds of the actual object (character/square/etc) we are making the zone for as {width, height}

  int x_min = origin[0] - margin;
  int y_min = origin[1] - margin;
  int x_max = origin[0] + box_dim[0] + margin;
  int y_max = origin[1] + box_dim[1] + margin;

  tft.drawRect(x_min, y_min, (x_max-x_min), (y_max-y_min), RED); //draw the touch zone for debugging purposes

  if((x_min < x_point) && (x_point < x_max)){if((y_min < y_point) && (y_point < y_max)){return true;}}

  return false;
}

//origin is an array of the top left corner coordinates
void printText(int origin[], String text, int size){

  if(origin[0] == -1){origin[0] = findCenter(text.length(), size);}  //center text if x-position is -1
  tft.setTextColor(BLACK, backgroundColor); tft.setTextSize(size); tft.setCursor(origin[0], origin[1]); tft.print(text);
}

void printTemp(int origin[], int temp, int size){

  if(origin[0] == -1){ origin[0] = findCenter(2, size);} //center temp if passing -1 as x_pos

  tft.setTextColor(BLACK, backgroundColor); tft.setTextSize(size); tft.setCursor(origin[0], origin[1]); tft.print(temp);
  if(size == 11){ tft.setTextSize(5); tft.print((char)247);}
  else {tft.setTextSize(size-1); tft.print((char)247);} //print degrees symbol
}

int findCenter(int length, int size){return (240 - ((length  * 5 * size) + ((length - 1) * size))) / 2;}

//calculates text size array to make touch zone
struct arrWrap textSize(String text, int size){
  struct arrWrap textSize;
  textSize.arr[0] = (text.length() * 5 * size) + ((text.length() - 1) * size);
  textSize.arr[1] = (8 * size) + 4;
  return textSize;
}

String setPointToString(struct setPoint setpoint){

  return String(setpoint.hour) + ":" + String(setpoint.minute) + " " + setpoint.half + " " + String(setpoint.temp) + ((char)247) + " " + setpoint.status;
}
