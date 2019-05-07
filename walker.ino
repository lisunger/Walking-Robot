/*
 * Author: Nikolay Kanev
 */
#include <SoftwareSerial.h>

/* Motor1 */
#define BPWM    5  // BENBL
#define BPHASE  6
/* Motor2 */
#define APWM    10  // AENBL
#define APHASE  8
#define MODE    9
#define OPT1    4   // home position
#define OPT2    12  // walk
#define OPT3    11  // turn
#define Rx      A5
#define Tx      A4

#define LEFT 0
#define RIGHT 1
#define FORWARD 2
#define BACKWARD 3
#define BUFF_SIZE 30

SoftwareSerial BTSerial(Rx, Tx);  //Rx , Tx
int velocitySpin = 80;
int velocityWalk = 80;
int readDelay = 10;
char RxBuffer[BUFF_SIZE];

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  Serial.println("Starting");

  /* setup на моторите */
  pinMode(BPWM, OUTPUT);
  pinMode(BPHASE, OUTPUT);
  pinMode(APWM, OUTPUT);
  pinMode(APHASE, OUTPUT);
  pinMode(MODE, OUTPUT);
  digitalWrite(MODE, HIGH);

  /* Optron setup */
  pinMode(OPT1, INPUT_PULLUP);
  pinMode(OPT2, INPUT_PULLUP);
  pinMode(OPT3, INPUT_PULLUP);

}

void loop() {
  
    // изчиства натрупани команди в буфера
    while(BTSerial.available()) {
    BTSerial.read();
    }

    // изчаква, докато се събере цяла команда от 5 байта
    while(BTSerial.available() < 5) {
      delay(readDelay);
    }

    // чете и изпълнява команда
    readCommand();
}

void readCommand() {
  byte command = BTSerial.read() - 48; // първият байт е видът на командата, превръща се от ASCII символ в число
  
  int value = 0; // следващите 4 байта са 4 символа, които се превръщат в четирицифрено число
  value += ((long)BTSerial.read() - 48) * 1000;
  value += ((long)BTSerial.read() - 48) * 100;
  value += (BTSerial.read() - 48) * 10;
  value += BTSerial.read() - 48;
      
  Serial.println(command);
  Serial.println(value);

  executeCommand(command, value);
}

/*
 * command - код на командата: 
 *  1 : напред / forward
 *  2 : назад / backward
 *  3 : обратно на часовника / counter-clockwise
 *  4 : по часовника / clockwise
 *  5 : сменя скоростта на стъпване / set walking speed
 *  6 : сменя скоростта на въртене / set turning speed
 *  7 : спирачка / brake
 *  
 * value - число от 0 до 9999, което върши следното: 
 *  за команди от 1 до 4 зависи от променливата useHoles
 *  за команди 5 и 6 е нова стойност за скоростта на моторите; трябва да е между 0 и 255
 */
void executeCommand(byte command, int value) {

  // ако е true, стойността на value ще бъде броят дупки, на които да се придвижи роботът
  // ако е false, стойността на value ще бъде броят милисекунди, за които да се движи роботът, преди да спре (9999 значи безкрайно, докато не се прати команда brake)
  bool useHoles = ;
                    
  if(useHoles) {
    switch(command) {
      case 1 : {
        stepForward(value);
        break;
      }
      case 2 : {
        stepBackward(value);
        break;
      }
      case 3 : {
        turnCClockwise(value);
        break;
      }
      case 4 : {
        turnClockwise(value);
        break;
      }
    }
  } 
  else {
    switch(command) {
      case 1 : {
        forward();
        if(value != 9999) {
          delay(value);
          brake();
        }
        break;
      }
      case 2 : {
        backward();
        if(value != 9999) {
          delay(value);
          brake();
        }
        break;
      }
      case 3 : {
        cClockwise();
        if(value != 9999) {
          delay(value);
          brake();
        }
        break;
      }
      case 4 : {
        clockwise();
        if(value != 9999) {
          delay(value);
          brake();
        }
        break;
      }
    }
  }
  switch(command) {
    case 5 : {
      setSpeedWalk(value);
      break;
    }
    case 6 : {
      setSpeedSpin(value);
      break;
    }
    case 7 : {
      brake();
      break;
    }
  }
}

// направи завой надясно (по часовника) с holes на брой дупки
void turnClockwise(int holes) {
  turn(holes, RIGHT);
}

// направи завой наляво (обратно на часовника) с holes на брой дупки
void turnCClockwise(int holes) {
  turn(holes, LEFT);
}

// направи движение напред с holes на брой дупки
void stepForward(int holes) {
  makeStep(holes, FORWARD);
}

// направи движение назад с holes на брой дупки
void stepBackward(int holes) {
  makeStep(holes, BACKWARD);
}

// завъртане с holes на брой дупки; посоката е LEFT/RIGHT
void turn(int holes, int direct) {
  int changes = 0;
  int value = digitalRead(OPT3) ? false : true;
  if (direct == LEFT) cClockwise();
  if (direct == RIGHT) clockwise();

  while (changes < (holes * 2)) {
    bool currentHole = digitalRead(OPT3) ? false : true;
    if (currentHole != value) {
      changes++;
      value = currentHole;
    }
    delay(5);
  }
  brake();
}

// придвижване на краката с holes на брой дупки; посоката е FORWARD/BACKWARD
void makeStep(int holes, int direct) {
  int changes = 0;
  int value = digitalRead(OPT2) ? false : true;
  if (direct == FORWARD) forward();
  if (direct == BACKWARD) backward();

  while (changes < (holes * 2)) {
    bool currentHole = digitalRead(OPT2) ? false : true;
    if (currentHole != value) {
      changes++;
      value = currentHole;
    }
    delay(5);
  }
  brake();
}

// задава скоростта на ходене
void setSpeedWalk(byte value) {
  if(value < 0) value = 0;
  if(value > 255) value = 255;
  velocityWalk = value;
}

// задава скоростта на въртене
void setSpeedSpin(byte value) {
  if(value < 0) value = 0;
  if(value > 255) value = 255;
  velocitySpin = value;
}

// пуска робота да се движи напред
void forward() {
  digitalWrite(BPHASE, HIGH);
  analogWrite(BPWM, velocityWalk);
}

// пуска робота да се движи назад
void backward() {
  digitalWrite(BPHASE, LOW);
  analogWrite(BPWM, velocityWalk);
}

// пуска робота да се движи по часовника
void clockwise() {
  digitalWrite(APHASE, LOW);
  analogWrite(APWM, velocitySpin);
}

// пуска робота да се движи обратно на часовника
void cClockwise() {
  digitalWrite(APHASE, HIGH);
  analogWrite(APWM, velocitySpin);
}

// спирачка!!
void brake() {
  analogWrite(APWM, 0);
  analogWrite(BPWM, 0);
}

/*
void connectSerialToBLE() {
  // време за изчакване между четенията
  int delay_time = 10;

  while (Serial.available()) {
    BTSerial.write(Serial.read());
  }

  if (BTSerial.available()) {
    int i = 0;
    while (BTSerial.available()) {
      RxBuffer[i++] = BTSerial.read();
      if (i >= BUFF_SIZE) {
        i--;
      }
      delay(delay_time);
    }
    RxBuffer[i] = '\0';
    Serial.println(RxBuffer);
  }
}
*/
