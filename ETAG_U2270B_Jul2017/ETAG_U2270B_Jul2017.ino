
/*
   Alexander Moreno
   David Mitchell
   Eli Bridge
   July-2017
*/

//INITIALIZE INCLUDE FILES AND I/O PINS
#include "logger.h"      //include the libarary for reading and parsing raw RFID input
#include <Wire.h>        //include the standard wire library - used for I2C communication with the clock
#include <SD.h>          //include the standard SD card library
#define Serial SerialUSB     //Designate the USB connection as the primary serial comm port
#define DEMOD_OUT_PIN   30   //(PB03) this is the target pin for the raw RFID data
#define SHD_PINA         8   //(PA06) Setting this pin high activates the primary RFID circuit (only one can be active at a time)
#define SHD_PINB         9    //(PA07) Setting this pin high activates the seconday RFID circuit (only one can be active at a time)
#define MOD_PIN          0    //not used - defined as zero
#define READY_CLOCK_PIN  0    //not used - defined as zero
#define chipSelect       7    //Chip Select for SD card - make this pin low to activate the SD card
#define LED_RFID        31    //Pin to control the LED indicator.  
logger L(DEMOD_OUT_PIN, MOD_PIN, READY_CLOCK_PIN); //Designate pins used in the looger library

// initialize variables
byte tagData[5];                  //Variable for storing RFID tag codes
unsigned long currentMillis;      //Used for exploiting the millisecond counter for timing functions - stores a recent result of the millis() function
unsigned long stopMillis;         //Used for exploiting the millisecond counter for timing functions - stores a less recent result of the millis() function
byte RFcircuit = 1;               //Used to determine which RFID circuit is active. 1 = primary circuit, 2 = secondary circuit. 
unsigned int serDelay;            //Used for timing on initiation of serial communication
byte ss, mm, hh, da, mo, yr;          //Byte variables for storing date/time elements
String sss, mms, hhs, das, mos, yrs;  //String variable for storing date/time text elements
String timeString;                    //String for storing the whole date/time line of data
byte incomingByte = 0;                 //Used for incoming serial data
unsigned int timeIn[12];              //Used for incoming serial data during clock setting

//CONSTANTS (SET UP LOGGING PARAMETERS HERE!!)
const unsigned int polltime = 3000;       //How long in milliseconds to poll for tags 
const unsigned int pausetime = 500;       //How long in milliseconds to wait between polling intervals 
const unsigned int readFreq = 200;        //How long to wait after a tag is successfully read.

void setup() {  // This function sets everything up for logging.
  pinMode(SHD_PINA, OUTPUT);     // Make the primary RFID shutdown pin an output.
  digitalWrite(SHD_PINA, LOW);   // turn the primary RFID circuit off (HIGH turns it on)
  pinMode(SHD_PINB, OUTPUT);
  digitalWrite(SHD_PINA, LOW);   // turn the secondary RFID circuit off
  pinMode(LED_RFID, OUTPUT);
  digitalWrite(LED_RFID, HIGH);  // turn the LED off (LOW turns it on)
  initclk();                     // Calls a function to start the clock ticking if it wasn't doing so already

  //Try to initiate a serial connection
  Serial.begin(9600);               //Initiate a serial connection with the given baud rate
  ss = 0;                           //Initialize a variable for counting in the while loop that follows
  while (ss < 5 && !Serial) {       //Loop to wait for Serial connection to come online
     delay(500);                    //Flash LED while waiting
    digitalWrite(LED_RFID, LOW);      // turn the LED on (LOW turns it on)
    delay(500);
    digitalWrite(LED_RFID, HIGH);     // turn the LED off (HIGH turns it off)
    ss = ss + 1;                    //add to counting variable
  }//end while
  digitalWrite(LED_RFID, HIGH);     // make sure LED is off 
   Serial.println("set clock?  Y or N.");             //Ask if the user wants to set the clock
   serDelay = 0;                                      //If there's no response then just start logging
   while(Serial.available() == 0 && serDelay < 10000) //wait about 10 seconds for a user response
    {
      delay(1);                                     
      serDelay++;
    } 
    if (Serial.available()){           //If there is a response check to see if it is "Y"
      incomingByte = Serial.read();
    } 
    if(incomingByte == 89) {           //"Y" is the ascii character equivalent of 89
      setClk();                        //if the user enters "Y" (89) then call the clock setting function
      }
    
  //Set up the SD card
  Serial.print("Initializing SD card...\n");    //message to user
  if (!SD.begin(chipSelect)) {                  //Initiate the SD card function with the pin that activates the card.
    Serial.println("\nSD card failed, or not present");   //SD card error message
    return;
  }// end check SD
  RFcircuit = 1;
  Serial.println("Scanning for tags...\n");
} // end setup

