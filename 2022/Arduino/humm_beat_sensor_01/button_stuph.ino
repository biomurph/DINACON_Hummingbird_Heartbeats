


void getButtonState(){
  buttonValue = digitalRead(BUTTON_PIN);
  if(buttonValue != lastButtonValue){
    delay(10);
    buttonValue = digitalRead(BUTTON_PIN);
    if(buttonValue != lastButtonValue){
      if(buttonValue == LOW){ // she's pressed!
        switch(buttonState){
          case RECORD_OFF:
            Serial.println(F("START RECORDING"));
            buttonState = RECORD_ON; break;
          case RECORD_ON:
            Serial.println(F("STOP RECORDING"));
            buttonState = RECORD_OFF; break;
          default: break;
        }
      }
      lastButtonValue = buttonValue;
    }
  }
}
