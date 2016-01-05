// Oregon V2 decoder modified for MySensors compatibility - Mickael Hubert <github@winlux.fr>
// Oregon V2 decoder modified - Olivier Lebrun
// Oregon V2 decoder added - Dominique Pierre
// New code to decode OOK signals from weather sensors, etc.
// 2010-04-11 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: ookDecoder.pde 5331 2010-04-17 10:45:17Z jcw $

#ifndef Oregon_h
#define Oregon_h
#include "Arduino.h"

/*--------------------------
Class DecodeOOK
--------------------------*/
class DecodeOOK
{
protected:
    byte total_bits, bits, flip, state, pos, data[25];

    virtual char decode (word width) =0;

public:

    enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };

    DecodeOOK () { resetDecoder(); }

    bool nextPulse (word width) {
        if (state != DONE)

            switch (decode(width)) {
                case -1: resetDecoder(); break;
                case 1:  done(); break;
            }
        return isDone();
    }

    bool isDone () const { return state == DONE; }

    const byte* getData (byte& count) const {
        count = pos;
        return data;
    }

    void resetDecoder () {
        total_bits = bits = pos = flip = 0;
        state = UNKNOWN;
    }

    // add one bit to the packet data buffer

    virtual void gotBit (char value) {
        total_bits++;
        byte *ptr = data + pos;
        *ptr = (*ptr >> 1) | (value << 7);

        if (++bits >= 8) {
            bits = 0;
            if (++pos >= sizeof data) {
                resetDecoder();
                return;
            }
        }
        state = OK;
    }

    // store a bit using Manchester encoding
    void manchester (char value) {
        flip ^= value; // manchester code, long pulse flips the bit
        gotBit(flip);
    }

    // move bits to the front so that all the bits are aligned to the end
    void alignTail (byte max =0) {
        // align bits
        if (bits != 0) {
            data[pos] >>= 8 - bits;
            for (byte i = 0; i < pos; ++i)
                data[i] = (data[i] >> bits) | (data[i+1] << (8 - bits));
            bits = 0;
        }
        // optionally shift bytes down if there are too many of 'em
        if (max > 0 && pos > max) {
            byte n = pos - max;
            pos = max;
            for (byte i = 0; i < pos; ++i)
                data[i] = data[i+n];
        }
    }

    void reverseBits () {
        for (byte i = 0; i < pos; ++i) {
            byte b = data[i];
            for (byte j = 0; j < 8; ++j) {
                data[i] = (data[i] << 1) | (b & 1);
                b >>= 1;
            }
        }
    }

    void reverseNibbles () {
        for (byte i = 0; i < pos; ++i)
            data[i] = (data[i] << 4) | (data[i] >> 4);
    }

    void done () {
        while (bits)
            gotBit(0); // padding
        state = DONE;
    }
};

/*--------------------------
Manipulation functions
--------------------------*/


volatile word pulse;

void ext_int_1(void)
{
    static word last;
    // determine the pulse length in microseconds, for either polarity
    pulse = micros() - last;
    last += pulse;
}

float temperature(const byte* data)
{
  int sign = (data[6]&0x8) ? -1 : 1;
  float temp = ((data[5]&0xF0) >> 4)*10 + (data[5]&0xF) + (float)(((data[4]&0xF0) >> 4) / 10.0);
  float result = sign * temp;
  #ifdef MY_DEBUG
    Serial.println("Oregon temperature: " + String(result));
  #endif
  return sign * temp;
}


byte humidity(const byte* data)
{
  byte humidity = (data[7]&0xF) * 10 + ((data[6]&0xF0) >> 4);
  #ifdef MY_DEBUG
    Serial.println("Oregon humidity: " + String(humidity));
  #endif
  return (humidity);
}

//10 => battery lovel is LOW
//90 => battery level is HIGH
byte battery(const byte* data)
{
  byte BatteryLevel = (data[4] & 0x4) ? 10 : 90;
  #ifdef MY_DEBUG
    Serial.println("Oregon battery level: " + String(BatteryLevel));
  #endif
  return BatteryLevel;
}

//Find id from Oregon Sensors
byte id(const byte* data)
{
  #ifdef MY_DEBUG
    Serial.println("Oregon ID: " + String(data[3]) + " Hexadecimal: " + (data[3],HEX));
  #endif
  return (data[3]);
}

