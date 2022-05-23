//including library
#include<TimeLib.h>
#include<time.h>
#include <ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

///// Sensor and Actuator configurations///////////
const int Moisture_Water=A0;
const int motor=D0;
String status_pump="NAN";
bool Emergency=false;

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
String date="20052022";
int in_day=0;
int in_mon=0;
int in_year=0;

////////////weather forecast and sensor parameters variables
String Temp_val="34C";
String Light_val="Daylight";
String  Moist_val="34";
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


///////////////////////////////////////////////HTML Web page for initial setup/////////////////////////

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<h1>Intial Setup form </h1>

<form action="/setup">
  Crop Type:<br>
  <input type="text" name="Crop" value="rice">
  <br>
  Field Type:<br>
  <input type="text" name="Field" value="loamy">
  <br>
  Land Area name:<br>
  <input type="text" name="Area" value="340">
  <br>
  PLace: Lattitude:<br>
  <input type="text" name="lattitude" value="25.7878336">
  <br>
  Place: Longitude:<br>
  <input type="text" name="longitude" value="93.9164595">
  <br>
  <br>
  <input type="submit" value="Submit">
</form> 
</body>
</html>
)=====";

//////////////////////////////////////////HTML page for Displaying the parameters////////////////////////

const char Disp_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="refresh" content="2">

<title>Automation</title>
</head>
<body>

<h1>Farm automation and Monitoring</h1>
<hr>

<h3>Crop Details</h3>
<hr>
<ul>
    <li>Crop Type: @crop@</li>

    <li>Field Type: @field@</li>
    <li>Field area: @area@</li>
    <li>Place: @place@
     <ul>
        <li>Lattitude: @lat@</li>
        <li>Longitude: @lon@</li>
     </ul>
    </li>

</ul>
<hr>
<h3>Current Conditions</h3>
<ul>
    <li>Temperature: @temp@\°C</li>
    <li>Light Condition: @light@</li>
    <li>Moisture: @moist@</li>
    <li>Humidity: @humid@%</li>
    <li>Rainfall: @rain@%</li>
</ul>
<hr>
<h3>Water State: </h3>
<ul>
    <li>Pump Status: @pump@</li>
</ul>
<hr>
<h3>Remarks: </h3>
<ul>
<li>Total time elapsed is @time@ days</li>
<li>Status : @remark@</li>
</u>
<form action="/emergency">
<input type="submit" name="Stop" value="Emergency Stop">
</form> 
</body>
</html>
)=====";

/////////////////////////////HTML page for Emergency status/////////////////////

const char Emergency_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h1> Emergency</h1>
<ul>
<li> Pump Status: @pump@</li>
<li> The System is Emergency Stopped</li>
<li> Please Reset and Restart the whole system by clicking the Reset option</li>

</ul>
<br>
<a href='/'>Reset the system </a>


</body>
</html>
)=====";

////////////////////server setup///////////
ESP8266WebServer server(80);
////////////////////////////WEb page Post for initial Setup : Acessing the root page/////////////////////
void handleSetup()
{
  Emergency=false;
  String s=MAIN_page;
  server.send(200,"text/html",s);
}
///////////////////////////////Web page Post fro display Setup: Accessing the display page/////////////////
void handleSystem()
{
  String s=Disp_page;
  if(server.arg("Crop")!="")
    crop_type=server.arg("Crop");
  if(server.arg("Area")!="")
    land_area=server.arg("Area");
  if(server.arg("Field")!="")
    soil_type=server.arg("Field");
  if(server.arg("lattitude")!="")
    loc_lat=server.arg("lattitude");
   if(server.arg("longitude")!="");
    loc_lon=server.arg("longitude");
  s.replace("@crop@",crop_type);
  s.replace("@field@",soil_type);
  s.replace("@area@",land_area);
  s.replace("@place@",Place_Name);
  s.replace("@lat@",loc_lat);
  s.replace("@lon@",loc_lon);
  s.replace("@temp@",Temp_val);
  s.replace("@light@",Light_val);
  s.replace("@moist@",String(Moist_Value));
  s.replace("@humid@",Humid_val);
  s.replace("@rain@",Rain_val);
  s.replace("@pump@",status_pump);
  s.replace("@time@",String(time_elap));
  s.replace("@remark@",Remark);
  server.send(200, "text/html",s);
}
////////////////////////Web page Post for Emergency status: Accessing the Emergency page//////////////////
void handleEmergency()
{
  Emergency=true;
  digitalWrite(motor,LOW);
  status_pump="Pump OFF";
  String s = Emergency_page;
  s.replace("@pump@",status_pump);
  
 server.send(200, "text/html", s);
}


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

  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(1000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());

  Serial.println("Hello");
/////////////////////////////////////////////Web server Request handling ///////////////////////
  server.on("/",handleSetup);
  server.on("/setup",handleSystem);
  server.on("/emergency",handleEmergency);
  server.begin();
  
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
////////////////////  WEb page and server  configuration   //////////////////
WiFiClient client;

    
/////////////////////    Setting up the API data for weather parameters from openweather api///////////////////////

    String servername="api.openweathermap.org";
    HTTPClient http;
    http.begin(client, "http://"+servername +"/data/2.5/weather?lat="+loc_lat+"&lon="+loc_lon+"&units=metric&APPID="+apikey);
    Serial.println("");
    Serial.println( servername +"/data/2.5/weather?lat="+loc_lat+"&lon="+loc_lon+"&units=metric&APPID="+apikey);
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
if(!Emergency)
 {
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
///////////////////////handling of client request///////////////////
      server.handleClient(); 
      delay(2000);

}
