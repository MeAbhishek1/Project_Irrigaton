///including library
#include<TimeLib.h>
#include<time.h>
#include<Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

///// Sensor and Actuator configurations///////////
const int Moisture_Water=A0;
const int motor=D0;
String status_pump="NAN";

/////////////////Variables for irrigation parameters//////////////

int  initial=20;
int  initial_moist=1;
int  develop=50;
int  develop_moist=4;
int  mid=95;
int  mid_moist=6;
int  late=120;
int  late_moist=2;
int elap_time=0;
int Moist_Value=0;

//including variable for user input

String crop_type="rice";
String soil_type="loamy";
String land_area="546";
String loc_lat="25.7878336";
String loc_lon="93.9164595";
String Place_Name;
String date="20042022";
int in_day=0;
int in_mon=0;
int in_year=0;

////////////weather forecast and sensor parameters variables
String Temp_val="34C";
String Light_val="Daylight";
String Moist_val="34%";
String Humid_val="54%";
String Rain_val="34%";
String Remark = "Everything is fine";
String sunrise="1650928342";

//////////////////credentials for web connectivity
const char* ssid="POCO X2";
const char* password="2018107017";
String apikey="bf51534ae6f852bea7f60ebb70a8c609";

///////////////////for time calculations///////////
tmElements_t T1;
time_t T1sec;
int time_elap=0;

////////////////////server variable///////////
WiFiServer server(80);
///////////////////////////////////////////////

void setup() {
    Serial.begin(9600);
  Serial.println("");
///////////////////pin setup//////////////
  pinMode(Moisture_Water,INPUT);
  pinMode(motor,OUTPUT);
  

  
////////////// connecting  to WIFI network//////////////
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  /////////////// Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(1000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
///////////////////////////////////////  Taking input from user//////////////////////////////////
  Serial.println("Hello");
  Serial.println("Please enter the Crop type");
  while(Serial.available()==0){}
  crop_type=Serial.readString();
  Serial.println("Please enter the Soil type");
  while(Serial.available()==0){}
  soil_type=Serial.readString();
  Serial.println("Please enter the Land Area");
  while(Serial.available()==0){}
  land_area=Serial.readString();
  Serial.println("Please enter the Latitude Location");
  while(Serial.available()==0){}
  loc_lat=Serial.readString();
  loc_lat.replace("\n","");
  Serial.println("Please enter the Longitude Location");
  while(Serial.available()==0){}
  loc_lon=Serial.readString();
  loc_lon.replace("\n","");
  Serial.println("Please Enter the Date:(ddmmyyyy) ");
  while(Serial.available()==0){}
  date=Serial.readString();
  
/////////////////Breaking the date into appropriate variable format/////////

  int n=date.length();
  String s="";
  s=String(date[0])+String(date[1]);
  Serial.println("day: ="+s);
  in_day=s.toInt();
  s=String(date[2])+String(date[3]);
  Serial.println("mon: ="+s);
  in_mon=s.toInt();
  s=String(date[4])+String(date[5])+String(date[6])+String(date[7]);
  Serial.println("year: ="+s);
  in_year=s.toInt();
  T1.Hour = 00; 
  T1.Minute = 00; 
  T1.Second = 00; 
  T1.Day = in_day;
  T1.Month = in_mon; 
  T1.Year = in_year - 1970;
////////////////////setting the initial time as per unix timestamp//////////////////////
  T1sec =  makeTime(T1);
  Serial.println("initial time: "+T1sec);
}


