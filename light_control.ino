#include <FastLED.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED_COUNT 142
#define LED_PIN 6
#define HUE_INC (255.0 / LED_COUNT)
#define INTERRUPT_PIN 2
#define UP_BUTTON_PIN 3
#define DOWN_BUTTON_PIN 4
#define ENTER_BUTTON_PIN 5

CRGB leds[LED_COUNT];
CRGB prevLeds[LED_COUNT];
CRGB nextLeds[LED_COUNT];

#define BUTTON_NONE 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_ENTER 3

uint8_t buttonPressed = BUTTON_NONE;

#define MENU_ITEM_ALL 0
#define MENU_ITEM_LEFT 1
#define MENU_ITEM_CENTER 2
#define MENU_ITEM_RIGHT 3
#define MENU_ITEM_COUNT (MENU_ITEM_RIGHT + 1)

uint8_t menuItem = MENU_ITEM_ALL;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENTER_BUTTON_PIN, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInput, CHANGE);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    leds[i] = CHSV((uint8_t)(i * HUE_INC), 255, 255);
    delay(50);
    FastLED.show();
  }
  delay(500);
}

void loop() {
  static bool transition = false;
  switch (buttonPressed) {
    case BUTTON_UP:
      if (menuItem > 0) { menuItem--; }
      break;
    case BUTTON_DOWN:
      if (menuItem < MENU_ITEM_COUNT) { menuItem++; }
      break;
    case BUTTON_ENTER:
      memcpy(prevLeds, leds, sizeof(CRGB) * LED_COUNT);
      fill_solid(nextLeds, LED_COUNT, CRGB::Black);
      transition = true;
      switch(menuItem) {
        case MENU_ITEM_ALL:
          fill_solid(nextLeds, LED_COUNT, CRGB::White);
          break;
        case MENU_ITEM_LEFT:
          fill_solid(nextLeds, LED_COUNT / 3, CRGB::White);
          break;
        case MENU_ITEM_CENTER:
          fill_solid(&(nextLeds[(LED_COUNT / 2) - (LED_COUNT / 3)]), LED_COUNT / 3, CRGB::White);
          break;
        case MENU_ITEM_RIGHT:
          fill_solid(&(nextLeds[LED_COUNT - (LED_COUNT / 3)]), LED_COUNT / 3, CRGB::White);
      }
      break;
  }
  buttonPressed = BUTTON_NONE;
  drawMenu();

  static fract8 pct = 0;
  if (transition) {
    pct++;
    if (pct == 255) {
      transition = false;
    }
    blend(prevLeds, nextLeds, leds, LED_COUNT, pct);
    FastLED.show();
    delay(20);
  }
}

void handleInput() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    if (digitalRead(UP_BUTTON_PIN)) {
      buttonPressed = BUTTON_UP;
      return;
    } else if (digitalRead(DOWN_BUTTON_PIN)) {
      buttonPressed = BUTTON_DOWN;
      return;
    } else if (digitalRead(ENTER_BUTTON_PIN)) {
      buttonPressed = BUTTON_ENTER;
      return;
    }
    buttonPressed = BUTTON_NONE;
  }
  last_interrupt_time = interrupt_time;
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("White LED Menu");
  display.println("--------");
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
