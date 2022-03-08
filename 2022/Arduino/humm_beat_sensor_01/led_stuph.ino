


int fadeLED(unsigned long callTime){
  if(callTime - lastFadeTime > LEDfadeTime){
    if(fadeUp){
      LEDfadeValue++;
    } else {
      LEDfadeValue--;
    }
    if(LEDfadeValue < 0){
      LEDfadeValue = 0;
      fadeUp = true;
    }
    if(LEDfadeValue > 255){
      LEDfadeValue = 255;
      fadeUp = false;
    }
    analogWrite(LED_PIN,LEDfadeValue);
    lastFadeTime = callTime;
  }
  return LEDfadeValue;
}
