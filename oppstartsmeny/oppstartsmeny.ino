/*
 * Code to test startup menu to select between manual and presets:
 * 
 *             *******************
 *             *  FILM DEV       *
 *             *                 *
 *             * > Manual        *
 *             *   Presets       *
 *             *                 *
 *             * SELECT * BACK   *
 *             *******************
 *             
 *   The idea is to make it possible to use the developing machine in manual mode too, with simple start/stop           
 *   function of the motor. If possible with a timer that shows for how long it has been turning.
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
TFT TFTscreen = TFT(lcd_cs, dc, rst);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfFiles=0;
short currentPosition=1;
short menuPosition=0; //denne her er det jeg prøver å få til å endre seg ved knappetrykk up/down. 
const short up=2,down=3,enter=4,back=5;
String fileName[20];
short runTemp;
float temperature=0;
int dataToexecute[10];
char BS; // Button State;
// short temperatureTolerence=5;// if it is set to 3, motor will be turned on +-3 degree centigrade of the temperature written in txt file
short dataExecutionCounter=0;
PImage logo;
String initMenu[] = {
  "Manual",            //0
  "Presets",           //1
};


void setup() {
  // put your setup code here, to run once:

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  TFTscreen.begin();


  TFTscreen.background(0,5,0);
  TFTscreen.stroke(255,255,255);
  for(int i=2; i<=5;i++){
    pinMode(i,INPUT);
   }
  pinMode(stp, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(DIR, OUTPUT);
//  resetStepper();

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  //Serial.print("Initializing SD card...");

//  if (!SD.begin(10)) {
    //Serial.println("initialization failed!");
//    while (1);
//  }
  //Serial.println("initialization done.");

//  root = SD.open("/");
//  printDirectory(root, 0);

//  Serial.println("done!");
//  Serial.print("Number of files: ");
//  Serial.println(numberOfFiles);




    TFTscreen.fill( 210, 210,198);
    TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
//    logo = TFTscreen.loadImage("LOGO2.bmp");
//    logo = TFTscreen.loadImage("LOGO.bmp");
//    logo = TFTscreen.loadImage("FOGD.bmp");
//  if (!logo.isValid()) {
//    Serial.println(F("error while loading logo"));
//  }
//  if (logo.isValid() == false) {
//    return;
//  }
//  TFTscreen.image(logo, 0,0);
//  delay(3000);
  TFTscreen.fill( 0,0,0);
  TFTscreen.stroke(255,255,255);
  TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
}

void loop() {
  int entered=1;
  initGUI();
  BS=Button();
    if (BS=='D'){
      menuPosition++;
      if (menuPosition>1)
        menuPosition=0;
      }
   else if (BS=='U'){
      menuPosition--;
      if (menuPosition<0)
        menuPosition=1;
      }
  else if (BS=='E'){
      entered=1;
      showTXT(initMenu[menuPosition]);
      for(;;){
//        tempGUI();
        BS=Button();
        if (BS=='B'){
            break;
            }
        else if(BS=='E'){
 //         askingGUI();
          for(;;){
//            tempGUI();
            BS=Button();
            if (BS=='B'){
              break;
              }
            else if (BS=='E'){
//              checkingGUI();
//              startingGUI();


              break;
              }

            }
          break;
          }

       }
    }


  if(entered==1){
    TFTscreen.fill(0,0,0);
    TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
    entered=0;
  }

}

/*
 * Hvordan skal jeg få til å velge mellom menyelementene? Jeg får den til å lese strengen "initMenu[]", men der stopper det.
 * Funskjonen "initGUI" kalles fra loop
 */
 
