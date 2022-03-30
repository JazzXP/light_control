#include <Arduino.h>

#include <FastLED.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED_COUNT 144

#define LED_PIN 23
#define UP_BUTTON_PIN 12
#define DOWN_BUTTON_PIN 27
#define ENTER_BUTTON_PIN 14
#define SOFT_POWER_PIN 19
#define I2C_SDA 22
#define I2C_SCL 21

#define HUE_INC (255.0 / LED_COUNT)

#define BUTTON_NONE 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_ENTER 3

byte buttonPressed = BUTTON_NONE;

#define MENU_ITEM_OFF 0
#define MENU_ITEM_ALL 1
#define MENU_ITEM_LEFT 2
#define MENU_ITEM_CENTER 3
#define MENU_ITEM_RIGHT 4
#define MENU_ITEM_COUNT (MENU_ITEM_RIGHT + 1)

byte menuItem = MENU_ITEM_ALL;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1

TwoWire DisplayWire = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &DisplayWire, OLED_RESET);

CRGB leds[LED_COUNT];
CRGB prevLeds[LED_COUNT];
CRGB nextLeds[LED_COUNT];

unsigned long last_interrupt_time[] = {0, 0, 0};

bool power = true;

void handleInput(int button) {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time[button] > 200) {
    last_interrupt_time[button] = interrupt_time;
    buttonPressed = button;
  }
}

void IRAM_ATTR handleUpInput() {
  handleInput(BUTTON_UP);
}
void IRAM_ATTR handleDownInput() {
  handleInput(BUTTON_DOWN);
}
void IRAM_ATTR handleEnterInput() {
  handleInput(BUTTON_ENTER);
}

void IRAM_ATTR handlePowerInput() {
  power = !power;
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("White LED Menu");
  display.println("--------");
  if (menuItem == MENU_ITEM_OFF) {
    display.println("->Off");  
  } else {
    display.println("  Off");  
  }
  if (menuItem == MENU_ITEM_ALL) {
    display.println("->All");  
  } else {
    display.println("  All");  
  }
  if (menuItem == MENU_ITEM_LEFT) {
    display.println("->Left");  
  } else {
    display.println("  Left");  
  }
  if (menuItem == MENU_ITEM_CENTER) {
    display.println("->Center");  
  } else {
    display.println("  Center");  
  }
  if (menuItem == MENU_ITEM_RIGHT) {
    display.println("->Right");  
  } else {
    display.println("  Right");  
  }
  
  display.display(); 
}

void setup() {
  DisplayWire.setPins(I2C_SDA, I2C_SCL);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENTER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SOFT_POWER_PIN, INPUT_PULLUP);
  
  attachInterrupt(UP_BUTTON_PIN, handleUpInput, FALLING);
  attachInterrupt(DOWN_BUTTON_PIN, handleDownInput, FALLING);
  attachInterrupt(ENTER_BUTTON_PIN, handleEnterInput, FALLING);
  attachInterrupt(SOFT_POWER_PIN, handlePowerInput, FALLING);
  Serial.begin(9600);
  Serial.println(F("Starting"));

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed")); // Typically OOM
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display(); 
  
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
  for (byte i = 0; i < LED_COUNT; i++) {
    leds[i] = CHSV((uint8_t)(i * HUE_INC), 255, 255);
    delay(25);
    FastLED.show();
  }
}
bool outputMenuSerial = false;
void loop() {
  static bool transition = false;
  switch (buttonPressed) {
    case BUTTON_UP:
      if (menuItem > 0) { menuItem--; }
      break;
    case BUTTON_DOWN:
      if (menuItem < MENU_ITEM_COUNT-1) { menuItem++; }
      break;
    case BUTTON_ENTER:
      memcpy(prevLeds, leds, sizeof(CRGB) * LED_COUNT);
      fill_solid(nextLeds, LED_COUNT, CRGB::Black);
      transition = true;
      switch(menuItem) {
        case MENU_ITEM_OFF:
          fill_solid(nextLeds, LED_COUNT, CRGB::Black);
          break;
        case MENU_ITEM_ALL:
          fill_solid(nextLeds, LED_COUNT, CRGB::White);
          outputMenuSerial = true;
          break;
        case MENU_ITEM_LEFT:
          fill_solid(nextLeds, LED_COUNT / 3, CRGB::White);
          break;
        case MENU_ITEM_CENTER:
          fill_solid(&(nextLeds[(LED_COUNT / 2) - (LED_COUNT / 3 / 2)]), LED_COUNT / 3, CRGB::White);
          break;
        case MENU_ITEM_RIGHT:
          fill_solid(&(nextLeds[LED_COUNT - (LED_COUNT / 3)]), LED_COUNT / 3, CRGB::White);
          break;
      }
      break;
  }
  buttonPressed = BUTTON_NONE;
  drawMenu();

  static fract8 pct = 0;
  static fract8 old_pct = pct;
  if (transition) {
    old_pct = pct;
    pct+=2;
    if (pct == 255 || old_pct > pct) {
      transition = false;
      pct = 0;
      old_pct = 0;
      memcpy(leds, nextLeds, sizeof(CRGB) * LED_COUNT);
    } else {
      blend(prevLeds, nextLeds, leds, LED_COUNT, pct);  
    }
    
    FastLED.show();
    //delay(2);
  }
}
