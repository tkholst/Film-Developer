#include <SPI.h>
#include <SD.h>
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
//aktiverer denne først (ONE_WIRE_BUS):
#define ONE_WIRE_BUS A0
TFT TFTscreen = TFT(lcd_cs, dc, rst);
/*
 * prøver så disse følgende for å se om de gjør noe galt med sketchen
 * først den ene, så den andre
 */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfFiles=0;
short currentPosition=1;
const short up=2,down=3,enter=4,back=5;
String fileName[20];
short runTemp;
float temperature=0;
int dataToexecute[10];
char BS; // Button State;
short temperatureTolerence=5;// if it is set to 3, motor will be turned on +-3 degree centigrade of the temperature written in txt file
short dataExecutionCounter=0;
PImage logo;


void setup() {
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
  resetStepper();

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  //Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    //Serial.println("initialization failed!");
    while (1);
  }
  //Serial.println("initialization done.");

  root = SD.open("/");
  printDirectory(root, 0);

//  Serial.println("done!");
//  Serial.print("Number of files: ");
//  Serial.println(numberOfFiles);




    TFTscreen.fill( 210, 210,198);
    TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
    logo = TFTscreen.loadImage("LOGO2.bmp");
//    logo = TFTscreen.loadImage("LOGO.bmp");
//  if (!logo.isValid()) {
//    Serial.println(F("error while loading logo"));
//  }
  if (logo.isValid() == false) {
    return;
  }
  TFTscreen.image(logo, 0,0);
  delay(3000);
  TFTscreen.fill( 0,0,0);
  TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
}

