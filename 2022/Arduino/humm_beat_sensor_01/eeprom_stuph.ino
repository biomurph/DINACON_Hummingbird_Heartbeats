



int setRecordDuration(int durationSetting){
  int ds = durationSetting / 1000;
  if(ds > 31){
    Serial.println(F("We're not set up to record more than 30 seconds"));
    return recordDuration;
  }
  
  if(durationSetting == STARTUP_DURATION){
    if(EEPROM.read(DURATION_ADDRESS) == 255){
      ds = DEFAULT_RECORD_DURATION;
      durationSetting = 5000;
    } else {
      ds = EEPROM.read(DURATION_ADDRESS);
      durationSetting = ds * 1000;
    }
  }
    
    EEPROM.update(DURATION_ADDRESS, ds);
    if(ds != EEPROM.read(DURATION_ADDRESS)){
      Serial.println(F("Error: Record Duration Not Set In EEPROM"));
      return;
    }
    
    return durationSetting;
}