void loop() {
  if (RFcircuit == 1) 
    {digitalWrite(SHD_PINA, HIGH);} //Turn on primary RFID circuit
    else 
    {digitalWrite(SHD_PINB, HIGH);} //Turn on secondary RFID circuit
  Serial.print("Scanning RFID circuit ");
  Serial.println(RFcircuit);
    
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  currentMillis = millis();
  stopMillis = currentMillis + polltime;
  while(stopMillis > millis()) {
    if (L.scanForTag(tagData) == true) {
      Serial.print("RFID Tag Detected: "); //print a header to the Serial port.
      getTime();
      for (int n = 0; n < 5; n++) {             //For loop to send tag data over serial comm one byte at a time
        if (tagData[n] < 10) Serial.print("0"); //send a leading zero if necessary
        Serial.print(tagData[n], HEX);          //Send byte
      }
      Serial.print(" on antenna ");  // add a note about which atenna was used
      Serial.print(RFcircuit);
      Serial.print(" at ");           // add period and complete text line
      Serial.println(timeString);
          
      for (unsigned int n = 0; n < readFreq ; n = n + 30) {      //loop to Flash LED and delay reading after getting a tag
           digitalWrite(LED_RFID, LOW);
           delay(5);
           digitalWrite(LED_RFID, HIGH);
           delay(25);
          }
      
      //delay(readFreq); //Delay between read attempts
      
      
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (dataFile) {
        for (int n = 0; n < 5; n++) {               //loop to print out the RFID code
          if (tagData[n] < 10) dataFile.print("0"); //add a leading zero if necessary
          dataFile.print(tagData[n], HEX);          //print to the SD card
          }
        dataFile.print(",");                        //comma for data delineation
        dataFile.print(RFcircuit);                  //log which antenna was active
        dataFile.print(",");                        //comma for data delineation
        dataFile.println(timeString);
        dataFile.close();
        Serial.println("saved to SD card.");
      } // check dataFile is present
      else {
        Serial.println("error opening datalog.txt");
      }// end check for file
    
    } // end ScanForTag
  } //end while
    digitalWrite(SHD_PINA, LOW); //Turn off both RFID circuits
    digitalWrite(SHD_PINB, LOW); //Turn off both RFID circuits
    delay(pausetime);
    if (RFcircuit == 1)  // switch between active RF circuits.
      {RFcircuit = 2;}
      else
      {RFcircuit = 1;}
    //RFcircuit = 1;              //comment out or delete to use both circuits
    //currentMillis = millis();
    //stopMillis = currentMillis + polltime;
}// end void loop


