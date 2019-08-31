/*****
 
 Threshold is the value that helps you 
 determine what's black and white. Sparki's 
 infrared reflectance sensors indicate white 
 as close to 900, and black as around 200.
 This example uses a threshold of 700 for 
 the example, but if you have a narrow line, 
 or perhaps a lighter black, you may need to 
 adjust.
******/

#include <Sparki.h> // include the sparki library

int count = 0;

void setup() 
{
}

bool atJunction(int lineCenter, int edgeLeft, int edgeRight, int threshold) {// update this?
  if (lineCenter < threshold && (edgeLeft < threshold || edgeRight < threshold)) {
    return true;
  }
  return false;
}

bool atDeadEnd(int lineCenter, int lineLeft, int lineRight, int edgeLeft, int edgeRight, int threshold) {
  if (lineCenter > threshold && edgeLeft > threshold && edgeRight > threshold && lineLeft > threshold && lineRight > threshold) {
    return true;
  }
  return false;
}

void courseCorrect(int lineCenter, int lineRight, int lineLeft){
  int threshold = 300;
  if( lineLeft < threshold ){

    sparki.moveLeft(3);
  }
  else if( lineRight < threshold ){

    sparki.moveRight(3);
  }
}

void loop() {
  count++;
  int threshold = 300;
  
  int lineLeft   = sparki.lineLeft();   // measure the left IR sensor
  int lineCenter = sparki.lineCenter(); // measure the center IR sensor
  int lineRight  = sparki.lineRight();  // measure the right IR sensor
  int edgeLeft   = sparki.edgeLeft();
  int edgeRight  = sparki.edgeRight();
/*
  int cm = sparki.ping();
  if(cm != -1) // make sure its not too close or too far
  { 
     if(cm < 10) // if the distance measured is less than 10 centimeters
     {
        sparki.beep(); // beep!
     }
  }
*/

int cm = sparki.ping();
 if(cm < 10) // if the distance measured is less than 10 centimeters
  {
//            sparki.beep(); // beep!
//    sparki.moveLeft(90);
    Serial.println("Done");
  }
          else{

if(atJunction(lineCenter, edgeLeft, edgeRight, threshold))
{
  if(edgeLeft < threshold)
  {
    sparki.moveForward(4.5);
    sparki.moveLeft(90);
  }
  else{
//  sparki.moveForward(1);
  //sparki.moveBackward(1);
  sparki.moveForward(4.5);
  if(sparki.lineCenter() > threshold)
  {
//    sparki.moveForward(1.4);
//    sparki.moveLeft(20);
//    sparki.moveRight(20);
    sparki.moveRight(90);
  }
  }
}
else if(atDeadEnd(lineCenter, lineLeft, lineRight, edgeLeft, edgeRight, threshold))
{
  Serial.print("number of loops = ");
  Serial.println(count);
  sparki.moveForward(7);
  sparki.moveLeft(180);
}
else
{
  courseCorrect(lineCenter, lineRight, lineLeft);
  sparki.moveForward();
}

          }     

  delay(100); // wait 0.1 seconds
}
