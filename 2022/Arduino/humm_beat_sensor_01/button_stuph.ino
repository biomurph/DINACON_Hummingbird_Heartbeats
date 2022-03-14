


int getButtonState(){
  buttonValue = digitalRead(BUTTON_PIN);
  if(buttonValue != lastButtonValue){
    delay(10);
    buttonValue = digitalRead(BUTTON_PIN);
    if(buttonValue != lastButtonValue){
      if(buttonValue == LOW){ // she's pressed!
        Serial.println(F("START RECORDING from button press"));
        isRunning = true;
        fadeLED(millis());
      }
    }
    lastButtonValue = buttonValue;
  }
  return buttonState;
}
