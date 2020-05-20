/*
   Code that functions by 20.05.2020, but only measuring temperature. That is: temp diff is set to
   constantly 0.5 degrees, so it will start at any temp. Added
   sort of a countdown timer, but need to use interrupts to make it function 
   properly. Now it count downe in 5 sec steps. 
   DO NOT edit this one!!!!!
*/

#include <SPI.h>
#include <SD.h>
#include <SdFat.h> //kan denne gjøre at lesing fra sd-kort blir bedre?
#include <TFT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
File root;
File dataFile;
#define lcd_cs 6
#define dc     9
#define rst    8
#define stp 19
#define EN  7
#define DIR 15
#define buzzerPin 16
#define ONE_WIRE_BUS A0
#define INITR_BLACKTAB 0x02
TFT TFTscreen = TFT(lcd_cs, dc, rst);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfFiles = 0;
short currentPosition = 1;
const short up = 2, down = 3, enter = 5, back = 4; //use this for 4-button pad marked 1 2 3 4
//or just change pin # depending on pad used
//const short up = 2, down = 3, enter = 4, back = 5; //for 4-button pad unmarked
String fileName[20];
short runTemp;
float temperature = 0;
int dataToexecute[10];
char BS; // Button State;
short temperatureTolerence = 1; // set tolerance +-1 degree centigrade of the temperature written in txt file
short dataExecutionCounter = 0;
short speed(400); //sets agitation speed, higher # slows down, set between 200 and 1000. 
bool dir = true;
PImage logo;


void setup() {
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  TFTscreen.begin();

  TFTscreen.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  TFTscreen.setRotation(1);
  TFTscreen.background(0, 5, 0);
  TFTscreen.stroke(255, 255, 255);
  for (int i = 2; i <= 5; i++) {
    pinMode(i, INPUT);
  }
  pinMode(stp, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(DIR, OUTPUT);
  resetStepper();

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  if (!SD.begin(10)) {
    while (1);
  }

  root = SD.open("/");
  printDirectory(root, 0);
// logo preload inside setup, works. Call to separate function does not!
  TFTscreen.fill( 0, 0, 0);
  TFTscreen.rect(0, 0, TFTscreen.width(), TFTscreen.height());
//  logo = TFTscreen.loadImage("fogd.bmp"); //Foto og Design logo
  logo = TFTscreen.loadImage("tfk_logo.bmp"); //Tromsø fotoklubb logo
//  logo = TFTscreen.loadImage("md_logo.bmp"); //TKH Media Design logo
//  logo = TFTscreen.loadImage("afd_logo.bmp"); //3D render of Arduino film developer
  if (logo.isValid() == false) {
    return;
  }
  TFTscreen.image(logo, 0, 0);
  delay(3000);
  TFTscreen.fill( 0, 0, 0);
  TFTscreen.rect(0, 0, TFTscreen.width(), TFTscreen.height()); 
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {
  int entered = 0;
  GUI();
  fileGUI();
  tempGUI();
  BS = Button();
  if (BS == 'D') {
    currentPosition++;
    if (currentPosition > numberOfFiles)
      currentPosition = 1;
  }
  else if (BS == 'U') {
    currentPosition--;
    if (currentPosition < 1)
      currentPosition = numberOfFiles;;
  }
  else if (BS == 'E') {
    entered = 1;
    showTXT(fileName[currentPosition]);
    for (;;) {
      tempGUI();
      BS = Button();
      if (BS == 'B') {
        break;
      }
      else if (BS == 'E') {
        askingGUI();
        for (;;) {
          tempGUI();
          BS = Button();
          if (BS == 'B') {
            break;
          }
          else if (BS == 'E') {
            startingGUI();
            break;
          }
        }
        break;
      }
    }
  }
  if (entered == 1) {
    TFTscreen.fill(0, 0, 0);
    TFTscreen.rect(0, 0, TFTscreen.width(), TFTscreen.height());
    entered = 0;
  }
}

/*
   printDirectory lister opp filene fra sd-kortet på skjermen
*/
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    numberOfFiles++;
    fileName[numberOfFiles] = entry.name();
    char x = fileName[numberOfFiles][fileName[numberOfFiles].length() - 1];
    if (x != 'T') {
      numberOfFiles--;
    }
    entry.close();
  }
}

void GUI() {
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(200, 0, 0);
  TFTscreen.rect(0, 0, TFTscreen.width(), 20);
  TFTscreen.setTextSize(2);
  TFTscreen.text("FILM DEV", 35, 3);
  TFTscreen.fill(200, 0, 0);
  TFTscreen.rect(0, 110, TFTscreen.width(), TFTscreen.height() - 110);
  TFTscreen.line( TFTscreen.width() / 2, 110, TFTscreen.width() / 2, TFTscreen.height());
  TFTscreen.setTextSize(1);
  TFTscreen.text("SELECT", 25, 115);
  TFTscreen.text("BACK", 105, 115);
}

