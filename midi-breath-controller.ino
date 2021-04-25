#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3f, 20, 4);

const int PIN_SENSOR = A0;
const int PIN_MIDI_CHANNEL = A1;
const int PIN_CC = A2;
const int PIN_THRESHOLD = A3;
// connect midi plug on pin TX of the arduino

int last_midi_val_out = -1;

void setup() {
  Serial.begin(31250);
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight
}

void sendCC(byte channel, byte number, byte value){
  Serial.write( 0xB0 | channel);
  Serial.write(number);  // controller number to use
  Serial.write(value);  // value to set controller
}

// just for debugging
void sendNote(byte channel, byte pitch, byte velocity) {
  Serial.write(0x90 + channel - 1);
  Serial.write(pitch);  // controller number to use
  Serial.write(velocity);  // value to set controller
}

const int CC_NUMS[] = {1, 4, 5, 11, 64, 65, 66, 67, 68,};
const char * CC_NAMES[] = {
    "modwheel",
    "foot pedal",
    "portamento",
    "expression",
    "damper",
    "portamento",
    "sostenuto",
    "softpedal",
    "legato",
};
const int CC_TABLE_LEN = sizeof(CC_NUMS) / sizeof(int*);


void draw_screen(int sensor_raw_value, int sensor_value, int midi_channel, int cc_index, int threshold) {
  int LINE_LENGTH = 20;

  char line[LINE_LENGTH];

  lcd.setCursor ( 0, 0 );
  memset(line, ' ', LINE_LENGTH);
  sprintf(line, "raw %4d | MIDI %3d", sensor_raw_value, sensor_value);
  lcd.print(line);

  lcd.setCursor ( 0, 1 );
  memset(line, ' ', LINE_LENGTH);
  sprintf(line, "midi channel: %2d", midi_channel + 1);
  lcd.print(line);

  lcd.setCursor ( 0, 2 );
  memset(line, ' ', LINE_LENGTH);
  sprintf(line, "cc %3d %13s", CC_NUMS[cc_index], CC_NAMES[cc_index]);
  lcd.print(line);

  lcd.setCursor ( 0, 3 );
  memset(line, ' ', LINE_LENGTH);
  sprintf(line, "low threshold: %3d", threshold);
  lcd.print(line);

}

void loop() {
  
  int sensor_raw_value = analogRead(PIN_SENSOR);
  int midi_channel = (analogRead(PIN_MIDI_CHANNEL) / 64);        // 1 to 16
  int threshold = max(0, (analogRead(PIN_THRESHOLD) / 8 - 1));       // scale to midi 0-127
  int midi_out_value = max(0, (sensor_raw_value / 8 - 1 - threshold)); // scale to midi 0-127
  int cc_index = max(0, analogRead(PIN_CC) * CC_TABLE_LEN / 1024); // scale to 0 -> CC_TABLE_LEN - 1
  draw_screen(sensor_raw_value, midi_out_value, midi_channel, cc_index, threshold);
  if (midi_out_value != last_midi_val_out) {
    last_midi_val_out = midi_out_value;
    sendCC(midi_channel, CC_NUMS[cc_index], midi_out_value);
  }
  
  delay(5);
}
