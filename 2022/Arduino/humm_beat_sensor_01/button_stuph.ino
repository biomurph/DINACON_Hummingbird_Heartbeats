


int getButtonState(){
  buttonValue = digitalRead(BUTTON_PIN);
  if(buttonValue != lastButtonValue){
    delay(10);
    buttonValue = digitalRead(BUTTON_PIN);
    if(buttonValue != lastButtonValue){
      if(buttonValue == LOW){ // she's pressed!
        switch(buttonState){
          case RECORD_OFF:
            Serial.println(F("START RECORDING"));
            buttonState = RECORD_ON; isRunning = true; fadeLED(millis()); break;   // RECORD_ON = 2
          case RECORD_ON:
            Serial.println(F("STOP RECORDING DOESN'T WORK YET!"));
            buttonState = RECORD_OFF; break;  // RECORD_OFF = 1
          default: break;
        }
      }
      lastButtonValue = buttonValue;
    }
  }
  return buttonState;
}
