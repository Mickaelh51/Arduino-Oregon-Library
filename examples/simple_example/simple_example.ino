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

void setup ()
{

  Serial.println("Setup started");

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
            //Decode Hex Data once
            DataDecoded = DataToDecoder(orscV1);
        if (orscV2.nextPulse(p))
            //Decode Hex Data once
            DataDecoded = DataToDecoder(orscV2);
        if (orscV3.nextPulse(p))
            //Decode Hex Data once
            DataDecoded = DataToDecoder(orscV3);

        if(DataDecoded)
        {
            //Find or save Oregon sensors's ID
            int SensorID = FindSensor(id(DataDecoded),COUNT_OREGON_SENSORS);

            // just for DEBUG
            OregonType(DataDecoded);
            channel(DataDecoded);
            temperature(DataDecoded);
            humidity(DataDecoded);
            battery(DataDecoded);
        }

    }
}
