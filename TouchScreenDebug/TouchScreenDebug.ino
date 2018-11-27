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

uint16_t arrowSize3[] = {15, 21}; //width, height of a size 3 arrow
uint16_t bitmapLogoSize[] = {30, 30};
uint16_t settingsButtonSize[] = {180, 40};

uint16_t home_setLeftArrow[] = {60, 176};
uint16_t home_setRightArrow[] = {138, 176};
uint16_t home_modeLeftArrow[] = {49, 240};
uint16_t home_modeRightArrow[] = {153, 240};
uint16_t home_holdButton[] = {0,0};

uint16_t settings_setPoints[] = {30, 96};
uint16_t settings_editTime[] = {30, 160};

uint16_t setPoints_weekdays[] = {30, 96};
uint16_t setPoints_weekends[] = {30, 160};

uint16_t br_cornerButton[] = {206, 286};

uint16_t backgroundColor = WHITE;






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

  
  
  
}







  

bool isInTouchZone(uint16_t coord_arr[], int box_width, int box_height, int x_point, int y_point){
    //box_width/height are the bounds of the actual object (character/square/etc) we are making the zone for

    int margin = 4; //safe zone of 4 px
    int x_min = coord_arr[0] - margin;
    int y_min = coord_arr[1] - margin;
    int x_max = coord_arr[0] + box_width + margin;
    int y_max = coord_arr[1] + box_height + margin;

    if((x_min < x_point) && (x_point < x_max)){

      if((y_min < y_point) && (y_point < y_max)){

        return true;
      }
    }
    else {
      return false;
    }
  }