//Find Channel from Oregon Sensors
byte channel(const byte* data)
{
    byte channel;
    switch (data[2])
    {
        case 0x10:
            channel = 1;
            break;
        case 0x20:
            channel = 2;
            break;
        case 0x40:
            channel = 3;
            break;
        default:
            channel = 0;
            break;
     }
     #ifdef MY_DEBUG
       Serial.println("Oregon channel: " + String(channel));
     #endif
     return channel;
}

// Detect type of sensor module
const char* OregonType (const byte* data)
{
    const char* Model;
    if(data[0] == 0xEA && data[1] == 0x4C)
    {
        Model = "THN132N";
    }
    else if(data[0] == 0x1A && data[1] == 0x2D)
    {
	       Model = "THGR228N";
    }
    else
    {
	      Model = "UNKNOWN";
    }
    #ifdef MY_DEBUG
      Serial.println("Oregon model: " + String(Model));
    #endif

    return Model;
}

// Decode data once
const byte* DataToDecoder (class DecodeOOK& decoder)
{
    byte pos;
    const byte* data = decoder.getData(pos);
    #ifdef MY_DEBUG
      Serial.println("Brute Hexadecimal data from sensor: ");
      for (byte i = 0; i < pos; ++i) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
      }
      Serial.println(' ');
    #endif
    decoder.resetDecoder();
    return data;
}

int FindSensor (const int id, int maxsensor)
{
  int i;
  bool find = false;
  for (i=0; i<maxsensor; i++){
    #ifdef MySensor_h
      int SensorID = loadState(i);
    #else
      int SensorID = EEPROM.read(i);
    #endif
    /*if(SensorID == 255)
    {
      #ifdef MySensor_h
        saveState(i,id);
      #else
        EEPROM.write(i,id);
      #endif

      #ifdef MY_DEBUG
        Serial.print("Sensor id: ");
        Serial.print(id);
        Serial.print(" has been saved in position EEPROM: ");
        Serial.println(i);
      #endif
      return i;
    }*/

    if(id == SensorID)
    {
      #ifdef MY_DEBUG
        Serial.print("Sensor id: ");
        Serial.print(SensorID);
        Serial.print(" has been find in position EEPROM: ");
        Serial.println(i);
      #endif
      find = true;
      return i;
    }
  }

    #ifdef MY_DEBUG
      Serial.print("I don't find ID to sensor ID: ");
      Serial.print(id);
      Serial.print(".");
      Serial.println("== Please check your configuration or active learning mode to add a new sensor ==");
    #endif
    return 256;
}

void SaveSensors (const int id, int maxsensor)
{
  #if LEARNING_MODE == true
    int y;
    int idfree;
    bool find = false;

    for (y=0; i<maxsensor; y++)
    {
      #ifdef MySensor_h
        int idfree = loadState(i);
      #else
        int idfree = EEPROM.read(i);
      #endif

      if(idfree == 255)
      {
        #ifdef MySensor_h
          saveState(i,id);
        #else
          EEPROM.write(i,id);
        #endif

        #ifdef MY_DEBUG
          Serial.print("Sensor id: ");
          Serial.print(id);
          Serial.print(" has been saved in position EEPROM: ");
          Serial.println(i);
        #endif
        find = true;
      }
    }

    if(find == false)
    {
      Serial.print("You have too much sensors (COUNT_OREGON_SENSORS = ");
      Serial.print(maxsensor);
      Serial.print(", I can't save ID: ");
      Serial.print(id);
      Serial.println(" in EEPROM");
    }
  #endif
  return;
}

bool isChecksumOK(class DecodeOOK& decoder) {
  int cs = 0;
  byte pos;
  const byte* data = decoder.getData(pos);

  for (byte i = 0; i < pos-2; ++i) {
      //all but last byte
      cs += data[i] >> 4;
      cs += data[i] & 0x0F;
   }
   cs -= 10;

   int csc = ((data[8] >> 4)*16) + (data[8] & 0x0F);
   return cs == csc;
}

