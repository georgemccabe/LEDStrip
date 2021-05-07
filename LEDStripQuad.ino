#include <FastLED.h>
#include <MIDIUSB.h>

#define POWER_PIN   2
#define LED_PIN     5
#define MIN_BRIGHTNESS  0
#define MAX_BRIGHTNESS  180
#define BRIGHTNESS  100

// 36, 72, 108, 144
#define NUM_LEDS    144
#define FOLLOW_INCREMENT   1

#define DELAY_BRIGHTNESS  5
#define DELAY_STROBE      50
#define DELAY_FOLLOW      10

#define FRONT_START 36
#define FRONT_END 108

// include 0 for off and 1 for loop all
#define NUM_LOOPS 7
int num_shuffles = 5;

#define Holly_Green 0x00580c
#define Holly_Red   0xB00402

//CRGB color_list[] = {CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Yellow, CRGB::Crimson, CRGB::LawnGreen, CRGB::Orange};
CRGB color_list[] = {CRGB::Grey, Holly_Red, Holly_Green, CRGB::Blue};


int current_color = 0;
int current_loop = 1;
int current_fade = 1;
int current_shuffle = 0;
int shuffle_iters = 1;
int current_shuffle_iter = 0;
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
//long debounce = 200;   // the debounce time, increase if the output flickers
long debounce = 600;   // the debounce time, increase if the output flickers

int current_follow = 0;

CRGB leds[NUM_LEDS];
void setup() {
  delay( 3000 ); // power-up safety delay

  Serial.begin(9600);

  // set pin for power button
  pinMode(POWER_PIN, INPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 10000);
  FastLED.setBrightness(  BRIGHTNESS );
  FastLED.clear();
  FastLED.show();
}

bool upDirection = true;
bool upDirBright = true;
int current_led = 0;
int current_increment = 1;
int led_increment = 1;

int wait_amt=10;
int current_wait = 0;
int current_bright = MAX_BRIGHTNESS;
int bright_increment = 15;
int reverse = true;
bool strobeOn = true;

int num_walk_iters = 32;
int current_walk = 0;

int remainder = 1;

CRGB color_a = CRGB::Grey;
CRGB color_b = Holly_Red;

CRGB color_c = Holly_Red;
CRGB color_d = Holly_Green;

bool ledpp() {
  if(upDirection){
    current_led+= led_increment;
  }
  else {
    current_led-= led_increment;
  }

  if(current_led >= NUM_LEDS){
    if(reverse) {
      current_led = NUM_LEDS-1;
      upDirection = !upDirection;
    } else {
      current_led = 0;      
    }
    return true;
  }
  if(current_led < 0){
    if(reverse) {
      current_led = 0;
      upDirection = !upDirection;
    } else {
     current_led = NUM_LEDS-1; 
    }
    return true;
  }
  return false;
}

bool brightpp() {
  if(upDirBright){
    current_bright+= bright_increment;
  }
  else {
    current_bright-= bright_increment;
  }

  if(current_bright >= MAX_BRIGHTNESS){
    if(reverse) {
      current_bright = MAX_BRIGHTNESS-1;
      upDirBright = !upDirBright;
    } else {
      current_bright = MIN_BRIGHTNESS;
      return true;
    }
    return false;
  }
  if(current_bright < MIN_BRIGHTNESS){
    if(reverse) {
      current_bright = MIN_BRIGHTNESS;
      upDirBright = !upDirBright;
    } else {
     current_bright = MAX_BRIGHTNESS-1; 
    }
    return true;
  }
  return false;
}

void colorpp() {
  current_color++;
  if(current_color >= sizeof(color_list)/sizeof(CRGB)){
    current_color = 0;
  }
}
bool wait() {
  current_wait++;
  if(current_wait < wait_amt) {
    delay(1);
    return true;  
  }
  current_wait = 0;
  return false;  
}

int offset=0;