byte bcdToDec(byte val)  {
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

byte decToBcd( byte val ) {
// Convert decimal to binary coded decimal
   return (byte) ((val / 10 * 16) + (val % 10));
}

static uint8_t conv2d(const char* p) {
// Convert parts of a string to decimal numbers
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

void initclk () {   //Start the clock running if it is not already
  Wire.begin();                  //Start up the I2C comm funcitons
  Wire.beginTransmission(0x68);  //Send the clock slave address
  Wire.write(0x0C);              //address for clearing the HT (halt time?) bit
  Wire.write(0x3F);              //HT is bit 6 so 00111111 clears it.
  Wire.endTransmission();
}

void setClk() {   //Function to set the clock)
  Serial.println("Enter mmddyyhhmmss");      //Ask for user input
   while(Serial.available() == 0) {} //wait for 12 characters to accumulate
   for (int n = 0; n < 13; n++) { 
      timeIn[n] = Serial.read();     //Read the characters from the buffer one at a time
   }
    while(Serial.available())        //Clear the buffer, in case there were extra characters
      {Serial.read();}

   mo = ((timeIn[0]-48)*10 + (timeIn[1]-48));    //Convert two ascii characters into a single decimal number
   da = ((timeIn[2]-48)*10 + (timeIn[3]-48));    //Convert two ascii characters into a single decimal number
   yr = ((timeIn[4]-48)*10 + (timeIn[5]-48));    //Convert two ascii characters into a single decimal number
   hh = ((timeIn[6]-48)*10 + (timeIn[7]-48));    //Convert two ascii characters into a single decimal number       
   mm = ((timeIn[8]-48)*10 + (timeIn[9]-48));    //Convert two ascii characters into a single decimal number
   ss = ((timeIn[10]-48)*10 + (timeIn[11]-48));  //Convert two ascii characters into a single decimal number

  //Now set the clock usind I2C protocols implemented in the Wire library
  initclk();
  Wire.beginTransmission(0x68);
  Wire.write(0x00);  //starting register - register 0
  Wire.write(0x00);  //write to register 0 - psecs (100ths of a second - can only be set to zero
  Wire.write(decToBcd(ss));  //write to register 1 - seconds
  Wire.write(decToBcd(mm));  //write to register 2 - minutes
  Wire.write(decToBcd(hh));  //write to register 3 - hours 
  Wire.write(0x01);          //write to register 4 - day of the week (we don't care about this)
  Wire.write(decToBcd(da));  //write to register 5 - day of month
  Wire.write(decToBcd(mo));  //write to register 6 - month
  Wire.write(decToBcd(yr));  //write to register 7 - year
  Wire.endTransmission();

  //When the clock is set (more specifically when Serial.read is used) the RFID circuit fails)
  //I don't know why this happens
  //Only solution seems to be to restart the devide. 
  //So the following messages inform the user to restart the device.
    getTime(); 
  Serial.print("Clock set to ");
  Serial.println(timeString);
  Serial.print("Restart reader to log data."); 
  while(1){}     //Endless while loop. Program ends here. User must restart.
}

void getTime()
{
      //Read in the time
      Wire.beginTransmission(0x68);
      Wire.write(0x01);    //start to read from register 1 (seconds)
      Wire.endTransmission();
      Wire.requestFrom(0x68, 7);
      if (Wire.available()) {
        ss = Wire.read();
        mm = Wire.read();
        hh = Wire.read();
        da = Wire.read(); //day of week gets overwritten in next line
        da = Wire.read();
        mo = Wire.read(); 
        yr = Wire.read(); 
    }

    sss = ss < 10 ? "0"+String(bcdToDec(ss), DEC) : String(bcdToDec(ss), DEC); //These lines convert decimals to characters to build a
    mms = mm < 10 ? "0"+String(bcdToDec(mm), DEC) : String(bcdToDec(mm), DEC); //string with the date and time 
    hhs = hh < 10 ? "0"+String(bcdToDec(hh), DEC) : String(bcdToDec(hh), DEC); //They use a shorthand if/then/else statement to add 
    das = da < 10 ? "0"+String(bcdToDec(da), DEC) : String(bcdToDec(da), DEC); //leading zeros if necessary
    mos = mo < 10 ? "0"+String(bcdToDec(mo), DEC) : String(bcdToDec(mo), DEC);
    yrs = yr < 10 ? "0"+String(bcdToDec(yr), DEC) : String(bcdToDec(yr), DEC);
    timeString = mos+"/"+das+"/"+yrs+" "+hhs+":"+mms+":"+sss;                  //Construct the date and time string
}
