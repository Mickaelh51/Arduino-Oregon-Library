/**
 *******************************
 *
 * Version 1.0 - Hubert Mickael <mickael@winlux.fr> (https://github.com/Mickaelh51)
 *  - Clean ino code
 *  - Add MY_DEBUG mode in library
 * Version 0.2 (Beta 2) - Hubert Mickael <mickael@winlux.fr> (https://github.com/Mickaelh51)
 *  - Auto detect Oregon 433Mhz
 *  - Add battery level
 *  - etc ...
 * Version 0.1 (Beta 1) - Hubert Mickael <mickael@winlux.fr> (https://github.com/Mickaelh51)
 *
 *******************************
 * DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature from Oregon sensor.
 * - Oregon sensor's battery level
 * - Oregon sensor's id
 * - Oregon sensor's type
 * - Oregon sensor's channel
 * - Oregon sensor's temperature
 * - Oregon sensor's humidity
 *
 * Arduino UNO <-- (PIN 2) --> 433Mhz receiver <=============> Oregon sensors
 */

// Enable debug prints
#define MY_DEBUG

#include <SPI.h>
#include <EEPROM.h>
#include <Oregon.h>
#include <OregonDecoderV1.h>
#include <OregonDecoderV2.h>
#include <OregonDecoderV3.h>

OregonDecoderV1 orscV1;
OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;

//Define pin where is 433Mhz receiver (here, pin 2)
#define MHZ_RECEIVER_PIN 2
//Define maximum Oregon sensors (here, 3 differents sensors)
#define COUNT_OREGON_SENSORS 3
//Active learning mode when you want add new sensor
#define LEARNING_MODE false
//Active erase mode when you want to delete Oregon's IDs
#define ERASE_REGISTERED_SENSORS false

void setup ()
{
  Serial.println("Setup started");

  //if ERASE_REGISTERED_SENSORS is true, we erase sensor data in EEPROM (write 0xff)
  ResetEEPROM(COUNT_OREGON_SENSORS);

  //Setup received data
  attachInterrupt(digitalPinToInterrupt(MHZ_RECEIVER_PIN), ext_int_1, CHANGE);

  Serial.println("Setup completed");
}


void loop () {
    //------------------------------------------
    //Start process new data from Oregon sensors
    //------------------------------------------
    cli();
    word p = pulse;
    pulse = 0;
    sei();
    const byte* DataDecoded;
    if (p != 0)
    {
        if (orscV1.nextPulse(p))
            //Decode version 1 hexadecimal data
            if(isChecksumOK(orscV1)) { DataDecoded = DataToDecoder(orscV1); }
        if (orscV2.nextPulse(p))
            //Decode version 2 hexadecimal data
            if(isChecksumOK(orscV2)) { DataDecoded = DataToDecoder(orscV2); }
        if (orscV3.nextPulse(p))
            //Decode version 3 hexadecimal data
            if(isChecksumOK(orscV3)) { DataDecoded = DataToDecoder(orscV3); }

        if(DataDecoded)
        {
            //Active learning mode to save news sensors in EEPROM
            SaveSensors(id(DataDecoded),COUNT_OREGON_SENSORS);
            //Find or save Oregon sensors's ID
            int SensorID = FindSensor(id(DataDecoded),COUNT_OREGON_SENSORS);

            if(SensorID < 255)
            {
              // just for DEBUG
              OregonType(DataDecoded);
              channel(DataDecoded);
              temperature(DataDecoded);
              humidity(DataDecoded);
              battery(DataDecoded);
            }
        }

    }
}
