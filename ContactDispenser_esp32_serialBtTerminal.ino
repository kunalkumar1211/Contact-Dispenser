#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

const int xDir = 23;
const int xStp = 22;

const int yDir = 21;
const int yStp = 19;

const int zDir = 18;
const int zStp = 17;

const int pDir = 16;
const int pStp = 4;

#define M0 25
#define M1 33
#define M2 32

int currentState = 0;
bool configComplete = false;
bool wasConnected = false; // Tracks connection status

int zLen = 0;
int xLen = 0;
int yLen = 0;
int stepsBtwDroplets = 0;
int pumpSteps=0;
int dropDelay = 0;

void setup() {

  Serial.begin(115200);
  SerialBT.begin("ContactDispenser(kun)"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
  pinMode(xDir, OUTPUT);
  pinMode(xStp, OUTPUT);

  pinMode(yDir, OUTPUT);
  pinMode(yStp, OUTPUT);

  pinMode(zDir, OUTPUT);
  pinMode(zStp, OUTPUT);

  pinMode(pDir, OUTPUT);
  pinMode(pStp, OUTPUT);
 

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);

  digitalWrite(M0,1); 
  digitalWrite(M1,1); 
  digitalWrite(M2,1); //6400 steps


  
}

void zDisp(int zLen){
  delay(1000);
  digitalWrite(zDir, LOW);
  for (int i = 0; i < zLen; i++) {   
    digitalWrite(zStp, HIGH);
    delayMicroseconds(300);
    digitalWrite(zStp, LOW);
    delayMicroseconds(300);
  }
  delay(500);
  digitalWrite(pDir, LOW);
  for (int i = 0; i < pumpSteps; i++) {   
    digitalWrite(pStp, HIGH);
    delayMicroseconds(10000);
    digitalWrite(pStp, LOW);
    delayMicroseconds(10000);
  }
  delay(dropDelay);

  digitalWrite(zDir, HIGH);
  for (int i = 0; i < zLen; i++) {   
    digitalWrite(zStp, HIGH);
    delayMicroseconds(300);
    digitalWrite(zStp, LOW);
    delayMicroseconds(300);
  }
  delay(1000);
}

void printPrompt() {
  switch (currentState) {
    case 0:
      SerialBT.println("\n--> zLen:");
      break;
    case 1:
      SerialBT.println("--> xLen:");
      break;
    case 2:
      SerialBT.println("--> yLen:");
      break;
    case 3:
      SerialBT.println("--> stepsBtwDroplets:");
      break;
    case 4:
      SerialBT.println("--> pumpSteps:");
      break;
    case 5:
      SerialBT.println("--> dropDelay:");
      break;
  }
}

void loop() {

    bool isConnected = SerialBT.hasClient(); // Check if phone is connected

  // 1. DETECT NEW CONNECTION
  if (isConnected && !wasConnected) {
    wasConnected = true;
    delay(500); // Short pause to let terminal UI stabilize on phone
    currentState = 0;
    configComplete = false;
    printPrompt(); // Prompt for the first variable right after connection
  } 
  else if (!isConnected && wasConnected) {
    wasConnected = false; // Reset if phone disconnects
  }

  // 2. READ INPUTS
  if (isConnected && !configComplete && SerialBT.available()) {
    int inputVal = SerialBT.parseInt();

    // Clear trailing line breaks or spaces from buffer
    while (SerialBT.available() && (SerialBT.peek() == '\r' || SerialBT.peek() == '\n' ||  SerialBT.peek() == ' ')) {
      SerialBT.read();
    }

    // Store value into the appropriate variable based on state
    switch (currentState) {
      case 0:
        zLen = inputVal;
        SerialBT.print("Saved zLen = "); SerialBT.println(zLen);
        currentState = 1;
        break;

      case 1:
        xLen = inputVal;
        SerialBT.print("Saved xLen = "); SerialBT.println(xLen);
        currentState = 2;
        break;

      case 2:
        yLen = inputVal;
        SerialBT.print("Saved yLen = "); SerialBT.println(yLen);
        currentState = 3;
        break;

      case 3:
        stepsBtwDroplets = inputVal;
        SerialBT.print("Saved stepsBtwDroplets = "); SerialBT.println(stepsBtwDroplets);
        currentState = 4;
        break;

      case 4:
        pumpSteps = inputVal;
        SerialBT.print("Saved pumpSteps = "); SerialBT.println(pumpSteps);
        currentState = 5;
        break;
      
      case 5:
        dropDelay = inputVal;
        SerialBT.print("Saved dropDelay = "); SerialBT.println(dropDelay);
        currentState = 0;
        // All 4 variables have been received!
        SerialBT.println("\n=== All Variables Configured ===");
        SerialBT.print("zLen: "); SerialBT.println(zLen);
        SerialBT.print("xLen: "); SerialBT.println(xLen);
        SerialBT.print("yLen: "); SerialBT.println(yLen);
        SerialBT.print("stepsBtwDroplets: "); SerialBT.println(stepsBtwDroplets);
        SerialBT.print("pumpSteps: "); SerialBT.println(pumpSteps);
        SerialBT.print("dropDelay: "); SerialBT.println(dropDelay);
        SerialBT.println("===============================");
        configComplete = true;
        break;
    }

       // Ask for the next variable if not done
    if (!configComplete) {
      printPrompt();
    }
  }
  
// 3. MOTOR MOTION CODE
  if (configComplete) {
  forward(xLen);
  up(yLen);
  backward(xLen);
  down(yLen);
  delay(10);
  configComplete=false;
  printPrompt();
  }
}
void forward(int xLen){
    digitalWrite(xDir, LOW);
  for (int i = 0; i < xLen; i++) {   
    digitalWrite(xStp, HIGH);
    delayMicroseconds(300);
    digitalWrite(xStp, LOW);
    delayMicroseconds(300);

      if(i%stepsBtwDroplets==0){
      zDisp(zLen);
      }
    }
}
void backward(int xLen){
    digitalWrite(xDir, HIGH);
  for (int i = 0; i < xLen; i++) {   
    digitalWrite(xStp, HIGH);
    delayMicroseconds(300);
    digitalWrite(xStp, LOW);
    delayMicroseconds(300);

      if(i%stepsBtwDroplets==0){
      zDisp(zLen);
      }
    }
}
void up(int yLen){
  digitalWrite(yDir, LOW);
  for (int i = 0; i < yLen; i++) {   
    digitalWrite(yStp, HIGH);
    delayMicroseconds(300);
    digitalWrite(yStp, LOW);
    delayMicroseconds(300);

      if(i%stepsBtwDroplets==0){
      zDisp(zLen);
      }
  }
}

void down(int yLen) {
  digitalWrite(yDir, HIGH);
  for (int i = 0; i < yLen; i++) {   
    digitalWrite(yStp, HIGH);
    delayMicroseconds(300);
    digitalWrite(yStp, LOW);
    delayMicroseconds(300);

      if(i%stepsBtwDroplets==0){
      zDisp(zLen);
      }
  }
}