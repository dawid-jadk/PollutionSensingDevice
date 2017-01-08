#include <GasSensor.h> //include gas library
#include <SoftwareSerial.h> //include SoftwareSerial library
#include <TinyGPS.h> //include GPS library

unsigned int GAS_PIN = 0; //analog pin of the gas sensor
int GAS_CO_VALUE, GAS_CH4_VALUE, GAS_LPG_VALUE; //variable for storing gas sensor data
float LAT_COORDS, LONG_COORDS; //variable for storing location data
int year; //variable to store year from GPS
byte month, day, hour, minute, second; //variable to store date/time data from GPS
int GPS_DATA_RECEIVED = 0; //variable to check if GPS data has been received
int GPRS_DATA_SENT = 0; //variable to check if GSM data has been received

unsigned char buffer[64]; //GPRS buffer size
int count=0; //while loop counter for the GPRS buffer

unsigned long theTimer; //variable of timer
unsigned long repeatInterval = 30000; // defining the amount of time at which the data will be sent

/* - Gas sensor related macros such as curves are defined in GasSensor.h of the library.
   - Curves and other macros have been obtained from the gas sensor datasheet
   - Link to datasheet: http://www.seeedstudio.com/wiki/images/f/f2/MQ-9.pdf
*/ 

GasSensor gas = GasSensor(GAS_PIN); //create and initialise an object of the GasSensor class
TinyGPS gps; //create an object of TinyGPS class
SoftwareSerial serialGPS(4,3); //set serial communication of digital pins (4,3) for GPS
SoftwareSerial gprs(7, 8); //set serial communication of digital pins (7,8) for GPRS/GSM

//initial method
void setup() {
  Serial.begin(9600); //initialise the serial communication port and set the baud rate to 9600bps
  serialGPS.begin(9600); //initialise serial communication for GPS on 9600bps
  gprs.begin(9600); //initialise serial communication for GSM/GPRS on 9600bps
  theTimer = millis(); //initialise/start the timer/millis returns the number of milliseconds since the arduino began running (will go back to 0 after approx. 50 days)
  Serial.println("Initialising the hardware...");
  Serial.println("Calibrating gas sensor...");
  gas.calibrate(); //start gas sensor calibration (to be done in clean air environment)
  Serial.println("Calibration done");
}

//loop/execution method
void loop() {
  getGas(); //call the method to produce the gas values
  
 //if the timer time has passed
 if ((millis()-theTimer) > repeatInterval) {  
     theTimer += repeatInterval; // reset timer by setting it to the next interval
     Serial.println("Waiting for GPS...");
     //GPS getter has to be in while loop since we have to make multiple requests before we can get the data from it
     while(GPS_DATA_RECEIVED == 0){
      getGPS(); //get GPS data
     }
      Serial.println("Uploading data...");
      uploadData(); //upload data to the server
 }
}

//method use to get the gas values
void getGas() {
  //if statement to prevent getting concentations over 1000PPM
  if(gas.getPPM(GAS_CO) > 1000){
    GAS_CO_VALUE = 1000;
  } else {
    GAS_CO_VALUE = gas.getPPM(GAS_CO); //gets the specific gas value in PPM
  }
  if(gas.getPPM(GAS_CH4) > 1000){
    GAS_CH4_VALUE = 1000;
  } else {
     GAS_CH4_VALUE = gas.getPPM(GAS_CH4); //gets the specific gas value in PPM
  }
  if(gas.getPPM(GAS_LPG) > 1000){
    GAS_LPG_VALUE = 1000;
  } else {
    GAS_LPG_VALUE = gas.getPPM(GAS_LPG); //gets the specific gas value in PPM
  }
}

//method to get GPS data
void getGPS() {
 //listen to GPS device
 serialGPS.listen();
  //if GPS has made a connection with satelites
  while(serialGPS.available()) {
  int readGPS = serialGPS.read(); //reads incoming serial data from GPS module
  //encode the data
  if(gps.encode(readGPS)) {
    gps.f_get_position(&LAT_COORDS, &LONG_COORDS); //store GPS location coordinates in variables (using pointers)
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second); //store GPS date/time data
    Serial.println("GPS DATA RECEIVED");
    GPS_DATA_RECEIVED = 1; //set the value to 1 stop GPS calls  
    GPRS_DATA_SENT = 0; //reset to allow GPRS requests     
  }
 }
}

