#include "iot_api.hpp"
#include <analogWrite.h>
#include <Tone32.h>

#include <ArduinoJson.h>

const int led = 4;

const int redPin = 12;
const int greenPin = 13;
const int bluePin = 14;

void randomColor(const String& payload = "") {
  Serial.println("Random Color Called");
  analogWrite(led, random(0,255));
  analogWrite(redPin, random(0,255));
  analogWrite(greenPin, random(0,255));
  analogWrite(bluePin, random(0,255));
}

void randomColor() {
  randomColor("");
}

void switchOn() {
  Serial.println("Light on.");
  analogWrite(led, 255);
}

void switchOff() {
  Serial.println("Light off.");
  analogWrite(led, 0);
}

void RgbOn() {
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 255);
}

void RgbOff() {
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void rgb_red() {
  Serial.println("rgb_red called");
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void rgb_green() {
  Serial.println("rgb_green called");
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
}

void rgb_blue() {
  Serial.println("rgb_blue called");
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
}

void set_red(const double v) {
  analogWrite(redPin, (int)v);
}

void set_green(const double v) {
  analogWrite(greenPin, (int)v);
}

void set_blue(const double v) {
  analogWrite(bluePin, (int)v);
}

enum Node {
  C = 261,
  D = 294,
  E = 330,
  F = 349,
  G = 392,
  A = 440,
  H = 494,
  C2 = 523
};

const char* boci = "CECEG G CECEG G *HAGF A GFEDC C ";

void playNote(char c, int duration) {
  //Serial.print("playNote called: ");
  //Serial.println(c);
  switch (c) {
    case ' ':
      noTone(led, 0);
      delay(duration);
      break;
    case 'C':
      tone(led, Node::C, duration, 0);
      break;
    case 'D':
      tone(led, Node::D, duration, 0);
      break;
    case 'E':
      tone(led, Node::E, duration, 0);
      break;
    case 'F':
      tone(led, Node::F, duration, 0);
      break;
    case 'G':
      tone(led, Node::G, duration, 0);
      break;
    case 'A':
      tone(led, Node::A, duration, 0);
      break;
    case 'H':
      tone(led, Node::H, duration, 0);
      break;
    case '*':
      tone(led, Node::C2, duration, 0);
      break;
    default:
      delay(duration);
  }
}

#define MARIO_PIN 15

struct Note {
  const int note, dur;
  Note(int n, int d) : note(n), dur(d) {}

  void play() const {
    //Serial.println("Play Note: " + String(note) + ", " + String(dur));
    if (note)
      tone(MARIO_PIN, note, dur, 0);
    else {
      noTone(MARIO_PIN, 0);
      delay(dur);
    }
  }
};

const int qDur = 650;
Note mario[] = {
  Note(NOTE_E4, qDur/4),
  Note(NOTE_E4, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_E4, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_C4, qDur/4),
  Note(NOTE_E4, qDur/4),
  Note(0, qDur/4),
  
  Note(NOTE_G4, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_G3, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  
  Note(NOTE_C4, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_G3, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_E3, qDur/4),
  Note(0, qDur/4),
  
  Note(0, qDur/4),
  Note(NOTE_A3, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_B3, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_AS3, qDur/4),
  Note(NOTE_A3, qDur/4),
  Note(0, qDur/4),
 
  Note(NOTE_G3, qDur/3),
  Note(NOTE_E4, qDur/3),
  Note(NOTE_G4, qDur/3),
  Note(NOTE_A4, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_F4, qDur/4),
  Note(NOTE_G4, qDur/4),
  
  Note(0, qDur/4),
  Note(NOTE_E4, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_C4, qDur/4),
  Note(NOTE_D4, qDur/4),
  Note(NOTE_B3, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  

  Note(NOTE_C4, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_G3, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_E3, qDur/4),
  Note(0, qDur/4),
  
  Note(0, qDur/4),
  Note(NOTE_A3, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_B3, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_AS3, qDur/4),
  Note(NOTE_A3, qDur/4),
  Note(0, qDur/4),
 
  Note(NOTE_G3, qDur/3),
  Note(NOTE_E4, qDur/3),
  Note(NOTE_G4, qDur/3),
  Note(NOTE_A4, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_F4, qDur/4),
  Note(NOTE_G4, qDur/4),
  
  Note(0, qDur/4),
  Note(NOTE_E4, qDur/4),
  Note(0, qDur/4),
  Note(NOTE_C4, qDur/4),
  Note(NOTE_D4, qDur/4),
  Note(NOTE_B3, qDur/4),
  Note(0, qDur/4),
  Note(0, qDur/4)
};

bool playMarioIsPlaying = false;

void marioOn() {
  playMarioIsPlaying = true;
}

void marioOff() {
  playMarioIsPlaying = false;
}

void *playMario(void*) {
  static int i = 0;
  while (1) {
    if (playMarioIsPlaying) {
      mario[i++].play();
      if (i >= sizeof(mario)/8)
        i = 0;
    } else {
      delay(500);
    }
  }
}

void setup() {
  Serial.begin(115200);

  iot::TopicInfo* *ts = (iot::TopicInfo**) malloc( 8 * sizeof(iot::TopicInfo*));
  ts[0] = new iot::TopicSwitch("LED", switchOn, switchOff);

  ts[1] = new iot::TopicSwitch("RGB Led", RgbOn, RgbOff);

  iot::TopicOption* *tos = (iot::TopicOption**) malloc( 3 * sizeof(iot::TopicOption*));
  tos[0] = new iot::TopicOption("Red", rgb_red);
  tos[1] = new iot::TopicOption("Green", rgb_green);
  tos[2] = new iot::TopicOption("Blue", rgb_blue);
  ts[2] = new iot::TopicOptions("RGB Light", tos, 3);

  ts[3] = new iot::TopicSlider("RGB Red", set_red, 0, 255);
  ts[4] = new iot::TopicSlider("RGB Green", set_green, 0, 255);
  ts[5] = new iot::TopicSlider("RGB Blue", set_blue, 0, 255);

  iot::TopicOption* *tos2 = (iot::TopicOption**) malloc( 3 * sizeof(iot::TopicOption*));
  tos2[0] = new iot::TopicOption("Blink", randomColor);
  ts[6] = new iot::TopicOptions("Lights", tos2, 1);
  
  ts[7] = new iot::TopicSwitch("Play Mario Song", marioOn, marioOff);

  pthread_t marioThread;
  pthread_create(&marioThread, NULL, playMario, (void*)0);
  
  pinMode(2, OUTPUT);
  if(iot::startDevice("ESP32 Bitch Ples", ts, 8) == iot::DeviceState::RUNNING){
    blink();
  } else {
    Serial.println("Could not start device.");
  }

  pinMode(led, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(MARIO_PIN, OUTPUT);
  digitalWrite(MARIO_PIN, LOW);
  esp_random();

  randomColor("");
}

void loop() {
  iot::loop();
  delay(40);

  /*static const char* c = boci;
  playNote(*c, 300);

  if (*(++c) == 0)
    c = boci;*/
  // Serial.println(sizeof(mario)/8);
  /* for(int i = 0; i < sizeof(mario)/8; i++) {
    mario[i].play();
    iot::loop();
  } */
}
