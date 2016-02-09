/**
 * Remote Controls for your Eyebrows
 * Copyright (c) 2016 by Alec Smecher
 *
 * See LICENSE for licensing details.
 * 
 * Note to me: H-bridge wiring is...
 * 2  5
 * 4  3
 */

#include <IRLib.h>

// Pin configurations
int motor_pins[2][2] = {
  {3, 5}, //Motor 1
  {4, 2} // Motor 2
};
IRrecv irr(10);

void setup() {
  Serial.begin(9600);
  irr.enableIRIn(); // Start the receiver
  for (int i = 0; i < 2; i++) for (int j=0; j<2; j++) {
    pinMode(motor_pins[i][j], OUTPUT);
  }
}

// Remote control key hashes
#define VOLD 0xDF604FDC // Big left up calibrate
#define VOLU 0x20EE60D6 // Big right up calibrate
#define UP 0x738F2F1A // Both up
#define DOWN 0x24CA54E0 // Both down
#define CENTER 0x18313136 // Invert brows
#define LEFT 0x18B07C3E // Left up, right down
#define RIGHT 0xAC77E57C // Right up, left down
#define SRC 0x6F75E25A // Small left up calibrate
#define AUDIO 0xA7AC0E04  // Small right up calibrate
#define MUTE 0x6BB9B45A // Small left down calibrate
#define PGM 0xF3923B3E // Small right down calibrate
#define OFFHOOK 0xBD892ABE // Big left down calibrate
#define ONHOOK 0x51B5C084 // Big right down calibrate
#define BAND 0x57FFB47E // Make movements smaller
#define FUNC 0x4A7C3080 // Make movements bigger
#define ONE 0xB203D919 // Waggles
#define TWO 0x97738AB9 // Waggles
#define THREE 0x39B169B5 // Waggles
#define FOUR 0x57AFEA19 // Waggles
#define FIVE 0xFBDD28D5 // Waggles
#define SIX 0xAA02A07D // Waggles
#define SEVEN 0x4C407F79 // Waggles
#define EIGHT 0xD012B39 // Waggles
#define NINE 0xC97A06F5 // Waggles
#define DIRECT 0xC833AE95 // Left waggle selector
#define CLEAR 0x66236799 // Right waggle selector
#define ZERO 0x554D315 // Both waggle selector

const unsigned char BROW_LEFT=1;
const unsigned char BROW_RIGHT=2;
const unsigned char BROW_BOTH=BROW_LEFT | BROW_RIGHT;

const unsigned char DIR_STOP = 0;
const unsigned char DIR_UP = 1;
const unsigned char DIR_DOWN = 2;

unsigned char brow_status = 0;
unsigned char wagglers = BROW_LEFT | BROW_RIGHT;
unsigned int brow_delay = 1000;

IRdecodeHash ird;

void loop() {
  if (irr.GetResults(&ird)) {
        unsigned char target_status = brow_status;
        unsigned char count = 0;
        ird.decode();

	// Pass 1: Handle almost all the buttons
        switch(ird.hash) {
          case VOLD: drive(BROW_LEFT, DIR_UP); delay(500); drive(BROW_LEFT, DIR_STOP); break;
          case VOLU: drive(BROW_RIGHT, DIR_UP); delay(500); drive(BROW_RIGHT, DIR_STOP); break;
          case SRC: drive(BROW_LEFT, DIR_UP); delay(250); drive(BROW_LEFT, DIR_STOP); break;
          case AUDIO: drive(BROW_RIGHT, DIR_UP); delay(250); drive(BROW_RIGHT, DIR_STOP); break;
          case MUTE: drive(BROW_LEFT, DIR_DOWN); delay(250); drive(BROW_LEFT, DIR_STOP); break;
          case PGM: drive(BROW_RIGHT, DIR_DOWN); delay(250); drive(BROW_RIGHT, DIR_STOP); break;
          case OFFHOOK: drive(BROW_LEFT, DIR_DOWN); delay(500); drive(BROW_LEFT, DIR_STOP); break;
          case ONHOOK: drive(BROW_RIGHT, DIR_DOWN); delay(500); drive(BROW_RIGHT, DIR_STOP); break;
          case UP: target_status = BROW_LEFT | BROW_RIGHT; break;
          case DOWN: target_status = 0; break;
          case CENTER: target_status = (BROW_LEFT | BROW_RIGHT)-target_status; break;
          case LEFT: target_status = BROW_LEFT; break;
          case RIGHT: target_status = BROW_RIGHT; break;
          case BAND: target_status=0; break;
          case FUNC: target_status=0; break;
          case ONE: target_status=0; count=1; break;
          case TWO: target_status=0; count=2; break;
          case THREE: target_status=0; count=3; break;
          case FOUR: target_status=0; count=4; break;
          case FIVE: target_status=0; count=5; break;
          case SIX: target_status=0; count=6; break;
          case SEVEN: target_status=0; count=7; break;
          case EIGHT: target_status=0; count=8; break;
          case NINE: target_status=0; count=9; break;
          case DIRECT: wagglers = BROW_LEFT; break;
          case CLEAR: wagglers = BROW_RIGHT; break;
          case ZERO: wagglers = BROW_LEFT | BROW_RIGHT; break;
          default: Serial.println(ird.hash, HEX);
        }

	// Set the brows to the desired state
        if (brow_status != target_status) {
          if ((brow_status & BROW_LEFT) && !(target_status & BROW_LEFT)) drive(BROW_LEFT, DIR_DOWN);
          if ((brow_status & BROW_RIGHT) && !(target_status & BROW_RIGHT)) drive(BROW_RIGHT, DIR_DOWN);
          if (!(brow_status & BROW_LEFT) && (target_status & BROW_LEFT)) drive(BROW_LEFT, DIR_UP);
          if (!(brow_status & BROW_RIGHT) && (target_status & BROW_RIGHT)) drive(BROW_RIGHT, DIR_UP);
          delay(brow_delay);
          drive(BROW_LEFT | BROW_RIGHT, DIR_STOP);
          brow_status = target_status;
        }

	// Pass 2: For a few buttons, we need to do something after setting states
        switch (ird.hash) {
          case BAND: if (brow_delay > 100) brow_delay -= 100; break;
          case FUNC: if (brow_delay < 2000) brow_delay += 100; break;
        }

	// If any waggles are needed, perform them.
        while (count>0) {
          drive(wagglers, DIR_UP);
          delay(brow_delay);
          drive(wagglers, DIR_STOP);
          drive(wagglers, DIR_DOWN);
          delay(brow_delay);
          drive(wagglers, DIR_STOP);
          count--;
        }
        irr.resume();
      }
}

/**
 * Drive the brow motors.
 * brow: bitfield (see BROW_... constants)
 * direction: One of the DIR_... constants
 */
void drive(unsigned char brow, unsigned char dir) {
  if (brow & BROW_LEFT) {
    digitalWrite(motor_pins[0][0], dir == DIR_UP);
    digitalWrite(motor_pins[0][1], dir == DIR_DOWN);

  }
  if (brow & BROW_RIGHT) {
    digitalWrite(motor_pins[1][0], dir == DIR_UP);
    digitalWrite(motor_pins[1][1], dir == DIR_DOWN);
  }
}