//method to send the GPRS/GSM requests
void sendGPRSRequest(){
 if (gprs.available()){ // if there is some data to be received from GPRS/GSM
    while(gprs.available()){ //while there is a data read it into char array 
      buffer[count++]=gprs.read(); //writing data into array
      if(count == 64)break;
    }
  Serial.write(buffer,count); //write buffer to serial port
  clearBufferArray(); //clearBufferArray function to clear data from the array
  count = 0;  //reset counter of while loop
}
}

//method which executes the AT Commands to send the data to the server
void uploadData() {
  gprs.listen();  //listen to GPRS/GSM device
  gprs.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");   //set the connection to GPRS
  delay(500); //wait 0.5sec
  sendGPRSRequest(); //send request
  
  gprs.println("AT+SAPBR=3,1,\"APN\",\"everywhere\"");  //set the APN to SIM CARD specific (EE in this case)
  delay(500);
  sendGPRSRequest(); //send request
  
  gprs.println("AT+SAPBR =1,1"); //enable GPRS
  delay(500);
  sendGPRSRequest(); //send request
  
  gprs.println("AT+SAPBR=2,1"); //test is the connection is setup properly (if we get back a IP address it is)
  delay(500);
  sendGPRSRequest(); //send request
    
  gprs.println("AT+HTTPINIT");//enable HTTP
  delay(500);
  sendGPRSRequest(); //send request
  
  gprs.println("AT+HTTPPARA=\"CID\",1");//set up HTTP bearer profile identifier
  delay(500);
  sendGPRSRequest(); //send request
  
  gprs.print("AT+HTTPPARA=\"URL\",\"http://YOU WEBHOST GOES HERE/add_data.php");//set the url 
  delay(500);
  sendGPRSRequest(); //send request

  gprs.print("?gas1="); //set next part of URL
  gprs.print(GAS_CO_VALUE); //get data from variable
  delay(500);
  sendGPRSRequest(); //send request

  gprs.print("&gas2="); //set next part of URL
  gprs.print(GAS_CH4_VALUE); //get data from variable
  delay(500);
  sendGPRSRequest(); //send request
  
  gprs.print("&gas3="); //set next part of URL
  gprs.print(GAS_LPG_VALUE); //get data from variable
  delay(500);
  sendGPRSRequest(); //send request

  char data[32];
  sprintf(data, "%02d-%02d-%02d %02d:%02d:%02d", //format the date/time
  year, month, day, hour+0x01, minute, second); //hour has to be increased by 1 because of GPS getting DST time
  gprs.print("&time="); //set next part of URL
  gprs.print(data); //get data from variable
  delay(500);
  sendGPRSRequest(); //send request

  gprs.print("&lat="); //set next part of URL
  gprs.print(LAT_COORDS,5); //get data from variable
  delay(500);
  sendGPRSRequest(); //send request
  
  gprs.print("&lng="); //set next part of URL
  gprs.print(LONG_COORDS,5); //get data from variable
  gprs.print("\"");
  delay(500);
  sendGPRSRequest(); //send request

  gprs.println();
  delay(500);
  sendGPRSRequest(); //send request

  Serial.write(0x1A);//the ASCII code (in order to send the URL)
  delay(500);
  sendGPRSRequest(); //send request
 
  gprs.println("AT+HTTPACTION=0"); //start HTTP GET session
  delay(500);
  sendGPRSRequest(); //send request

  GPS_DATA_RECEIVED = 0; //reset the variable to allow further GPS calls
  GPRS_DATA_SENT == 1; //stop sending GPRS data
}

//function to clear buffer array
void clearBufferArray(){ 
  for (int i=0; i<count;i++){
    buffer[i]=NULL;  //clear all array with NULL
  }
}
