/**
 *******************************
 *
 * REVISION HISTORY
 * WARNING: I use MySensors V1.6.0 (dev branch) (https://github.com/mysensors/Arduino/tree/development/libraries)
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
 * DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature (add battery level, id oregon sensor, type oregon sensor) 
 * sensor using Oregon sensor.
 * MySensors gateway <=======> Arduino UNO <--(PIN 2) --> 433Mhz receiver <=============> Oregon sensors
 */

// Enable debug prints
#define MY_DEBUG

#include <SPI.h>
#include <EEPROM.h>
#include <Oregon.h>

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
    if (p != 0)
    {
        if (orscV2.nextPulse(p))
        {      
            //Decode Hex Data once
            const byte* DataDecoded = DataToDecoder(orscV2);
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
