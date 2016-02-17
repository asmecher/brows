/**
 * Remote Controls for your Eyebrows
 * Copyright (c) 2016 by Alec Smecher
 *
 * See LICENSE for licensing details.
 * See http://www.cassettepunk.com for more information.
 */

#include <IRLib.h>

/**
 * Pin configurations
 */
int motor_pins[2][2] = {
  {3, 5}, // Motor 1 outputs to H-bridge on these pins
  {4, 2}  // Motor 2 outputs to H-bridge on these pins
};
IRrecv irr(10); // IR receiver data comes in on this pin


// Bitfield for identifying brow states
const unsigned char BROW_LEFT = 0x01;
const unsigned char BROW_RIGHT = 0x02;
const unsigned char BROW_BOTH = BROW_LEFT | BROW_RIGHT;

// Constants for movement directions
const unsigned char DIR_STOP = 0;
const unsigned char DIR_UP = 1;
const unsigned char DIR_DOWN = 2;

unsigned char brow_status = 0; // Which brows are up (BROW_... bitfield)
unsigned char wagglers = BROW_LEFT | BROW_RIGHT; // Which brows are selected for waggling
unsigned int brow_delay = 1000; // Size of movement

// Amount of time to let the motor run for when calibrating
const unsigned int CALIBRATE_LARGE = 500;
const unsigned int CALIBRATE_SMALL = 250;

IRdecodeHash ird;


void setup() {
  Serial.begin(9600); // May be used for debugging when breadboarded on the Arduino
  irr.enableIRIn(); // Start the receiver

  // Set the pin modes for the H-bridge outputs
  for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
      pinMode(motor_pins[i][j], OUTPUT);
    }
}


void loop() {
  if (irr.GetResults(&ird)) {
    // A button was pressed. Do something.
    unsigned char target_status = brow_status; // Track changes in the desired status
    unsigned char count = 0; // Waggle counter
    ird.decode();

    // Pass 1: Handle almost all the buttons
    switch (ird.hash) {

      // Calibration buttons
      case 0xDF604FDC: // Left brow, large up
        drive(BROW_LEFT, DIR_UP);
        delay(CALIBRATE_LARGE);
        drive(BROW_LEFT, DIR_STOP);
        break;
      case 0x20EE60D6: // Right brow, large up
        drive(BROW_RIGHT, DIR_UP);
        delay(CALIBRATE_LARGE);
        drive(BROW_RIGHT, DIR_STOP);
        break;
      case 0x6F75E25A: // Left brow, small up
        drive(BROW_LEFT, DIR_UP);
        delay(CALIBRATE_SMALL);
        drive(BROW_LEFT, DIR_STOP);
        break;
      case 0xA7AC0E04: // Right brow, small up
        drive(BROW_RIGHT, DIR_UP);
        delay(CALIBRATE_SMALL);
        drive(BROW_RIGHT, DIR_STOP);
        break;
      case 0x6BB9B45A: // Left brow, small down
        drive(BROW_LEFT, DIR_DOWN);
        delay(CALIBRATE_SMALL);
        drive(BROW_LEFT, DIR_STOP);
        break;
      case 0xF3923B3E: // Right brow, small down
        drive(BROW_RIGHT, DIR_DOWN);
        delay(CALIBRATE_SMALL);
        drive(BROW_RIGHT, DIR_STOP);
        break;
      case 0xBD892ABE: // Left brow, big down
        drive(BROW_LEFT, DIR_DOWN);
        delay(CALIBRATE_LARGE);
        drive(BROW_LEFT, DIR_STOP);
        break;
      case 0x51B5C084: // Right brow, big down
        drive(BROW_RIGHT, DIR_DOWN);
        delay(CALIBRATE_LARGE);
        drive(BROW_RIGHT, DIR_STOP);
        break;

      // Direct selection of brow states
      case 0x738F2F1A: // Both brows up
        target_status = BROW_LEFT | BROW_RIGHT;
        break;
      case 0x24CA54E0: // Both brows down
        target_status = 0;
        break;
      case 0x18313136: // Invert brows
        target_status = (BROW_LEFT | BROW_RIGHT) - target_status;
        break;
      case 0x18B07C3E: // Left up, right down
        target_status = BROW_LEFT;
        break;
      case 0xAC77E57C: // Right up, left down
        target_status = BROW_RIGHT;
        break;

      // Adjust the movement size
      case 0x57FFB47E: // Make movements smaller
        target_status = 0;
        break;
      case 0x4A7C3080: // Make movements bigger
        target_status = 0;
        break;

      // Waggler functions
      case 0xB203D919: target_status = 0; count = 1; break; // 1 waggle
      case 0x97738AB9: target_status = 0; count = 2; break; // 2 waggles
      case 0x39B169B5: target_status = 0; count = 3; break; // 3 waggles
      case 0x57AFEA19: target_status = 0; count = 4; break; // 4 waggles
      case 0xFBDD28D5: target_status = 0; count = 5; break; // 5 waggles
      case 0xAA02A07D: target_status = 0; count = 6; break; // 6 waggles
      case 0x4C407F79: target_status = 0; count = 7; break; // 7 waggles
      case 0xD012B39: target_status = 0; count = 8; break; // 8 waggles
      case 0xC97A06F5: target_status = 0; count = 9; break; // 9 waggles
      case 0xC833AE95: wagglers = BROW_LEFT; break; // Choose left brow for waggling
      case 0x66236799: wagglers = BROW_RIGHT; break; // Choose right brow for waggling
      case 0x554D315: wagglers = BROW_LEFT | BROW_RIGHT; break; // Choose both brows for waggling

      // Dump unknown presses to the serial port
      default: Serial.println(ird.hash, HEX);
    }

    // If a brow state change was flagged, set the brows to the desired state
    if (brow_status != target_status) {
      if ((brow_status & BROW_LEFT) && !(target_status & BROW_LEFT)) drive(BROW_LEFT, DIR_DOWN);
      if ((brow_status & BROW_RIGHT) && !(target_status & BROW_RIGHT)) drive(BROW_RIGHT, DIR_DOWN);
      if (!(brow_status & BROW_LEFT) && (target_status & BROW_LEFT)) drive(BROW_LEFT, DIR_UP);
      if (!(brow_status & BROW_RIGHT) && (target_status & BROW_RIGHT)) drive(BROW_RIGHT, DIR_UP);
      delay(brow_delay);
      drive(BROW_LEFT | BROW_RIGHT, DIR_STOP);
      brow_status = target_status;
    }

    // Pass 2: For a few buttons, we need to do something after setting states.
    switch (ird.hash) {
      // Make movements smaller
      case 0x57FFB47E: if (brow_delay > 100) brow_delay -= 100; break;
      // Make movements bigger
      case 0x4A7C3080: if (brow_delay < 2000) brow_delay += 100; break;
    }

    // If any waggles are needed, perform them.
    while (count > 0) {
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
 * Drive the brow motors via the H-bridge.
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