bool fill_2(int wait_time, CRGB color1, CRGB color2, int size){
  wait_amt = wait_time;
  if(wait()) { return false; }
  FastLED.setBrightness(  BRIGHTNESS );
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB use_color;
    if(i % (size*2) < size) {
      use_color = color1;
    } else {
      use_color = color2;
    }
    int index = i + offset;
    if(index >= NUM_LEDS) {
      index -= NUM_LEDS;  
    }
    leds[index] = use_color;
  }
  FastLED.show();
  offset++;
  if(offset >= NUM_LEDS) {
    offset = 0;
  }
  return true;
}

bool follow(CRGB color){
  if(wait()) { return false; }
  FastLED.setBrightness(  BRIGHTNESS );
  leds[current_led] = color;
  FastLED.show();
   
  if(ledpp()) {
    return true;
  }
  return false;
}

bool follow_alternate() {
  
}

CRGB get_strobe(CRGB color) {
 CRGB show_color;
 if(strobeOn) {
   show_color = color;
 }
 else {
   show_color = CRGB::Black;
 }
 strobeOn = !strobeOn;
 return show_color;
}

bool strobe(CRGB color) {
 if(wait()) { return false; }
 FastLED.setBrightness(  BRIGHTNESS );

 CRGB show_color = get_strobe(color);

 for (int i = 0; i < NUM_LEDS; i=i+led_increment) {
    leds[i] = show_color;
  }
  FastLED.show();
  return true;
}

bool fade_all(CRGB color) {
  // set all LEDs to color
  for (int i = 0; i < NUM_LEDS; i=i+led_increment) {
    leds[i] = color;
  }

  fade();
}

bool fade() {
  if(wait()) { return false; }

  FastLED.setBrightness(current_bright);  
  FastLED.show();
  if(brightpp()) {
    colorpp();
    return true;
  }

 return false;
}

bool random_flash(int wait_time, int iters) {
  wait_amt = wait_time;
  shuffle_iters = iters;
  if(wait()) { return false; }
  FastLED.setBrightness(  BRIGHTNESS );
  int randIters = random(NUM_LEDS, NUM_LEDS+40);
  for(int i=0; i<randIters; i++) {
    int randLED = random(NUM_LEDS);
    leds[randLED] = color_list[current_color++];
    int color_list_len = sizeof(color_list)/sizeof(CRGB);
    if(current_color >= color_list_len) {
      current_color=0;  
    }
  }
  FastLED.show();
  current_shuffle_iter++;
  return true;

}

void clear_all(){
    FastLED.setBrightness(  BRIGHTNESS );
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, 0);
    }
    FastLED.show();
}

void increment_current_shuffle() {
  if(current_shuffle == num_shuffles-1){
    current_shuffle = 0;  
  }
  else {
    current_shuffle++;  
  }
}
void shuffle_loop_b() {
  num_shuffles = 4;
  if(current_shuffle_iter == shuffle_iters) {
    increment_current_shuffle();
    current_shuffle_iter = 0;
  }

  if(current_shuffle == 0){
    candy_cane_walk(4);
  }
  else if(current_shuffle == 1){
    xmas_sides(4);
  }
  else if(current_shuffle == 2){
    wait_amt = 10;
    led_increment=1;
    reverse=true;
    if(follow(color_list[current_color])) {
      colorpp();
      current_shuffle_iter++;
    }
    shuffle_iters = 4;
  }
  else if(current_shuffle == 3){
    random_flash(10, 100);
  }
}
void shuffle_loop_a() {
  num_shuffles = 5;
  if(current_shuffle_iter == shuffle_iters) {
    increment_current_shuffle();
    current_shuffle_iter = 0;
  }

  if(current_shuffle == 0){
    wait_amt = 10;
    led_increment=1;
    reverse=true;
    if(follow(color_list[current_color])) {
      colorpp();
      current_shuffle_iter++;
    }
    shuffle_iters = 4;
  }
  else if(current_shuffle == 1){
    // strobe colors
    wait_amt = 50;
    led_increment=2;
    if(strobe(color_list[current_color])) {
      colorpp();
      current_shuffle_iter++;
    }
    shuffle_iters = 20;
  }
  else if(current_shuffle == 2){
    // fade colors
    wait_amt = 100;
    reverse=true;
    led_increment=2;
    if(fade_all(color_list[current_color])) {
      current_shuffle_iter++;  
    }
    shuffle_iters = 4;
  }
  else if(current_shuffle == 3){
    // spin alternating colors
    wait_amt = 20;
    reverse=false;
    led_increment=2;
    // if odd, make even or if even make odd
    if(current_led % 2 == remainder) {
      current_led++;
    }
    if(follow(color_list[current_color])) {
      colorpp();
      current_led = 1;
      remainder = !remainder;
      current_shuffle_iter++;
    }
    shuffle_iters = 8;
  }
  else if(current_shuffle == 4){
    random_flash(10, 100);
  }
}

