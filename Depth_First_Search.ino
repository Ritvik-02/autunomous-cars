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
double x_dist_cm = 0.0;
double y_dist_cm = 0.0;
int current_angle_degrees = 0;

const double TURN_DEGREES = 5;
const double DEFAULT_MOVE_FORWARD_CM = 1.0;
const double DEGREES_TO_RADIANS = PI / 180.0;
const int JUNCTION_DIST_THRES = 4;

struct junction {
  double x;
  double y;
  double exit_angles[4];
  int num_exits;
  int num_exits_seen;
  double prev_x;
  double prev_y;
  int entry_angle = 0;
};

struct junction visited_junctions[5];
junction last_seen_junction;
junction last_seen_not_first;
int num_junctions_seen = 0;

void setup() 
{
  Serial.begin(9600);      // open the serial port at 9600 bps:    
}

void moveLeftAndUpdatePos(double angle_to_turn=TURN_DEGREES) {
  sparki.moveLeft(angle_to_turn);
  current_angle_degrees += angle_to_turn;
  if(angle_to_turn != TURN_DEGREES){
  if(current_angle_degrees <= 110 && current_angle_degrees >= 70)
  {
    y_dist_cm += 4.4;
  }
  else if(current_angle_degrees >= -110 && current_angle_degrees <= -70)
  {
    y_dist_cm -= 4.4;
  }
  else if(abs(current_angle_degrees) >=160 && abs(current_angle_degrees) <= 200)
  {
    x_dist_cm -= 4.4;
  }
  else if(current_angle_degrees <= 20 && current_angle_degrees >= -20)
  {
    x_dist_cm += 4.4;
  }
  }
}

void moveRightAndUpdatePos(double angle_to_turn=TURN_DEGREES) {
  sparki.moveRight(angle_to_turn);
  current_angle_degrees -= angle_to_turn;
  if(angle_to_turn != TURN_DEGREES){
  if(current_angle_degrees <= 110 && current_angle_degrees >= 70)
  {
    y_dist_cm += 4.4;
  }
  else if(current_angle_degrees >= -110 && current_angle_degrees <= -70)
  {
    y_dist_cm -= 4.4;
  }
  else if(abs(abs(current_angle_degrees) - 180) <= 20)
  {
    x_dist_cm -= 4.4;
  }
  else
  {
    x_dist_cm += 4.4;
  }
  }
}

void moveForwardAndUpdatePos(double MOVE_FORWARD_CM=DEFAULT_MOVE_FORWARD_CM) {
  sparki.moveForward(MOVE_FORWARD_CM);
  x_dist_cm += MOVE_FORWARD_CM * cos(current_angle_degrees * DEGREES_TO_RADIANS);
  y_dist_cm += MOVE_FORWARD_CM * sin(current_angle_degrees * DEGREES_TO_RADIANS);
}

void moveBackwardAndUpdatePos(double MOVE_BACKWARD_CM) {
  sparki.moveBackward(MOVE_BACKWARD_CM);
  x_dist_cm -= MOVE_BACKWARD_CM * cos(current_angle_degrees * DEGREES_TO_RADIANS);
  y_dist_cm -= MOVE_BACKWARD_CM * sin(current_angle_degrees * DEGREES_TO_RADIANS);
}

double restrict_angle_to_360(double angle=current_angle_degrees) {
  if (angle > 180.0) {
    angle -= 360;
  }
  else if (angle < -180.0) {
    angle += 360;
  }
  return angle;
}

bool atJunction(int lineCenter, int edgeLeft, int edgeRight, int threshold) {// update this?
  if (lineCenter < threshold && (edgeLeft < threshold || edgeRight < threshold)) {
    return true;
  }
  return false;
}

bool seenJunctionBefore(junction& current_junction, int& current_juction_index) {
  for (int i = 0; i <= num_junctions_seen; i++) {
    if (abs(x_dist_cm - visited_junctions[i].x) <= JUNCTION_DIST_THRES && abs(y_dist_cm - visited_junctions[i].y) <= JUNCTION_DIST_THRES) {
      current_junction = visited_junctions[i];
      current_juction_index = i;
      return true;
    }
  }
  return false;
}

