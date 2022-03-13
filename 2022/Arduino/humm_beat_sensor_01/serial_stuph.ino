



void serialCheck(){
  if(Serial.available()) {
    char inChar = Serial.read();
    Serial.println();
    switch(inChar){
      // ignore endings and beginnings
      case '\n': case '\r': case '\t': break;
      case 'b':
        openBinFile(); break;
      case 'c':
        if (createCsvFile()) {
          binaryToCsv();
        }
        break;
      case 'l':
        Serial.print(F("ls: ")); digitalClockDisplay();
        sd.ls(&Serial, LS_DATE | LS_SIZE);
        printUnusedStack();
        break;
      case 'p':
        printData(); break;
      case 'r':
        isRunning = true; fadeLED(millis());
        break;
      case 't':
        testSensor(); break;
      case '1':
        recordDuration = setRecordDuration(5000); 
        printRecordDruation();
        break;
      case '2':
        recordDuration = setRecordDuration(10000);  
        printRecordDruation();
        break;
      case '3':
        recordDuration = setRecordDuration(15000); 
        printRecordDruation();
        break;
      case '4':
        recordDuration = setRecordDuration(20000); 
        printRecordDruation();
        break;
      case '?':
        printControl(); break;
      default:
        Serial.print("'"); Serial.print(inChar); Serial.print("'");
        Serial.println(F(" is not a command")); 
        Serial.println(F("send '?' to list commands")); 
//        printControl(); // need this here?
        break;
    }
  }
}

void printRecordDruation(){
  Serial.print(F("current record duration: "));
  Serial.println(recordDuration/1000);
}
void printControl(){
  if (ERROR_LED_PIN >= 0) {
    digitalWrite(ERROR_LED_PIN, LOW);
  }
  Serial.println();
  Serial.println(F("type: "));
  Serial.println(F("b - open existing bin file"));
  Serial.println(F("c - convert file to csv"));
  Serial.println(F("l - list files"));
  Serial.println(F("p - print data to Serial"));
  Serial.println(F("r - record data"));
  Serial.println(F("t - test without logging"));
  Serial.println(F("? - print this message"));
  Serial.println(F("type a number to set record duration"));
  Serial.println(F("\t 1 - 5 seconds"));
  Serial.println(F("\t 2 - 10 seconds"));
  Serial.println(F("\t 3 - 15 seconds"));
  Serial.println(F("\t 4 - 20 seconds"));
  printRecordDruation();
}

//------------------------------------------------------------------------------
bool serialReadLine(char* str, size_t size) {
  size_t n = 0;
  while(!Serial.available()) {
    yield();
  }
  while (true) {
    int c = Serial.read();
    if (c < ' ') break;
    str[n++] = c;
    if (n >= size) {
      Serial.println(F("input too long"));
      return false;
    }
    uint32_t m = millis();
    while (!Serial.available() && (millis() - m) < 100){}
    if (!Serial.available()) break;
  }
  str[n] = 0;
  return true;
}

//-----------------------------------------------------------------------------
void printData() {
  if (!binFile.isOpen()) {
    Serial.println(F("No current binary file"));
    return;
  }
  // Skip first dummy sector.
  if (!binFile.seekSet(512)) {
    error("seek failed");
  }
  clearSerialInput();
  Serial.println(F("type any character to stop\n"));
  delay(1000);
  printRecord(&Serial, nullptr);
  while (binFile.available() && !Serial.available()) {
    data_t record;
    if (binFile.read(&record, sizeof(data_t)) != sizeof(data_t)) {
      error("read binFile failed");
    }
    printRecord(&Serial, &record);
  }
}

//------------------------------------------------------------------------------
void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}
//------------------------------------------------------------------------------