void xmas_sides(int iters) {
  wait_amt = 120;
  // set all LEDs to color
  for (int i = 0; i < FRONT_START; i++) {
    leds[i] = color_c;
  }
  for (int i = FRONT_END; i < NUM_LEDS; i++) {
    leds[i] = color_c;
  }
  for (int i = FRONT_START; i < FRONT_END; i++) {
    leds[i] = color_d;
  }
  if(fade()) {
      CRGB temp = color_c;
      color_c = color_d;
      color_d = temp;
      current_shuffle_iter++;
  }
    shuffle_iters = iters;
}

void candy_cane_walk(int iters) {
   // candy cane walk
    if(fill_2(100, color_a, color_b, 4)) {
//      CRGB temp = color_a;
//      color_a = color_b;
//      color_b = temp;
      current_shuffle_iter++;
    }
    shuffle_iters = 16*iters;
}


void processMidi(midiEventPacket_t rx) {
    switch (rx.header) {
    case 0x0:
      // do nothing
      break;

    // note on
    case 0x9:
    Serial.print("NOTE ON");

//      handleNoteOn(rx.byte1 & 0xF, rx.byte2, rx.byte3);
      break;
    
      // note off
      case 0x8:
    Serial.print("NOTE OFF");
//        handleNoteOn(rx.byte1 & 0xF, rx.byte2, 0);
      break;

    // control change
    case 11:
        Serial.print("CC: ");
        Serial.print(rx.byte2);
        Serial.print(":");
        Serial.print(rx.byte3);
        Serial.print("\n");
      break;

    default:
      Serial.println(rx.header);
      break;
  }
}

void loop() {
  //listen for new MIDI messages
  midiEventPacket_t rx = MidiUSB.read();
  processMidi(rx);

//  check_power_button();
//  candy_cane_walk(4);
//shuffle_loop_b();
//xmas_sides();;
/*
  // spin alternating colors
  wait_amt = 10;
  reverse=false;
  led_increment=2;
  // if odd, make even
  if(current_led % 2 == remainder) {
    current_led++;
  }
  if(follow(color_list[current_color])) {
    colorpp();
    current_led = 1;
    remainder = !remainder;
  }
*/
/*
  // spin colors
  wait_amt = 10;
  reverse=false;
  if(follow(color_list[current_color])) {
    colorpp();
  }
*/

/*
  // strobe colors
  wait_amt = 50;
  if(strobe(color_list[current_color])) {
    colorpp();
  }
*/

/*
  // fade colors
  wait_amt = 100;
  fade_all(color_list[current_color]);
*/


  // shuffle multiple loops
//  shuffle_loop_b();

/*
  // random sparkle
  wait_amt = 10;
  random_flash();
*/
/*
  if(current_loop == 0) {
    clear_all();
  }
  else if(current_loop == 1) {
    shuffle_loop();
  }
  else if(current_loop == 2) {
    follow_loop();
  }
  else if(current_loop == 3) {
    strobe_1_loop();
  }
  else if(current_loop == 4) {
    fade_loop();
  }
  else if(current_loop == 5) {
    strobe_speedup_loop();
  }
  else if(current_loop == 6) {
    night_rider_loop();
  }
  else if(current_loop == 7) {
    random_flash_loop();
  }
*/

}