bool routesLeftToExplore(junction& current_junction) {
  return current_junction.num_exits_seen < current_junction.num_exits;
}

void addJunctionToSeen(junction& current_junction) {
  current_junction.x = x_dist_cm;
  current_junction.y = y_dist_cm;
  visited_junctions[num_junctions_seen] = current_junction;
  num_junctions_seen += 1;
  current_junction.prev_x = last_seen_junction.x;
  current_junction.prev_y = last_seen_junction.y;
  current_junction.entry_angle = current_angle_degrees;
}

void addAllPossibleDirections(junction& current_junction, int threshold) {
  int edgeLeft = sparki.edgeLeft();
  int edgeRight = sparki.edgeRight();
  bool leftExists = edgeLeft < threshold;
  bool rightExists = edgeRight < threshold;

  moveForwardAndUpdatePos(2.0);
  int lineCenter = sparki.lineCenter();
  moveBackwardAndUpdatePos(2.0);

  current_junction.num_exits = 0;
  current_junction.num_exits_seen = 0;

  bool straightExists = lineCenter < threshold;

  current_junction.exit_angles[current_junction.num_exits] = current_angle_degrees - 180.0;
  current_junction.num_exits += 1;
  current_junction.num_exits_seen += 1;

  if (leftExists) {
   current_junction.exit_angles[current_junction.num_exits] = current_angle_degrees + 90.0;
   current_junction.num_exits += 1;   
  }
  if (straightExists) {
   current_junction.exit_angles[current_junction.num_exits] = current_angle_degrees;
   current_junction.num_exits += 1;      
  }
  if (rightExists) {
   current_junction.exit_angles[current_junction.num_exits] = current_angle_degrees - 90.0;
   current_junction.num_exits += 1;
  }

  for (int i = 0; i < current_junction.num_exits; i++) {
    current_junction.exit_angles[i] = restrict_angle_to_360(current_junction.exit_angles[i]);
  }
  
  
}

bool atDeadEnd(int lineCenter, int lineLeft, int lineRight, int edgeLeft, int edgeRight, int threshold) {
  if (lineCenter > threshold && edgeLeft > threshold && edgeRight > threshold && lineLeft > threshold && lineRight > threshold) {
    return true;
  }
  return false;
}

void turnAround(){
  sparki.moveForward(7.0);
  moveLeftAndUpdatePos(180);
  x_dist_cm -= 1.2 * cos(current_angle_degrees * DEGREES_TO_RADIANS);
  y_dist_cm -= 1.2 * sin(current_angle_degrees * DEGREES_TO_RADIANS);
}

void courseCorrect(int lineCenter, int lineRight, int lineLeft){
  int threshold = 300;
  if( lineLeft < threshold ){

    moveLeftAndUpdatePos();
  }
  else if( lineRight < threshold ){

    moveRightAndUpdatePos();
  }
}

bool anyRoutesLeft()
{
  for(int i=0; i<9; i++)
  {
    if(routesLeftToExplore(visited_junctions[i]))
    {
      return true;
    }
  }
  return false;
}