char Button() {
  char pressed;
  for (;;) {
    if (digitalRead(up) == 1) {
      waitTorelease();
      pressed =  'U';
      break;
    }
    else if (digitalRead(down) == 1) {
      waitTorelease();
      pressed =  'D';
      break;
    }
    else if (digitalRead(enter) == 1) {
      waitTorelease();
      pressed = 'E';
      break;
    }
    if (digitalRead(back) == 1) {
      waitTorelease();
      pressed =  'B';
      break;
    }
  }
  return pressed;
}

void waitTorelease() {
  for (;;) {
    if (digitalRead(up) == 0 && digitalRead(down) == 0 && digitalRead(enter) == 0 && digitalRead(back) == 0 )
      break;
  }
}

void tempGUI() {

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
//  temperature = 25;
  TFTscreen.stroke(0, 0, 255);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.fillRect(83,85,75,20,ST7735_BLACK);  
  TFTscreen.setCursor(85, 90);
  TFTscreen.setTextSize(2);
  TFTscreen.print(temperature);
  TFTscreen.setCursor(150, 87);
  TFTscreen.setTextSize(1);
  TFTscreen.print("o");
}

void showTXT(String txt) { //open txt file and reads the content into an array 
  TFTscreen.stroke(0, 0, 0); //var 255, må se om den må tilbake
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect(0, 0, TFTscreen.width(), TFTscreen.height());
  short newLineCounter = 0;
  short data = 0;
  char dataArray[100];

  TFTscreen.stroke(0, 0, 255);
  TFTscreen.setCursor(0, 22);
  TFTscreen.print("   ");
  TFTscreen.println(txt);
  TFTscreen.stroke(0, 255, 255);
  File dataFile = SD.open(txt);
  String dataString = "";
  TFTscreen.print("   ");

  if (dataFile) {
    while (dataFile.available()) {
      char x = dataFile.read();
      TFTscreen.print(x);
      if (x == '\n') {
        newLineCounter++;
        TFTscreen.print("   ");
      }
      if (newLineCounter > 1) {
        data++;
        dataArray[data] = x;
        if (dataArray[data] == 'c' || dataArray[data] == 't') {
          dataExecutionCounter++;
          dataToexecute[dataExecutionCounter] = dataString.toInt();
          dataString = "";
        }
        else {
          dataString += dataArray[data];
        }
      }
    }
    dataFile.close();
  }
  else {
    wfe();
  }
  GUI();
  tempGUI();
}

void wfe() {
  for (;;) {
  }
}

