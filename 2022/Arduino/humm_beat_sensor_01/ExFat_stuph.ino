//==============================================================================


// Replace logRecord(), printRecord(), and ExFatLogger.h for your sensors.
int logRecord(data_t* data, uint16_t overrun) {
  if (overrun) {
    // Add one since this record has no adc data. Could add overrun field.
    overrun++;
    data->adc[0] = 0X8000 | overrun;
    return -1;
  } else {
    timeIn = micros();
    for(size_t i=0; i<ADC_COUNT; i++) {
      data->adc[i] = analogRead(i);
    }
  }
  timeOut = micros() - timeIn;
  return timeOut;
}
//------------------------------------------------------------------------------
void printRecord(Print* pr, data_t* data) {
  static uint8_t nr = 0;
  if (!data) {
    pr->print(F("%LOG_INTERVAL_USEC,"));
    pr->println(LOG_INTERVAL_USEC);
    pr->print(F("%FILE_NAME,")); pr->println(binName);
    pr->print(F("%RECORD_DURATION")); pr->println(recordDuration/1000);
    pr->print(F("%Sample_Counter"));
    for (size_t i = 0; i < ADC_COUNT; i++) {
      pr->print(getChannelString(i));
    }
    pr->println();
    nr = 0;
    return;
  }
  if (data->adc[0] & 0x8000) {
    uint16_t n = data->adc[0] & 0x7FFF;
    nr += n;
    pr->print(F("-1,"));
    pr->print(n);
    pr->println(F(",overuns"));
  } else {
    pr->print(nr++);
    for (size_t i = 0; i < ADC_COUNT; i++) {
      pr->write(',');
      pr->print(data->adc[i]);
    }
    pr->println();
  }
}

//==============================================================================
String getChannelString(int i){
  String s = "";
  switch(i){
    case 0:
      s = F(",Pulse_RED");
      break;
    case 1:
      s = F(",Pulse_BLUE");
      break;
    case 2:
      s = F(",Accel_X");
      break;
    case 3:
      s = F(",Accel_Y");
      break;
    case 4:
      s = F(",Accel_Z");
      break;
    default: 
      s = F(",Channel Name Error");
      break;
  }
  return s;
}

//==============================================================================

void openBinFile() {
  char name[FILE_NAME_DIM];
  clearSerialInput();
  Serial.println(F("Enter file name"));
  if (!serialReadLine(name, sizeof(name))) {
    return;
  }
  if (!sd.exists(name)) {
    Serial.println(name);
    Serial.println(F("File does not exist"));
    return;
  }
  binFile.close();
  if (!binFile.open(name, O_RDONLY)) {
    Serial.println(name);
    Serial.println(F("open failed"));
    return;
  }
  Serial.print(name);
  Serial.println(F("\tFile opened"));
}

//-------------------------------------------------------------------------------
void logData() {
  int32_t delta;  // Jitter in log time.
  int32_t maxDelta = 0;
  uint32_t maxLogMicros = 0;
  uint32_t maxWriteMicros = 0;
  uint32_t analogReadMicros;
  uint32_t maxAnalogReadMicros = 0;
  size_t maxFifoUse = 0;
  size_t fifoCount = 0;
  size_t fifoHead = 0;
  size_t fifoTail = 0;
  uint16_t overrun = 0;
  uint16_t maxOverrun = 0;
  uint32_t totalOverrun = 0;
  uint32_t fifoBuf[128*FIFO_SIZE_SECTORS];
  data_t* fifoData = (data_t*)fifoBuf;

  // Write dummy sector to start multi-block write.
  dbgAssert(sizeof(fifoBuf) >= 512);
  memset(fifoBuf, 0, sizeof(fifoBuf));
  if (binFile.write(fifoBuf, 512) != 512) {
    error("write first sector failed");
  }
  clearSerialInput();
  Serial.println(F("Type any character to stop"));

  // Wait until SD is not busy.
  while (sd.card()->isBusy()) {}

  // Start time for log file.
  uint32_t m = millis();

  // Time to log next record.
  uint32_t logTime = micros();
  // Here's where it locks up
  while (true) {
    digitalWrite(RED_LED,HIGH);
    analogWrite(BLU_LED,0);
    // Time for next data record.
    logTime += LOG_INTERVAL_USEC;
    sampleClockPinState = !sampleClockPinState;
    digitalWrite(SAMPLE_CLK_PIN,sampleClockPinState);
    // Wait until time to log data.
    delta = micros() - logTime;
    if (delta > 0) {
      Serial.print(F("delta: "));
      Serial.println(delta);
      error("Rate too fast");
    }
    while (delta < 0) {   // this is where the delay happens
      delta = micros() - logTime;
    }

    if (fifoCount < FIFO_DIM) {
      uint32_t m = micros();
      analogReadMicros = logRecord(fifoData + fifoHead, overrun);
      if(analogReadMicros > maxAnalogReadMicros){ maxAnalogReadMicros = analogReadMicros; }
      m = micros() - m;
      if (m > maxLogMicros) {
        maxLogMicros = m;
      }
      fifoHead = fifoHead < (FIFO_DIM - 1) ? fifoHead + 1 : 0;
      fifoCount++;
      if (overrun) {
        if (overrun > maxOverrun) {
          maxOverrun = overrun;
        }
        overrun = 0;
      }
    } else {
      totalOverrun++;
      overrun++;
      if (overrun > 0XFFF) {
        error("too many overruns");
      }
      if (ERROR_LED_PIN >= 0) {
        digitalWrite(ERROR_LED_PIN, HIGH);
      }
    }
    // Save max jitter.
    if (delta > maxDelta) {
      maxDelta = delta;
    }
    // Write data if SD is not busy.
    if (!sd.card()->isBusy()) {
      size_t nw = fifoHead > fifoTail ? fifoCount : FIFO_DIM - fifoTail;
      // Limit write time by not writing more than 512 bytes.
      const size_t MAX_WRITE = 512/sizeof(data_t);
      if (nw > MAX_WRITE) nw = MAX_WRITE;
      size_t nb = nw*sizeof(data_t);
      uint32_t usec = micros();
      if (nb != binFile.write(fifoData + fifoTail, nb)) {
        error("write binFile failed");
      }
      usec = micros() - usec;
      if (usec > maxWriteMicros) {
        maxWriteMicros = usec;
      }
      fifoTail = (fifoTail + nw) < FIFO_DIM ? fifoTail + nw : 0;
      if (fifoCount > maxFifoUse) {
        maxFifoUse = fifoCount;
      }
      fifoCount -= nw;
      // escape routines
      // vvvvvvvvvvvvvvv
      if (Serial.available()) { break; }
////     if (getButtonState() != RECORD_ON) { break; } // DO NOT USE needs reliable fast switch to impliment 
// recordDuration set in EEPROM or update via serial port
      if (millis() - m >= recordDuration) { break; }
      // ^^^^^^^^^^^^^^^
      // escape routines
    }
  }
  Serial.print(F("\nLog time Seconds: ")); Serial.println(0.001*(millis() - m));
  Serial.print(F("File size: "));
  Serial.print((uint32_t)binFile.fileSize()/1000); Serial.println(F(" Kbytes"));
  Serial.print(F("totalOverrun: ")); Serial.println(totalOverrun);
  Serial.print(F("FIFO_DIM: ")); Serial.println(FIFO_DIM);
  Serial.print(F("maxFifoUse: ")); Serial.println(maxFifoUse);
  Serial.print(F("maxLogMicros: ")); Serial.println(maxLogMicros);
  Serial.print(F("maxWriteMicros: ")); Serial.println(maxWriteMicros);
  Serial.print(F("Log interval micros: ")); Serial.println(LOG_INTERVAL_USEC);
  Serial.print(F("maxDelta micros: ")); Serial.println(maxDelta);
  Serial.print(F("maxAnalogReadMicros: ")); Serial.println(maxAnalogReadMicros);
  binFile.truncate();
  binFile.sync();
}