void loop() {
  //Serial.println(currentPosition);
  int entered=0;
  GUI();
  fileGUI();
  /*
   * tempGUI kan ikke kalles opp om den ikke er aktiv. Så må se på den først
   */
  tempGUI();
  BS=Button();
  if (BS=='D'){
      currentPosition++;
      if (currentPosition>numberOfFiles)
        currentPosition=1;
      }
   else if (BS=='U'){
      currentPosition--;
      if (currentPosition<1)
        currentPosition=numberOfFiles;;
      }
  else if (BS=='E'){
      entered=1;
      showTXT(fileName[currentPosition]);
      for(;;){
        tempGUI();
        BS=Button();
        if (BS=='B'){
            break;
            }
        else if(BS=='E'){
          askingGUI();
          for(;;){
            tempGUI();
            BS=Button();
            if (BS=='B'){
              break;
              }
            else if (BS=='E'){
              checkingGUI();
              startingGUI();


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
//void logoGUI(){
//    TFTscreen.fill( 210, 210,198);
//    TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
//    logo = TFTscreen.loadImage("LOGO.bmp");
////  if (!logo.isValid()) {
////    Serial.println(F("error while loading arduino.bmp"));
////  }
//  if (logo.isValid() == false) {
//    return;
//  }
//  TFTscreen.image(logo, 8,1);
//  delay(1500);
//  TFTscreen.fill( 0,0,0);
//  TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
//  }


/*
 * printDirectory lister opp filene fra sd-kortet på skjermen
 */
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }

    numberOfFiles++;
    fileName[numberOfFiles]=entry.name();


    //Serial.println(entry.name());
    char x=fileName[numberOfFiles][fileName[numberOfFiles].length()-1];
    //Serial.println(x) ;
    if (x!='T'){
      //Serial.println(":(");
      numberOfFiles--;
      }
    ////Serial.println((fileName[numberOfFiles][fileName[numberOfFiles].length()-1])) ;
    //TFTscreen.println(entry.name());
    entry.close();
  }
}

/*
 * GUI gjør menylistinga på skjermen valgbar
 */
void GUI(){
  TFTscreen.stroke(255,255,255);
  TFTscreen.fill(255,0,0);
  TFTscreen.rect(0,0, TFTscreen.width(),20);
  TFTscreen.setTextSize(2);
  TFTscreen.text("FILM DEV",35,3);
  TFTscreen.fill(0,200,0);
  TFTscreen.rect(0,110, TFTscreen.width(),TFTscreen.height()-110);
  TFTscreen.line( TFTscreen.width()/2,110, TFTscreen.width()/2,TFTscreen.height());
  TFTscreen.setTextSize(1);
  TFTscreen.text("SELECT",25,115);
  TFTscreen.text("BACK",105,115);
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
/*    
 *     Så har vi denne rakkaren her da. Skal se om jeg bare kan lese temperaturen. PIDen gjør likevel styringa av tempen. 
 *     Jeg aktiverer den. Den ser ut til å bare lese av tempen. Ok, ting ser ut til å funke, bortsett fra at temperatur-
 *     avlesinga er helt borti ræva.
 */

 void tempGUI(){

//  temperature= analogRead(A0);
//  Serial.println(temperature);
//  temperature=182-temperature*0.28;/  // This is calibration calculation
//  temperature=(temperature-32)*5;     // converting from F to C
//  temperature=temperature/9;          // converting from F to C
//  Serial.println(temperature);

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
//  temperature=(temperature-32)*5/9;

  TFTscreen.stroke(0,0,0);
  TFTscreen.fill(0,0,0);
  TFTscreen.rect(90,85,70,20);
  TFTscreen.stroke(50,50,255);
  TFTscreen.setCursor(90,90);
  TFTscreen.setTextSize(2);
  TFTscreen.print(temperature);
  TFTscreen.setCursor(150,87);
  TFTscreen.setTextSize(1);
  TFTscreen.print("o");

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


    File dataFile = SD.open(txt);
    String dataString = "";
    TFTscreen.print("   ");

    if (dataFile) {
      while (dataFile.available()) {
        char x=dataFile.read();
        Serial.print(x);

        TFTscreen.print(x);
        if (x=='\n'){
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
  }
  else {
//    Serial.println(txt);

    wfe();
    }
//  Serial.println("");
//  Serial.print("{");
  for (int ri=1; ri<=data; ri++){
//    Serial.print(dataArray[ri]);
    }
  //Serial.println("}");
  //Serial.println("Converted into integers");
  for (int ri=1; ri<=dataExecutionCounter; ri++){
//    Serial.print(dataToexecute[ri]);
    }
  GUI();
  tempGUI();
    }

// Okey, this one Waits For Ever :) - or until someone press a button
void wfe(){
  for(;;){

        }
  }

// Ask to start agitation
void askingGUI(){
  TFTscreen.stroke(255,255,255);
  TFTscreen.fill(0,0,0);
  TFTscreen.rect( 0,(TFTscreen.height()/2), TFTscreen.width(),20);
  TFTscreen.setTextSize(2);
  TFTscreen.text("Start Motor?",5+TFTscreen.width()*(3/4),(TFTscreen.height()/2)+2);
  }

void checkingGUI(){
//  TFTscreen.stroke(255,255,255);
//  TFTscreen.fill(0,0,0);
//  TFTscreen.rect( 0,(TFTscreen.height()/2), TFTscreen.width(),20);
//  TFTscreen.setTextSize(2);
//  TFTscreen.text("Checking",5,(TFTscreen.height()/2)+2);
//  TFTscreen.print(".");
//  delay(500);
//  TFTscreen.print(".");
//  delay(500);
//  TFTscreen.println(".");

  }
/*
 * Denne funksjonen starter motoren. Temperatursjekken som skulle sjekket at den er innenfor tillatt temp har
 * jeg deaktivert. Når den er aktiv, fungerer ikke lesing av sd-kortet. Så inntil videre: deaktivert. 
 */
// Here it runs the motor
void startingGUI()/*{
  float tempCheck=temperature-dataToexecute[1];
  if (tempCheck<0){
    tempCheck=-1*tempCheck;
    }
    //                                                                                                       tempCheck=.5;
  if (tempCheck>temperatureTolerence){
    TFTscreen.stroke(0,0,255);
    TFTscreen.fill(0,0,0);
    TFTscreen.setTextSize(2);
    TFTscreen.rect( 0,(TFTscreen.height()/2)-40, TFTscreen.width(),60);
    TFTscreen.setCursor(5,(TFTscreen.height()/2)-38);
    TFTscreen.println("ERROR");
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(255,255,255);
    TFTscreen.print(" Safe temp:");

    TFTscreen.println(dataToexecute[1]);
    TFTscreen.print(" Current Temp:");
    TFTscreen.println(temperature);
    //TFTscreen.print("try");
    //delay(5000);
    BS=Button();
    TFTscreen.stroke(255,255,255);
    TFTscreen.fill(50,50,50);
    TFTscreen.rect(0,0, TFTscreen.width(),TFTscreen.height());
    TFTscreen.setTextSize(2);
    TFTscreen.setCursor(0,10);
    TFTscreen.println(" Reboot");
    TFTscreen.println(" again");
    wfe();
    }
  else*/{
    TFTscreen.stroke(200,200,0);
    TFTscreen.fill(0,0,0);
    TFTscreen.setTextSize(2);
    TFTscreen.rect( 0,(TFTscreen.height()/2)-45, TFTscreen.width(),90);
    TFTscreen.setCursor(5,(TFTscreen.height()/2)-43);
    TFTscreen.println("EXECUTING...");
    //TFTscreen.setTextSize(1);
    for(int rl=2;rl<=dataExecutionCounter;rl++){
      TFTscreen.setCursor(0,(TFTscreen.height()/2)-28);
      for (int rk=2;rk<=dataExecutionCounter;rk++){
        if (rk<=rl){
          TFTscreen.stroke(0,255,0);
          TFTscreen.print(dataToexecute[rk]);
          }
        else{
          TFTscreen.stroke(255,255,255);
          TFTscreen.print(dataToexecute[rk]);
          }
         TFTscreen.print("  ");
         if (rk%6==0){
          TFTscreen.println("");
          }

      }
    if (rl>2){
      for(;;){
        BS=Button();
        if (BS=='E'){
          break;
          }
        }
      }


    runMotor(dataToexecute[rl]);


    }
    TFTscreen.stroke(0,255,0);
    TFTscreen.fill(0,0,0);
    TFTscreen.setTextSize(3);
    TFTscreen.rect( 0,(TFTscreen.height()/2)-45, TFTscreen.width(),90);
    TFTscreen.setCursor(10,(TFTscreen.height()/2)-25);
    TFTscreen.println("FINISHED");
    BS=Button();
//    loop();

    TFTscreen.stroke(255,255,255);
    TFTscreen.fill(50,50,50);
    TFTscreen.rect(0,0, TFTscreen.width(),TFTscreen.height());
    TFTscreen.setTextSize(2);
    TFTscreen.setCursor(8,40);
    TFTscreen.println(" Reboot now!");
//    TFTscreen.println(" again");
    wfe();
//    }
  }

// This function actually turns the motor, but sadly so far only in one direction
  void runMotor( int u){
    unsigned long dt;
    unsigned long t=u;
    Serial.println(t);
//  short stepTime=10000;
    short dirTime(4500);
// Legg inn *60 i neste linje om du bruker hele minutter på SD-kortet slik: t=t*60*1000;   
     t=t*1000;
     dt=dirTime;
//     Serial.println(t);
    Serial.println("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx:                ");
    Serial.println("MotorStart");
    digitalWrite(EN, LOW);
    unsigned long timeCounter=millis();
    //unsigned long preTimeCounter=timeCounter;
    for(/*int rm=1; rm<=t; rm++*/;;){
      
/*  Her kommer kode som jeg har skrevet for å få motoren
 *  til å skifte retning mens den går. Nå ser det ut til å fungere som det skal, 
 *  men den fullfører sitt eget løp, slik at det kan bli inntil 9 sek påslag på
 *  tiden. Ikke så veldig kritisk, tenker jeg.
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
/*  Min kode går hit.
 *   
 *  Her kommer den originale
 *  koden fra Bangla:

//      digitalWrite(stp,HIGH); //Trigger one step forward
//      delay(1);
//      digitalWrite(stp,LOW);
 */

      unsigned long executed= millis()-timeCounter;

      Serial.println(executed);
      if (executed>t){
        break;
        }
      }
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);
    Serial.println("MotorStop");
    resetStepper();
    delay(5000);
    }


void resetStepper(){
  digitalWrite(stp, LOW);
  digitalWrite(EN, HIGH);
}

void fileGUI(){
  TFTscreen.fill(0,0,0);
  short maximumFileOnScreen=10;
  TFTscreen.setCursor(0,22);
  TFTscreen.setTextSize(1);
  TFTscreen.stroke(255,255,255);
  for(int i=1;i<=maximumFileOnScreen;i++){
    if (i==currentPosition){
      TFTscreen.stroke(0,0,255);
      TFTscreen.print("  ");
      TFTscreen.println(fileName[i]);
      TFTscreen.stroke(255,255,255);

      }
    else {
      TFTscreen.print("  ");
      TFTscreen.println(fileName[i]);
    }

   }
 }
