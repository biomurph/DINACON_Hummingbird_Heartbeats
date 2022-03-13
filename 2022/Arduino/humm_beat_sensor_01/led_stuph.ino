


int fadeLED(unsigned long callTime){
  if(isRunning){
    analogWrite(BLU_LED,0);
    digitalWrite(RED_LED,HIGH);
    return -1;
  }
  digitalWrite(RED_LED,LOW);
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
    analogWrite(BLU_LED,LEDfadeValue);
    lastFadeTime = callTime;
  }
  return LEDfadeValue;
}
