 /***************************************************
  Arduino Thermostat
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

uint16_t home_setLeftArrow[] = {60, 176};
uint16_t home_setRightArrow[] = {138, 176};
uint16_t home_modeLeftArrow[] = {49, 240};
uint16_t home_modeRightArrow[] = {169, 240};
uint16_t home_holdButton[] = {0,0};
uint16_t home_settingsButton[] = {206, 286};

uint16_t backgroundColor = WHITE;
int setTemp = 75;
int currentTemp = 79;
String modes[] = {"COOL", "HEAT", "AUTO", "OFF"};
int currentModeIndex = 0;
String screenState = "";

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


void setup(void) {
  while (!Serial);     // used for leonardo debugging
 
  Serial.begin(115200);
  
  tft.begin();

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  //if cooling
backgroundColor = WHITE;

tft.setRotation(2);
homePage();

}


void loop() {
  // Wait for a touch
  if (! ctp.touched()) {
    return;
  }

  // Retrieve a point  
  TS_Point p = ctp.getPoint();

  //flip it around to match the screen.
  //Rotation #3 setting
  //int oldX = p.x;
  //int oldY = p.y;
  //p.x = map(oldY, 0, 320, 0, 320); 
  //p.y = map(oldX, 0, 240, 240, 0);


  
 

  if(screenState = "HOME"){

    //set temp arrows
    if(isInTouchZone(home_setLeftArrow, p.x, p.y)){
      setTemp = setTemp - 1;
      homePage();
    }

    if(isInTouchZone(home_setRightArrow, p.x, p.y)){
      setTemp = setTemp + 1;
      homePage();
    }

    if(isInTouchZone(home_modeLeftArrow, p.x, p.y)){

      if (currentModeIndex > 0){
        currentModeIndex -= 1;
      }
      else {
        currentModeIndex = 3;
      }
      homePage();
    }

    if(isInTouchZone(home_modeLeftArrow, p.x, p.y)){

      if (currentModeIndex < 3){
        currentModeIndex += 1;
      }
      else {
        currentModeIndex = 0;
      }
      homePage();
    }

    
    //settings button
    if(isInTouchZone(home_settingsButton, p.x, p.y)){

      settingsPage();
    }

    

    
    
  }


  if(screenState = "SETTINGS"){

  
  }

}



void homePage(){

  screenState = "HOME";
  
  tft.fillScreen(backgroundColor);
  tft.setTextColor(BLACK);

  //display date and time
  tft.setTextSize(2);
  tft.setCursor(6,8);
  tft.print("Wed Oct 31 10:46 AM");

  //display current temp
  tft.setTextSize(11);
  tft.setCursor(55, 48);
  tft.print(currentTemp);
  tft.setTextSize(5);
  tft.print((char)247);

  //display set to block
  tft.setTextSize(2);
  tft.setCursor(80, 152);
  tft.print("Set To");
  tft.setTextSize(3);
  tft.setCursor(home_setLeftArrow[0], home_setLeftArrow[1]);
  tft.print("< ");
  tft.print(setTemp);
  tft.setTextSize(2);
  tft.print((char)247);
  tft.setTextSize(3);
  tft.setCursor(home_setRightArrow[0], home_setRightArrow[1]);
  tft.print(" >");

  //display mode block
  tft.setTextSize(2);
  tft.setCursor(92, 216);
  tft.print("Mode");
  tft.setTextSize(3);
  tft.setCursor(home_modeLeftArrow[0], home_modeLeftArrow[1]);
  tft.print("< ");
  tft.print(modes[currentModeIndex]);
  tft.print(" >");

  //display hold button
  tft.drawRect(6, 286, 75, 29, BLACK);
  tft.setTextSize(3);
  tft.setCursor(9, 291);
  tft.setTextColor(BLACK);
  tft.print("HOLD");

  //display settings button
  tft.drawBitmap(206, 286, settingsButton, 30, 30, BLACK);
  
  }



  

void settingsPage(){

  screenState = "SETTINGS";
  
  tft.fillScreen(backgroundColor);
  tft.drawBitmap(280, 10, homeButton, 30, 30, BLACK); // pixel location of homepage: 280-310 x, 10-40 y
  
  tft.setTextSize(3);
  tft.setTextColor(0x0000);

  tft.setCursor(40,40);
  tft.print("Settings");
 
  }

bool isInTouchZone(uint16_t coord_arr[], int x_point, int y_point){

    int zone_bound = 30; //makes a square of this many pixels to be touch zone

    if((coord_arr[0] < x_point) && (x_point < (coord_arr[0] + zone_bound))){

      if((coord_arr[1] < y_point) && (y_point < (coord_arr[0] + zone_bound))){

        return true;
      }
    }
    else {
      return false;
    }
  }


  