void askingGUI() { // Ask to start agitation
  TFTscreen.stroke(255, 255, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.rect( 0, ((TFTscreen.height() / 2)-15), TFTscreen.width(), 50);
  TFTscreen.setTextSize(2);
  TFTscreen.text("Start?", 15 + TFTscreen.width() * (3 / 4), (TFTscreen.height() / 2) + 2);
}

void startingGUI() {
  float tempCheck=temperature-dataToexecute[1]; //Houston, it seems like we have a problem here with tempCheck
  if (tempCheck<0){
    tempCheck=-1*tempCheck;
    }
    tempCheck=.5; //uncomment to override tempchk //if you do, you will have problems
  if (tempCheck>temperatureTolerence){
    TFTscreen.stroke(255,255,0);
    TFTscreen.fill(0,0,0);
    TFTscreen.setTextSize(2);
    TFTscreen.rect( 0,(TFTscreen.height()/2)-40, TFTscreen.width(),60);
    TFTscreen.setCursor(5,(TFTscreen.height()/2)-38);
    TFTscreen.println("Temp error!");
    TFTscreen.setTextSize(1);
    TFTscreen.println(" ");
    TFTscreen.print(" Safe temp:    ");
    TFTscreen.println(dataToexecute[1]);
    TFTscreen.println(" ");
    TFTscreen.print(" Current temp: ");
    TFTscreen.println(temperature);
    BS=Button();
    resetFunc();
  } else {

  TFTscreen.stroke(150, 50, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.setTextSize(2);
  TFTscreen.rect( 0, (TFTscreen.height() / 2) - 45, TFTscreen.width(), 90);
  TFTscreen.setCursor(8, (TFTscreen.height() / 2) - 40);
  TFTscreen.println("Processing");
  TFTscreen.setTextSize(1); //here set text size for developing steps.
  for (int rl = 2; rl <= dataExecutionCounter; rl++) {
    TFTscreen.setCursor(0, (TFTscreen.height() / 2) - 20);
    for (int rk = 2; rk <= dataExecutionCounter; rk++) {
      if (rk <= rl) {
        TFTscreen.stroke(0, 255, 0);
        TFTscreen.print("  ");
        TFTscreen.print(dataToexecute[rk]);
      }
      else {
        TFTscreen.stroke(255, 255, 255);
        TFTscreen.print("  ");
        TFTscreen.print(dataToexecute[rk]);
      }
      TFTscreen.print(" s. ");
      if (rk % 3 == 0) { //this one defines how many step printed on one line on display
        TFTscreen.println("");
      }
    }
    if (rl > 2) {
            for (;;) {
        BS = Button();
        break;
      }
    }
    runMotor(dataToexecute[rl]);
  }
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.fill(0, 0, 0);
  TFTscreen.stroke(255, 0, 0);
  TFTscreen.setTextSize(1);
  TFTscreen.rect( 0, (TFTscreen.height() / 2) - 45, TFTscreen.width(), 90);
  TFTscreen.setCursor(10, (TFTscreen.height() / 2) - 25);
  TFTscreen.print("PRESS A KEY");
  BS = Button();
  resetFunc();
  }
}

void runMotor( int u) { //turns the motor 5 secs each direction for specified time
  unsigned long t = u;
  short dirTime(5000);
  t = t * 1000;
  tempGUI();
  TFTscreen.fillRect(8,85,75,20,ST7735_BLACK);
  digitalWrite(EN, LOW);
  
  unsigned long timeCounter = millis();
  for (/*int rm=1; rm<=t; rm++*/;;) {
    digitalWrite(stp, HIGH);
    delayMicroseconds(speed);
    digitalWrite(stp, LOW);
    delayMicroseconds(speed);
    uint32_t ms = millis();
    static uint32_t last_time = 0;
    if ((ms - last_time) > dirTime) {
       if (dir) {
        digitalWrite(DIR, HIGH);
       } else {
        digitalWrite(DIR, LOW);
      }
    dir = !dir;
    last_time = ms;

    unsigned long executed = millis() - timeCounter;
    int seconds = ((t-executed)/1000)%60;
    int sec = (t-executed)/1000;
    int minutes = sec/60;
    
    TFTscreen.fillRect(8,85,75,20,ST7735_BLACK);  
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setTextSize(2);
    TFTscreen.setCursor(8, 90);
    if (minutes < 10) {
      TFTscreen.print("0"); TFTscreen.print(minutes);
    }else{
      TFTscreen.print(minutes);
    }
    TFTscreen.print(":");
    if ((seconds == 0)&&(minutes>=1)) {
      seconds = 59;
      seconds--;
    }
    if (seconds < 9) {
      TFTscreen.print("0"); TFTscreen.print(seconds+1);
    }else{
    TFTscreen.print(seconds+1); // 1000);
    }
    if (executed > t) {
      TFTscreen.fillRect(8,85,75,20,ST7735_BLACK);
      TFTscreen.stroke(255, 0, 0);
      TFTscreen.setCursor(8, 90);
      TFTscreen.print("STOP!");
//      delay(1000);
      tempGUI();  
      break;
      }
    }
  }
  digitalWrite(buzzerPin, HIGH);
  delay(500); //set to 1000 to beep for 1 second
  digitalWrite(buzzerPin, LOW);
  resetStepper();
  delay(2000); //can be set to 5000
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.fillRect(8,85,75,20,ST7735_BLACK);
  TFTscreen.setTextSize(2);
  TFTscreen.setCursor(8, 90);
  TFTscreen.print("NEXT!");
  TFTscreen.setTextSize(1);
}

void resetStepper() {
  digitalWrite(stp, LOW);
  digitalWrite(EN, HIGH);
}

void fileGUI() {
  TFTscreen.fill(0, 0, 0);
  short maximumFileOnScreen = 10;
  TFTscreen.setCursor(0, 22);
  TFTscreen.setTextSize(1);
  TFTscreen.stroke(255, 255, 0);
  for (int i = 1; i <= maximumFileOnScreen; i++) {
    if (i == currentPosition) {
      TFTscreen.stroke(0, 0, 255);
      TFTscreen.print("   ");
      TFTscreen.println(fileName[i]);
      TFTscreen.stroke(255, 255, 0);
    } else {
      TFTscreen.print("   ");
      TFTscreen.println(fileName[i]);
    }
  }
}

/* 
* can I use this somehow?
* 
// RTM_TimerCalc 1.20,  RuntimeMicro.com
// Timer-1 Mode_14_16Bit_Fast_TOP_is_ICR

TCCR1B = 0x18; // 0001 1000, Disable Timer Clock 
TCCR1A = 0xA2; // 1010 0010

ICR1 = 50000-1;
OCR1A = (int) (ICR1 * 0,25);
OCR1B = (int) (ICR1 * 0,50);
TCNT1=0x0;

// UnComment following lines for UNO-NANO Timer-1 Pins 
// pinMode(9, OUTPUT);  // OC1a
// pinMode(10, OUTPUT); // OC1b

// UnComment following lines for 2560 Timer-1 Pins 
// pinMode(11, OUTPUT);  // OC1a
// pinMode(12, OUTPUT);  // OC1b

TCCR1B |= 3; // Prescale=64, Enable Timer Clock


*/
