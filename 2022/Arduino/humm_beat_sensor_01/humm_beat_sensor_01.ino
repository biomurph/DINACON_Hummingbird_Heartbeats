/*
 *    this code targeted to Teensy 3.6
 *    use for baseline record capability and data retrieval
 *    set to highest ADC resolution
 *    
 */


// Example to demonstrate write latency for preallocated exFAT files.
// I suggest you write a PC program to convert very large bin files.
//
// The maximum data rate will depend on the quality of your SD,
// the size of the FIFO, and using dedicated SPI.
#include "SdFat.h"
#include "FreeStack.h"
#include "HummBird.h"



//==============================================================================
// Replace logRecord(), printRecord(), and ExFatLogger.h for your sensors.
void logRecord(data_t* data, uint16_t overrun) {
  if (overrun) {
    // Add one since this record has no adc data. Could add overrun field.
    overrun++;
    data->adc[0] = 0X8000 | overrun;
  } else {
    for (size_t i = 0; i < ADC_COUNT; i++) {
      data->adc[i] = analogRead(i);
    }
  }
}
//------------------------------------------------------------------------------
void printRecord(Print* pr, data_t* data) {
  static uint32_t nr = 0;
  if (!data) {
    pr->print(F("LOG_INTERVAL_USEC,"));
    pr->println(LOG_INTERVAL_USEC);
    pr->print(F("Sample Counter"));
    for (size_t i = 0; i < ADC_COUNT; i++) {
//      pr->print(F(",channel"));
//      pr->print(i);
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
      s = F(",Pulse 1");
      break;
    case 1:
      s = F(",Pulse 2");
      break;
    case 2:
      s = F(",Accel X");
      break;
    case 3:
      s = F(",Accel Y");
      break;
    case 4:
      s = F(",Accel Z");
      break;
    default: 
      s = F(",Channel Name Error");
      break;
  }
  return s;
}

//==============================================================================
const uint64_t PREALLOCATE_SIZE  =  (uint64_t)PREALLOCATE_SIZE_MiB << 20;
// Max length of file name including zero byte.
#define FILE_NAME_DIM 40
// Max number of records to buffer while SD is busy.
const size_t FIFO_DIM = 512*FIFO_SIZE_SECTORS/sizeof(data_t);

#if SD_FAT_TYPE == 0
typedef SdFat sd_t;
typedef File file_t;
#elif SD_FAT_TYPE == 1
typedef SdFat32 sd_t;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
typedef SdExFat sd_t;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
typedef SdFs sd_t;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

sd_t sd;

file_t binFile;
file_t csvFile;
// You may modify the filename.  Digits before the dot are file versions.
char binName[] = "Humm_00.bin";
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#define error(s) sd.errorHalt(&Serial, F(s))
#define dbgAssert(e) ((e) ? (void)0 : error("assert " #e))
//-----------------------------------------------------------------------------






//------------------------------------------------------------------------------
void printUnusedStack() {
#if HAS_UNUSED_STACK
  Serial.print(F("\nUnused stack: "));
  Serial.println(UnusedStack());
#endif  // HAS_UNUSED_STACK
}

//------------------------------------------------------------------------------
void testSensor() {
  const uint32_t interval = 200000;
  int32_t diff;
  data_t data;
  clearSerialInput();
  Serial.println(F("\nTesting - type any character to stop\n"));
  delay(1000);
  printRecord(&Serial, nullptr);
  uint32_t m = micros();
  while (!Serial.available()) {
    m += interval;
    do {
      diff = m - micros();
    } while (diff > 0);
    logRecord(&data, 0);
    printRecord(&Serial, &data);
  }
}
//------------------------------------------------------------------------------
void setup() {
  if (ERROR_LED_PIN >= 0) {
    pinMode(ERROR_LED_PIN, OUTPUT);
    digitalWrite(ERROR_LED_PIN, HIGH);
  }
  pinMode(LED_PIN,OUTPUT); analogWrite(LED_PIN,255);
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  buttonValue = lastButtonValue = digitalRead(BUTTON_PIN);

  
  Serial.begin(2000000);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  delay(1000);
  Serial.println(F("Hummingbird Beats v1"));
//  while (!Serial.available()) {
//    yield();
//  }
  FillStack();
#if !ENABLE_DEDICATED_SPI
  Serial.println(F(
    "\nFor best performance edit SdFatConfig.h\n"
    "and set ENABLE_DEDICATED_SPI nonzero"));
#endif  // !ENABLE_DEDICATED_SPI

  Serial.print(FIFO_DIM);
  Serial.println(F(" FIFO entries will be used."));

  // Initialize SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
//#if USE_RTC
//  if (!rtc.begin()) {
//    error("rtc.begin failed");
//  }
//  if (!rtc.isrunning()) {
//    // Set RTC to sketch compile date & time.
//    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//    error("RTC is NOT running!");
//  }
//  // Set callback
//  FsDateTime::setCallback(dateTime);
//#endif  // USE_RTC

  lastFadeTime = millis();
  printUnusedStack();
  clearSerialInput();
  printControl();
}
//------------------------------------------------------------------------------
void loop() {
  // Read any Serial data.
  fadeLED(millis());
  getButtonState();
  serialCheck();
}
