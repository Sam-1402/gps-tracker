/* This code works with SIM800L Evb version, DHT11 and MLX90614
 * It sets the module on receiving SMS mode, wait if the user has sent a SMS containing keywords "DHT" or "MLX"
 * Check which one is received then proceed to get data from the chosen sensor and send it via SMS to the programmed phone number
 * And come back to receiving mode.
 * Refer to www.SurtrTech.com for more detaims
 */
#include <TinyGPS++.h>
#include <SoftwareSerial.h>        //Libraries required for Serial communication, i²c communication, DHT11 and MLX90614
#include <AltSoftSerial.h>

char Received_SMS;              //Here we store the full received SMS (with phone sending number and date/time) as char
short GET_OK=-1;      //Used later it shows if there's the word "DHT"/"MLX" within the received SMS "-1" means they are not found

String Data_SMS;              //Here's the SMS that we gonna send to the phone number, it may contain DHT data or MLX data
double Latitude, Longitude;
String latitude="", longitude="";
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;

AltSoftSerial GPS;
SoftwareSerial sim800l(3, 2);                     // RX,TX for Arduino and for the module it's TXD RXD, they should be inverted

void setup()
{
  delay(3000);
  Serial.begin(115200);
  sim800l.begin(9600);   //Begin all the communications needed Arduino with PC serial and Arduino with all devices (SIM800L+DHT+MLX)
  GPS.begin(9600);
  delay(1000);
  Serial.println("Starting ...");
  delay(3000);         //Delay to let the module connect to network, can be removed
  ReceiveMode();       //Calling the function that puts the SIM800L moduleon receiving SMS mode
  
}



void loop() {
  
  String RSMS;             //We add this new variable String type, and we put it in loop so everytime gets initialized
                           //This is where we put the Received SMS, yes above there's Recevied_SMS variable, we use a trick below
                           //To concatenate the "char Recevied_SMS" to "String RSMS" which makes the "RSMS" contains the SMS received but as a String
                           //The recevied SMS cannot be stored directly as String
  
    while(sim800l.available()>0){       //When SIM800L sends something to the Arduino... problably the SMS received... if something else it's not a problem
        
        Received_SMS=sim800l.read();  //"char Received_SMS" is now containing the full SMS received
        Serial.print(Received_SMS);   //Show it on the serial monitor (optional)     
        RSMS.concat(Received_SMS);    //concatenate "char received_SMS" to RSMS which is "empty"
        GET_OK=RSMS.indexOf("GET");   //And this is why we changed from char to String, it's to be able to use this function "indexOf"
        
    }
    
  if(GET_OK!=-1){
    getgps();
    Send_Data();                      //This function set the sending SMS mode, prepare the phone number to which we gonna send, and send "Data_SMS" String
    ReceiveMode();                   //Come back to Receving SMS mode and wait for other SMS
    
    GET_OK=-1;                      //If the GET is found the variable should be reset to -1 otherwise it will be kept as !=-1 and will send SMS over and over                     //Maybe not required... I did a lot of tests and maybe at the beginning the RSMS string kept concating and MLX word was kept there
                                    //And at this point I'm too lazy to reupload the code without it and test...
  }
}


void Serialcom() //This is used with ReceiveMode function, it's okay to use for tests with Serial monitor
{
  delay(500);
  while(Serial.available())                                                                      
  {
    sim800l.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(sim800l.available())                                                                      
  {
    Serial.write(sim800l.read());//Forward what Software Serial received to Serial Port
  }
}

String double_string_con(double input)
{
  String storag1 = "";
  int count=0, count2=0;
  String storag2="";
  char dot='.';
  String val_string="";

  
  storag2=(String)input;
  storag1= (String)(input*1000000);
  //Serial.println(b);
  //Serial.println(j);

  for(int i=0; i<6; i++)
    {
     
     if(storag2.charAt(i)==dot) break;
     count++;
     
    }

  for(int i=0; i<15; i++)
    {
     
     if(storag1.charAt(i)==dot) break;
     count2++;
     
    }
  //Serial.println(count2);
  
  for(int i=0; i<count2; i++)
    {
      if(i==count) 
           {
            val_string = val_string + dot ;
           }
           val_string = val_string + storag1.charAt(i);
    
    }
   
   count=0;
   count2=0;
   return val_string;
}
void getgps(){
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;)
  {
    while (GPS.available())
    {
      if (gps.encode(GPS.read()))
      {
        newData = true;
      }
    }
  }
  if (newData)      //If newData is true
  {
        Data_SMS = "Latitude= "; 
        Latitude = gps.location.lat(), 6;
        latitude = double_string_con(Latitude);
        Data_SMS = Data_SMS + latitude;
        Data_SMS = Data_SMS + "\n";

        Data_SMS += " Longitude= "; 
        Longitude = gps.location.lng(), 6;
        longitude = double_string_con(Longitude);
        Data_SMS = Data_SMS + longitude;
        Data_SMS = Data_SMS + "\n";
    
        Data_SMS =  Data_SMS + "https://www.google.com/maps/search/?api=1&query=";
        Data_SMS =  Data_SMS + latitude;
        Data_SMS =  Data_SMS + ",";
        Data_SMS =  Data_SMS + longitude;
        newData = false;
  }
}

void ReceiveMode(){       //Set the SIM800L Receive mode
  
  sim800l.println("AT"); //If everything is Okay it will show "OK" on the serial monitor
  Serialcom();
  sim800l.println("AT+CMGF=1"); // Configuring TEXT mode
  Serialcom();
  sim800l.println("AT+CNMI=2,2,0,0,0"); //Configure the SIM800L on how to manage the Received SMS... Check the SIM800L AT commands manual
  Serialcom();
}

void Send_Data()
{
  Serial.println("Sending Data...");     //Displays on the serial monitor...Optional
  sim800l.print("AT+CMGF=1\r");          //Set the module to SMS mode
  delay(100);
  sim800l.print("AT+CMGS=\"+917017936738\"\r");  //Your phone number don't forget to include your country code example +212xxxxxxxxx"
  delay(500);  
  sim800l.print(Data_SMS);  //This string is sent as SMS
  delay(500);
  sim800l.print((char)26);//Required to tell the module that it can send the SMS
  delay(500);
  sim800l.println();
  Serial.println("Data Sent.");
  delay(500);

}