void loop() {
  count++;
  int threshold = 300;
  
  int lineLeft   = sparki.lineLeft();   // measure the left IR sensor
  int lineCenter = sparki.lineCenter(); // measure the center IR sensor
  int lineRight  = sparki.lineRight();  // measure the right IR sensor
  int edgeLeft   = sparki.edgeLeft();
  int edgeRight  = sparki.edgeRight();
  int cm         = sparki.ping();

  if(cm <= 5)
  {
    Serial.println("Done");
  }

  else{
 
  if (atJunction(lineCenter, edgeLeft, edgeRight, threshold)) {
    junction current_junction;
    int current_junction_index;
    if (seenJunctionBefore(current_junction, current_junction_index)) {
      x_dist_cm = current_junction.x;
      y_dist_cm = current_junction.y;
      Serial.println("Seen this junction before (x,y)");
      Serial.println(current_junction.x);
      Serial.println(current_junction.y);
      Serial.print("number of exits: ");
      Serial.println(current_junction.num_exits);
      Serial.print("number of exits seen: ");
      Serial.println(current_junction.num_exits_seen);
      if (routesLeftToExplore(current_junction)) {
        double direction_to_explore = current_junction.exit_angles[current_junction.num_exits_seen];
        current_junction.num_exits_seen += 1;
        visited_junctions[current_junction_index] = current_junction; // change the index
        sparki.moveForward(4.4);
        moveLeftAndUpdatePos(direction_to_explore-current_angle_degrees);
      } else {
        if(anyRoutesLeft())
        {
          Serial.print("angle to turn = ");
          Serial.println(current_angle_degrees - current_junction.entry_angle);
          int direction_to_be = current_angle_degrees - current_junction.entry_angle;
            sparki.moveForward(4.4);
            moveRightAndUpdatePos(direction_to_be);
            if(current_junction.x != visited_junctions[0].x || current_junction.y != visited_junctions[0].y){
              if(abs(x_dist_cm - current_junction.prev_x) >= JUNCTION_DIST_THRES)
              {
                Serial.print("moving backward = ");
                Serial.println(abs(x_dist_cm - current_junction.prev_x));
                moveBackwardAndUpdatePos(abs(x_dist_cm - current_junction.prev_x));
              }
              else
              {
                Serial.print("moving backward = ");
                Serial.println(abs(y_dist_cm - current_junction.prev_y));
                moveBackwardAndUpdatePos(abs(y_dist_cm - current_junction.prev_y));
              }
            }
            else
            {
              if(abs(x_dist_cm - last_seen_not_first.x) >= JUNCTION_DIST_THRES)
              {
                Serial.print("moving backward = ");
                Serial.println(abs(x_dist_cm - last_seen_not_first.x));
                moveBackwardAndUpdatePos(abs(x_dist_cm - last_seen_not_first.x));
              }
              else
              {
                Serial.print("moving backward = ");
                Serial.println(abs(y_dist_cm - last_seen_not_first.y));
                moveBackwardAndUpdatePos(abs(y_dist_cm - last_seen_not_first.y));
              }
            }
            
        }
        else
        {
        Serial.println("Done!");
        }
      }
    }  //doneExploring();
    else {
      Serial.println("Seeing junction for the first time (x,y):");
      addJunctionToSeen(current_junction);
      addAllPossibleDirections(current_junction, threshold);
      Serial.println(current_junction.x);
      Serial.println(current_junction.y);
      Serial.print("number of exits: ");
      Serial.println(current_junction.num_exits);
      Serial.print("number of exits seen: ");
      Serial.println(current_junction.num_exits_seen);
      double direction_to_explore = current_junction.exit_angles[current_junction.num_exits_seen];
      current_junction.num_exits_seen += 1;
      visited_junctions[num_junctions_seen-1] = current_junction;
      sparki.moveForward(4.4);
      moveLeftAndUpdatePos(direction_to_explore-current_angle_degrees);
    }
    last_seen_junction=current_junction;
    if(current_junction.x != visited_junctions[0].x && current_junction.y != visited_junctions[0].y){
      last_seen_not_first = current_junction;
    }
  } else if (atDeadEnd(lineCenter, lineLeft, lineRight, edgeLeft, edgeRight, threshold)) {
    Serial.print("number of loops = ");
    Serial.println(count);
    junction last_junction = last_seen_junction;
    if(abs(x_dist_cm - last_junction.x) >= JUNCTION_DIST_THRES)
    {
      Serial.print("at dead end, moving backward = ");
      Serial.println(abs(x_dist_cm - last_junction.x));
      moveBackwardAndUpdatePos(abs(x_dist_cm - last_junction.x));
    }
    else
    {
      Serial.print("at dead end, moving backward = ");
      Serial.println(abs(y_dist_cm - last_junction.y));
      moveBackwardAndUpdatePos(abs(y_dist_cm - last_junction.y));
    }
  }else {
    courseCorrect( lineCenter, lineRight, lineLeft );
    moveForwardAndUpdatePos();
  }

  current_angle_degrees = restrict_angle_to_360(current_angle_degrees);


  }
  
  delay(100); // wait 0.1 seconds
}
