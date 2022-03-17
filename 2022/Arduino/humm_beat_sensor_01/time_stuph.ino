
void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(F(" "));
  Serial.print(day());
  Serial.print(F(" "));
  Serial.print(month());
  Serial.print(F(" "));
  Serial.print(year()); 
  Serial.println(); 
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}



void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(F(":"));
  if(digits < 10)
    Serial.print(F("0"));
  Serial.print(digits);
}

// callback for file time stamp

void dateTime(uint16_t* d, uint16_t* t, uint8_t* ms10) {
//  DateTime now = rtc.now();

  // Return date using FS_DATE macro to format fields.
  *d = FS_DATE(year(), month(), day());

  // Return time using FS_TIME macro to format fields.
  *t = FS_TIME(hour(), minute(), second());

  // Return low time bits in units of 10 ms.
    *ms10 = second() & 1 ? 100 : 0;
}