void ResetEEPROM(int maxsensor)
{
  #if ERASE_REGISTERED_SENSORS == true
    Serial.println("Started clearing. Please wait...");
    for (int i=0;i<maxsensor;i++)
    {
      #ifdef MySensor_h
        saveState(i,0xff);
      #else
        EEPROM.write(i,0xff);
      #endif
    }
    Serial.println("Clering done. You're ready to go!");
  #endif
}

/*
// OWL Electricty Meter
  if (data[0] == 0x06 && data[1] == 0xC8) {
    //Serial.print("Current ");
    PowerNow = (data[3] + ((data[4] & 0x03)*256));
    //Serial.print(float(PowerNow)/10,1);
    //Serial.println("amps");
    int Duration = now.unixtime()-PreviousTime;
    float ActualPower = Duration * ((PowerNow * 240 * PF)/36000.0);
    if (ActualPower < 0) ActualPower = -ActualPower;
    if (ActualPower > 0) {
      TotalPowerHr += ActualPower; //W/s
      TotalPower24 += ActualPower; //W/s
      YearData.TotalPowerY += ActualPower;
      PreviousTime = now.unixtime();
      PowerTime = now.unixtime();
      //Serial.print("Added... ");
      //Serial.print(ActualPower);
      //Serial.print("W (over ");
    }
    //Serial.print(Duration);
    //Serial.print(" seconds). Total Today ");
    //Serial.print(TotalPower24,0);
    //Serial.println("W/hs ");
    //Check Extremes
    if (PowerNow > MaxPower24 && PowerNow != -999) {
      MaxPower24 = PowerNow;
    }
  }

  // WGR918 Annometer
  if (data[0] == 0x3A && data[1] == 0x0D) {
    //Checksum - add all nibbles from 0 to 8, subtract A and compare to byte 9, should = 0
    int cs = 0;
    for (byte i = 0; i < pos-1; ++i) { //all but last byte
        cs += data[i] >> 4;
        cs += data[i] & 0x0F;
    }
    int csc = ((data[9] >> 4)*16) + (data[9] & 0x0F);
    cs -= 10;
    //Serial.print(csc);
    //Serial.print(" vs ");
    //Serial.println(cs);
    if (cs == csc){ //if checksum is OK
      //Serial.print("Direction ");
      DirectionNow = ((data[5]>>4) * 100)  + ((data[5] & 0x0F) * 10) + (data[4] >> 4);
      //Serial.print(DirectionNow);
      //Serial.print(" degrees  Current Speed (Gust) ");
      GustNow = ((data[7] & 0x0F) * 100)  + ((data[6]>>4) * 10)  + ((data[6] & 0x0F)) ;
      //Serial.print(float(GustNow)/10,1);
      //Serial.print("m/s  Average Speed ");
      AverageNow = ((data[8]>>4) * 100)  + ((data[8] & 0x0F) * 10)+((data[7] >>4)) ;
      //Serial.print(float(AverageNow)/10,1);
      //Serial.print("m/s  Battery ");
      WindBat=(10-(data[4] & 0x0F))*10;
      //Serial.print(WindBat);
      //Serial.println("%");
      // Check Extremes
      WindTime = now.unixtime();
      if (GustNow > MaxGust24 && GustNow != -999) {
        MaxGust24 = GustNow;
        Direction24 = DirectionNow;
        //Serial.print("MaxGust today ");
        //Serial.println(MaxGust24);
      }
      if (MaxGust24 > YearData.MaxGustY && GustNow != -999) {
        YearData.MaxGustY = MaxGust24;
        YearData.DirectionY = Direction24;
        YearData.WindYD = now.unixtime();
        //Serial.print("MaxGust this Year ");
        //Serial.println(YearData.MaxGustY);
      }
      // Check Triggers
      if ((float(AverageNow)/10)*2.2369362920544025 > WindTrigger && WindTriggerFlag == false){
        WindTriggerFlag = true; //stops multiple emails for same excursion
        WindTriggerTime=now.unixtime() + 7200; //2 hours
        SendEmail(1);
      }
      if (((float(AverageNow)/10)*2.2369362920544025 < WindTrigger) && (now.unixtime() > WindTriggerTime )){
        WindTriggerFlag = false;
      }
    }
  }

  //RGR918 Rain Guage
  if (data[0] == 0x2A && data[1] == 0x1D) {
    //Checksum - add all nibbles from 0 to 8, subtract 9 and compare, should = 0
    //Serial.print(" - ");
    int cs = 0;
    for (byte i = 0; i < pos-2; ++i) { //all but last byte
        cs += data[i] >> 4;
        cs += data[i] & 0x0F;
    }
    int csc = (data[8] >> 4) + ((data[9] & 0x0F)*16);
    cs -= 9;  //my version as A fails?
    //Serial.print(csc);
    //Serial.print(" vs ");
    //Serial.println(cs);
    if (cs == csc){ //if checksum is OK
      //Serial.print("Rain ");
      RainRateNow = ((data[5]>>4) * 100)  + ((data[5] & 0x0F) * 10) + (data[4] >> 4);
      //Serial.print(RainRateNow);
      //Serial.print("mm/hr  Total ");
      int RainTotal = ((data[7]  >> 4) * 10)  + (data[6]>>4);
      RainTime = now.unixtime();
      if (RainTotal != OldRainTotal){
        if (RainNewFlag == false){  //Stops 1st reading going through and giving additonal values
          TotalRainFrom0000 += 1;
          TotalRainHour += 1;
          YearData.TotalRainY += 1;
          SendEmail(2);
        }
        OldRainTotal=RainTotal;
        RainNewFlag=false;
      }
      //Serial.print(TotalRainFrom0000);
      //Serial.print(" ");
      //Serial.print(RainTotal);
      //Serial.print(" ");
      //Serial.print(OldRainTotal);
      //Serial.print("mm  Battery ");
      if ((data[4] & 0x0F) >= 4){
        RainBat=0;
        //Serial.println("Low");
      }
      else
      {
        RainBat=100;
        //Serial.println("OK");
      }
    }
  }

  //UV138
  if (data[0] == 0xEA && data[1] == 0x7C) {
    //Serial.print("UV ");
    UVNow = ((data[5] & 0x0F) * 10)  + (data[4] >> 4);
    UVTime = now.unixtime();
    //Serial.print(UVNow);
    //Serial.print("  Battery ");
    if ((data[4] & 0x0F) >= 4){
      UVBat=0;
      //Serial.println("Low");
    }
    else
    {
      UVBat=100;
      //Serial.println("OK");
    }
    //Check Extremes
    if (UVNow > MaxUV24 && UVNow != -999) {
      MaxUV24 = UVNow;
    }
  }

  //THGR228N Inside Temp-Hygro
  if (data[0] == 0x1A && data[1] == 0x2D) {
    int battery=0;
    int celsius= ((data[5]>>4) * 100)  + ((data[5] & 0x0F) * 10) + ((data[4] >> 4));
    if ((data[6] & 0x0F) >= 8) celsius=-celsius;
    int hum = ((data[7] & 0x0F)*10)+ (data[6] >> 4);
    if ((data[4] & 0x0F) >= 4){
      battery=0;
    }
    else
    {
      battery=100;
    }
    //Serial.print("Additional Channel ");
    switch (data[2]) {
    case 0x10:
      CH1TempNow=celsius;
      CH1HumNow=hum;
      CH1Bat=battery;
      CH1Time = now.unixtime();
      //Serial.print("1  ");
      break;
    case 0x20:
      CH2TempNow=celsius;
      CH2HumNow=hum;
      CH2Bat=battery;
      CH2Time = now.unixtime();
      //Serial.print("2  ");
      break;
    case 0x40:
      CH3TempNow=celsius;
      CH3HumNow=hum;
      CH3Bat=battery;
      CH3Time = now.unixtime();
      //Serial.print("3  ");
      break;
    }
    //Serial.print(float(celsius)/10,1);
    //Serial.print("C  Humidity ");
    //Serial.print(hum);
    //Serial.print("%  Battery ");
    //Serial.print(battery);
    //Serial.println("%");
  }

  //THGR918  Outside Temp-Hygro
  if (data[0] == 0x1A && data[1] == 0x3D) {
    //Checksum - add all nibbles from 0 to 8, subtract 9 and compare, should = 0
    //Serial.print(" - ");
    int cs = 0;
    for (byte i = 0; i < pos-2; ++i) { //all but last byte
      cs += data[i] >> 4;
      cs += data[i] & 0x0F;
    }
    int csc = ((data[8] >> 4)*16) + (data[8] & 0x0F);
    cs -= 10;
    //Serial.print(csc);
    //Serial.print(" vs ");
    //Serial.println(cs);
    if (cs == csc){ //if checksum is OK
      //Serial.print("Outdoor temperature ");
      OutTempNow= ((data[5]>>4) * 100)  + ((data[5] & 0x0F) * 10) + ((data[4] >> 4));
      if ((data[6] & 0x0F) >= 8) OutTempNow=-OutTempNow;
      //Serial.print(float(OutTempNow)/10,1);
      //Serial.print("C  Humidity ");
      OutHumNow = ((data[7] & 0x0F)*10)+ (data[4] >> 4);
      //Serial.print(OutHumNow);
      //Serial.print("%  Battery ");
      OutTempBat=(10-(data[4] & 0x0F))*10;
      //Serial.print(OutTempBat);
      //Serial.println("%");
      OutTime = now.unixtime();
      //Check Extremes
      if (OutTempNow > MaxTemp24 && OutTempNow != -999) {
        MaxTemp24 = OutTempNow;
      }
      if (OutTempNow < MinTemp24 && OutTempNow != -999) {
        MinTemp24 = OutTempNow;
      }
      if (MaxTemp24 > YearData.MaxTempY && OutTempNow != -999) {
        YearData.MaxTempY = MaxTemp24;
        YearData.MaxTempYD = now.unixtime();
      }
      if (MinTemp24 < YearData.MinTempY && OutTempNow != -999) {
        YearData.MinTempY = MinTemp24;
        YearData.MinTempYD = now.unixtime();
      }
      // Check Triggers
      if (OutTempNow > TempTriggerMax && TempTriggerMaxFlag == false){
        TempTriggerMaxFlag = true; //stops multiple emails for same excursion
        TempTriggerTime=now.unixtime() + 7200; //2 hours
        SendEmail(3);
      }
      if ((OutTempNow < TempTriggerMax) && (now.unixtime() > TempTriggerTime)){
        TempTriggerMaxFlag = false;
      }
      if (OutTempNow < TempTriggerMin && TempTriggerMinFlag == false){
        TempTriggerMinFlag = true; //stops multiple emails for same excursion
        TempTriggerTime=now.unixtime() + 7200;  //2 hours
        SendEmail(4);
      }
      if ((OutTempNow > TempTriggerMin)&& (now.unixtime() > TempTriggerTime)){
        TempTriggerMinFlag = false;
      }
    }
  }


  //BTHR918 Temp-Hygro-Baro
  if (data[0] == 0x5A && data[1] == 0x6D) {
    //Serial.print("Indoor temperature ");
    InTempNow= ((data[5]>>4) * 100)  + ((data[5] & 0x0F) * 10) + ((data[4] >> 4));
    if ((data[6] & 0x0F) >= 8) InTempNow=-InTempNow;
    //Serial.print(float(InTempNow)/10,1);
    //Serial.print("C  Humidity ");
    InHumNow = ((data[7] & 0x0F)*10)+ (data[6] >> 4);
    //Serial.print(InHumNow);
    //Serial.print("%  Pressure ");
    BarrometerNow = (data[8])+856;
    //Serial.print(BarrometerNow);
    //Serial.print("hPa  ");
    switch (data[7] & 0xC0) {
    case 0x00:
      Comfort="Normal";
      break;
    case 0x40:
      Comfort="Comfortable";
      break;
    case 0x80:
      Comfort="Dry";
      break;
    case 0xC0:
      Comfort="Wet";
      break;
    }
    //Serial.print(Comfort);
    //Serial.print("  ");
    switch (data[9] >> 4) {
    case 0x0C:
      Forecast="Sunny";
      break;
    case 0x06:
      Forecast="Partly Cloudy";
      break;
    case 0x02:
      Forecast="Cloudy";
      break;
    case 0x03:
      Forecast="Wet";
      break;
    }
    InTime = now.unixtime();
    //Serial.print(Forecast);
    //Serial.print("  Battery ");
    InTempBat=(10-(data[4] & 0x0F))*10;
    //Serial.print(InTempBat);
    //Serial.println("%");
  }



*/

#endif
