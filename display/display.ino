#include <MicroView.h>

#define UP_PIN 2
#define DOWN_PIN 3

#define DEBOUNCE_TIME 150

int incomingByte = 0;

unsigned long timeLastPressedUpButton = 0;
unsigned long timeLastPressedDownButton = 0;


void setup() {

  Serial.begin(9600);
  
  
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);


  uView.begin();      // begin of MicroView
  uView.clear(PAGE);   // erase hardware memory inside the OLED controller
  uView.display();
  uView.setFontType(1);

}

void loop(){


  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:

    incomingByte = Serial.read();

    if(incomingByte == '@'){
      uView.clear(PAGE);
      uView.setCursor(0,0);
    } 
    else {
      uView.write(incomingByte);
    }


    uView.display();
  }
  
  
  
  int upButtonState = digitalRead(UP_PIN);
  int downButtonState = digitalRead(DOWN_PIN);
  
  unsigned long currentMillis = millis();
  
  if(!upButtonState && ((currentMillis - timeLastPressedUpButton) > DEBOUNCE_TIME)){
      timeLastPressedUpButton = currentMillis;
      Serial.print("u");
  }
  
   if(!downButtonState && ((currentMillis - timeLastPressedDownButton) > DEBOUNCE_TIME)){
      timeLastPressedDownButton = currentMillis;
      Serial.print("d");
  }
  
  
  
  
  
  
  
  
}