//-------------------------------------------------------------------------------
bool createCsvFile() {
  char csvName[FILE_NAME_DIM];
  if (!binFile.isOpen()) {
    Serial.println(F("No current binary file"));
    return false;
  }

  // Create a new csvFile.
  binFile.getName(csvName, sizeof(csvName));
  char* dot = strchr(csvName, '.');
  if (!dot) {
    error("no dot in filename");
  }
  strcpy(dot + 1, "csv");
  if (!csvFile.open(csvName, O_WRONLY | O_CREAT | O_TRUNC)) {
    error("open csvFile failed");
  }
  clearSerialInput();
  Serial.print(F("Writing: "));
  Serial.print(csvName);
  Serial.println(F(" - type any character to stop"));
  return true;
}

// Convert binary file to csv file.
void binaryToCsv() {
  uint8_t lastPct = 0;
  uint32_t t0 = millis();
  data_t binData[FIFO_DIM];

  if (!binFile.seekSet(512)) {
    error("binFile.seek failed");
  }
  uint32_t tPct = millis();
  printRecord(&csvFile, nullptr);
  while (!Serial.available() && binFile.available()) {
    int nb = binFile.read(binData, sizeof(binData));
    if (nb <= 0 ) {
      error("read binFile failed");
    }
    size_t nr = nb/sizeof(data_t);
    for (size_t i = 0; i < nr; i++) {
      printRecord(&csvFile, &binData[i]);
    }

    if ((millis() - tPct) > 1000) {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
        Serial.print(pct, DEC);
        Serial.println('%');
        csvFile.sync();
      }
    }
    if (Serial.available()) {
      break;
    }
  }
  csvFile.close();
  Serial.print(F("Done: "));
  Serial.print(0.001*(millis() - t0));
  Serial.println(F(" Seconds"));
}

//-------------------------------------------------------------------------------
void createBinFile() {
  binFile.close();
  while (sd.exists(binName)) {
    char* p = strchr(binName, '.');
    if (!p) {
      error("no dot in filename");
    }
    while (true) {
      p--;
      if (p < binName || *p < '0' || *p > '9') {
        error("Can't create file name");
      }
      if (p[0] != '9') {
        p[0]++;
        break;
      }
      p[0] = '0';
    }
  }
  if (!binFile.open(binName, O_RDWR | O_CREAT)) {
    error("open binName failed");
  }
  Serial.print(F("recording to file: "));
  Serial.println(binName);
  if (!binFile.preAllocate(PREALLOCATE_SIZE)) {
    error("preAllocate failed");
  }

  Serial.print(F("preAllocated: "));
  Serial.print(PREALLOCATE_SIZE_MiB);
  Serial.println(F(" MiB"));
  Serial.print(F("record duration: "));
  Serial.print(recordDuration/1000);
  Serial.println(F(" Seconds"));
}


void printUnusedStack() {
#if HAS_UNUSED_STACK
  Serial.print(F("\nUnused stack: "));
  Serial.println(UnusedStack());
#endif  // HAS_UNUSED_STACK
}