void loop() {
  /////////////////  Continuously reading the sensor data       ///////////
Moist_Value=analogRead(Moisture_Water);
Serial.print("\nMoisture_val from sensor= ");
Serial.println(Moist_Value);
////////////////////  WEb page nad server  configuration   //////////////////
WiFiClient client = server.available(); ;

client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("Connection: close");
client.println();
client.println("<!DOCTYPE html>");
client.println("<html>");
client.println("<head>");
client.println("<title>Automation</title>");
client.println("</head>");
client.println(".button {border: none; color: white; padding: 10px 20px; text-align: center;");
 client.println(".button1 {background-color: #13B3F0;}");
client.println("<body>");
client.println("<h1>Farm automation and Monitoring</h1>");
client.println("<hr>");
client.println("<h3>Crop Details</h3>");
client.println("<hr>");
client.println("<ul>");
client.println("<li>Crop Type: ");
client.println(crop_type);
client.println("</li>");
client.print("<li>Field Type:");
client.print(soil_type);
client.println("</li>");
client.print("<li>Field area: ");
client.print(land_area);
client.println("</li>");
client.println("<li>Place: ");
client.print(Place_Name);
client.print("<li>Lattitude: ");
client.print(loc_lat);
client.println("</li>");
client.print("<li>Longitude: ");
client.print(loc_lon);
client.println("</li>");
client.println("</li>");
client.println("</ul>");
client.println("<hr>");
client.println("<h3>Current Conditions</h3>");
client.println("<ul>");
client.println("<li>Temperature: ");
client.print(Temp_val);
client.println(" °C");
client.println("</li>");
client.println("<li>Light Condition: ");
client.print(Light_val);
client.println("</li>");
client.println("<li>Moisture: ");
client.print(Moist_Value);
client.println("</li>");
client.println("<li>Humidity: ");
client.print(Humid_val);
client.println(" %");
client.println("</li>");
client.println("<li>Rainfall: ");
client.print(Rain_val);
client.println(" %");
client.println("</li>");
client.println("</ul>");
client.println("<hr>");
client.println("<h3>Water State: </h3>");
client.println("<ul>");
client.println("<li>Pump Status: ");
client.println(status_pump);
client.println("</li>");
client.println("</ul>");
client.println("<hr>");
client.println("<h3>Remarks: </h3>");
client.println("<ul>");
client.println("<li>");
client.println("Total time elapsed (after sowing) is: ");
client.println(time_elap);
client.println(" days");
client.println("</li>");
client.println("</ul");
client.print("<p><button class=\"button button1\">Emergency Stop</button></a></p>");
client.println("</body></html>");

///////////////////////       HTML part Ends ///////////////////////
    Serial.print("\nYou entered crop:  ");
    Serial.print(crop_type);
    Serial.print("\nYou entered soil:  ");
    Serial.print(soil_type);
    Serial.print("\nYou entered area:  ");
    Serial.print(land_area);
    Serial.print("\nYou entered location: lattitude: ");
    Serial.print(loc_lat);
    Serial.print("\nYou entered location: longitude: ");
    Serial.println(loc_lon);
    
/////////////////////    Setting up the API data for weather parameters from openweather api///////////////////////

    String servername="api.openweathermap.org";
    HTTPClient http;
    http.begin(client, "http://"+servername +"/data/2.5/weather?lat="+loc_lat+"&lon="+loc_lon+"&units=metric&APPID="+apikey);
    Serial.println("");
    //Serial.println( servername +"/data/2.5/weather?lat="+loc_lat+"&lon="+loc_lon+"&units=metric&APPID="+apikey);
    http.GET();
    //Serial.print(http.getString());
    String result=http.getString();
    http.end();

  ////////////////////////// Parsing the JSON data from API /////////////
  //Serial.println("Result= ");
  //Serial.println(result);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, result);

  // Test if parsing succeeds.
  if (error) {
    Serial.println("DEserialization failed()");
    Serial.println(error.f_str());

  }
  // Assigning the data from API ////
  Temp_val = doc["main"]["temp"].as<String>(); 
  Light_val = doc["weather"][0]["description"].as<String>(); 
  Humid_val = doc["main"]["humidity"].as<String>(); 
  Rain_val = doc["clouds"]["all"].as<String>(); 
  Place_Name=doc["name"].as<String>();
  sunrise=doc["sys"]["sunrise"].as<String>();

  Serial.print("initial time: ");
  Serial.println(T1sec);
  Serial.println(sunrise.toInt());
  Serial.println("///////////////Data//////////////");
  //////////////////////// Calculating the Elapsed Time from Sowing Plant//////////////
  int diff =(sunrise).toInt()-T1sec;
  int hours = diff / 3600;
  time_elap=hours/24;
  Serial.println("elap_time "+String(time_elap));
  client.stop();
///////////////////////////////////////////////////////////Logic Control////////////////////////////////////////////////

/////////////////////////////variable condition setup///////////////
if(crop_type=="wheat")
{
  initial=20;
  initial_moist=2;
  develop=50;
  develop_moist=4;
  mid=95;
  mid_moist=6;
  late=120;
  late_moist=2;
}
if(crop_type=="rice")
{
  initial=25;
  initial_moist=40;
  develop=60;
  develop_moist=200;
  mid=105;
  mid_moist=500;
  late=130;
  late_moist=40;
  
}
if(crop_type=="pulses")
{
  initial=15;
  initial_moist=1;
  develop=40;
  develop_moist=5;
  mid=75;
  mid_moist=6;
  late=90;
  late_moist=1;
}
////////////////////////Main logic code/////////////////////
//////////initial plantation/////////
  if(time_elap<=initial)
  {
    if(Moist_Value<initial_moist)
    {
      digitalWrite(motor,HIGH);
      status_pump="Pump ON";
      //turn on the motor;
    }
    else
    {
      digitalWrite(motor,LOW);
      status_pump="Pump OFF";
      //no need to turn on 
      //turn off if already on;
    }
  }
  ///////////////////////develop plantation///////////////
  else if(time_elap>initial && time_elap<develop)
  {
    if(Moist_Value<=develop_moist)
    {
      digitalWrite(motor,HIGH);
      status_pump="Pump ON";
      
    }
    else
    {
      digitalWrite(motor,LOW);
      status_pump="Pump OFF";
    }
  }
  ///////////////////mid plantation//////////////
  else if(time_elap>=develop && time_elap<mid)
  {
    if(Moist_Value<mid_moist)
    {
      //drip on
      digitalWrite(motor,HIGH);
      status_pump="Pump ON";
    }
    else{
      //drip off
      digitalWrite(motor,LOW);
      status_pump="Pump OFF";
    }
  }
  //////////////////late plantation/////////////////////////
  else if(time_elap>=mid && time_elap<late)
  {
    if(Moist_Value<late_moist)
    {
      //drip on;
      digitalWrite(motor,HIGH);
      status_pump="Pump ON";
    }
    else
    {
      //drip off;
      digitalWrite(motor,LOW);
      status_pump="Pump OFF";
    }
  }

///////////////////////end of main logic coontrol//////////////

   /////// Printing on serial monitor  
      Serial.print("API data: ");
      Serial.println(Temp_val);
      Serial.println(Light_val);
      Serial.println(Humid_val);
      Serial.println(Rain_val);
      Serial.println(Place_Name);
      Serial.println("Sunrise: "+sunrise);
      Serial.println("printing page");
    delay(2000);

}