void initGUI(){
  TFTscreen.fill(0,0,0);
//  short maximumLines=3;
  TFTscreen.setCursor(0,32);
  TFTscreen.setTextSize(2);
  TFTscreen.stroke(255,255,255);
  for(int m=0; m<=1; m++) {  
    if (m==menuPosition){
      TFTscreen.stroke(0,255,255);
      TFTscreen.print(" >");
      Serial.print(" >");
      TFTscreen.println(initMenu[m]);
      Serial.println(initMenu[m]);
      TFTscreen.stroke(255,255,255);

      }
    else {
      TFTscreen.print("  ");
      Serial.print("  ");
      TFTscreen.println(initMenu[m]);
      Serial.println(initMenu[m]);
    }
   }
//   break;
   wfe(); //ok. denne stoppet galskapen. ok. 
 }

char Button(){
  char pressed;
  for(;;){
    if (digitalRead(up)==1){
      waitTorelease();
      pressed=  'U';
      break;
    }
    else if (digitalRead(down)==1){
      waitTorelease();
      pressed=  'D';
      break;
    }
    else if (digitalRead(enter)==1){
      waitTorelease();
      pressed= 'E';
      break;
    }
    if (digitalRead(back)==1){
      waitTorelease();
      pressed=  'B';
      break;
    }
  }
  return pressed;
}

void waitTorelease(){
    for(;;){
      if (digitalRead(up)==0 && digitalRead(down)==0 && digitalRead(enter)==0 && digitalRead(back)==0 )
      break;
    }
}

  void showTXT(String txt){
    TFTscreen.stroke(255,255,255);
    TFTscreen.fill(50,50,50);
    TFTscreen.rect(0,0, TFTscreen.width(),TFTscreen.height());
    short newLineCounter=0;
    short data=0;
    char dataArray[100];

    TFTscreen.stroke(0,0,255);
    TFTscreen.setCursor(0,22);
    TFTscreen.print("    ");
    TFTscreen.println(txt);
    TFTscreen.stroke(0,255,255);


    String dataString = "";
    TFTscreen.print("   ");


        TFTscreen.print(dataString);
        Serial.print(dataString);
 /*       if (x=='\n'){
          newLineCounter++;
          TFTscreen.print("   ");
          }
        if (newLineCounter>1){
          data++;
          dataArray[data]=x;
          if (dataArray[data]=='c'||dataArray[data]=='t'){
            dataExecutionCounter++;
            dataToexecute[dataExecutionCounter]=dataString.toInt();
            dataString="";
            }
          else{
            dataString+=dataArray[data];
            }


        }

      }
    dataFile.close();
  } */
//  else {
//    Serial.println(txt);

    wfe();
//    }
//  Serial.println("");
//  Serial.print("{");
  for (int ri=1; ri<=data; ri++){
//    Serial.print(dataArray[ri]);
    }
//  Serial.println("}");
//  Serial.println("Converted into integers");
  for (int ri=1; ri<=dataExecutionCounter; ri++){
//    Serial.print(dataToexecute[ri]);
    }
//  GUI();
//  tempGUI();
    }

void wfe(){   // wait for ever
  for(;;){

        }
  }

void manualRunMotor(){
    unsigned long dt;
    short dirTime(4200);

    digitalWrite(EN, LOW); //Denne starter motoren
  
/*  Her kommer kode som jeg har skrevet for å få motoren
 *  skifte retning ca hver halvannen omgang.
 */
      unsigned long dirCounter=millis();
      digitalWrite(DIR, LOW);
      for(dirCounter=1; dirCounter<=dt; dirCounter++) {
        digitalWrite(stp, HIGH); //Trigger one step forward
        delay(1);
        digitalWrite(stp, LOW); 
        }

      digitalWrite(DIR, HIGH);
      for(dirCounter=1; dirCounter<=dt; dirCounter++) {
        digitalWrite(stp, HIGH); //Trigger one step forward
        delay(1);
        digitalWrite(stp, LOW); 
        }
/*  
 *  Min kode går hit. 
 */
}
void resetStepper(){
  digitalWrite(stp, LOW);
  digitalWrite(EN, HIGH);
}

/*
void initTXT(){
  
}
*/
