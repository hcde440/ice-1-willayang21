/*
 * This is a sketch that gets the ESP8266 to the WiFi and then connects to api.ipify.org using API key to get
 * our external IP address. With all that, ESP then connects to api.openweathermap.org using another key as well as the 
 * location info to get the weather info.
 * 
 * Every time ESP uses an api it has to make a HTTP request, parse the string data, transform the data to json format, and 
 * stores the data we need into the struct (location and conditions) as strings.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "yeetbois";
const char* pass = "petersch";
const String key = "b226ebd2ec194b655cb23e4e46a41e10"; // the key to get IP address and locations
const String weatherKey = "7bc7661254d77ecdaf02640767252c88"; // the key to get weather info

typedef struct {  // here we create a new data type definition, a box to hold other data types
  String ip;      // for each name:value pair coming in from the service, we will create a slot                
  String cc;      // in our structure to hold our data
  String cn;
  String rc;
  String rn;
  String cy;
  String tz;
  String ln;
  String lt;
} GeoData;        // we name it GeoData to store location data

typedef struct { // here we create a new data type definition, a box to hold other data types
  String tp;     // for each name:value pair coming in from the service, we will create a slot
  String pr;     // in our structure to hold our data
  String hd;
  String ws;
  String wd;
  String cd;
} MetData;       // we name it MetData to store weather data

GeoData location; // data type is GeoData and name is location
MetData conditions; // data type is MetData and name is conditions

void setup() {
  Serial.begin(115200); // initialize serial communications
  delay(10); // delay 10 miliseconds 
  
  // print wifi name
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  
  WiFi.mode(WIFI_STA); // start connecting to wifi
  WiFi.begin(ssid, pass); // initialize the wifi name and pw

  while (WiFi.status() != WL_CONNECTED) {   // keep connecting until the wifi is connected
    delay(500); // wait for 0.5 sec
    Serial.print(".");  // print dots
  }

  // inform the wifi is connected and print the IP address
  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP());

  String ipAddress = getIP(); // create a variable to store the IP address
  getGeo(); // call the getGeo function which will parse the location data

  Serial.println();
  // print location information from the location GeoData
  Serial.println("Your external IP address is " + location.ip); 
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),"); 
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.print("You are in the " + location.tz + " timezone ");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");

  // call getMet function to update values in the 'conditions' variable
  getMet(location.cy);

  // print the location info from location variable and weather conditions from conditions variable
  Serial.println();
  Serial.println("With " + conditions.cd + ", the temperature in " + location.cy + ", " + location.rc);
  Serial.println("is " + conditions.tp + "F, with a humidity of " + conditions.hd + "%. The winds are blowing");
  Serial.println(conditions.wd + " at " + conditions.ws + " miles per hour, and the ");
  Serial.println("barometric pressure is at " + conditions.pr + " millibars.");
}

void loop() {
}

String getIP() {
  HTTPClient theClient;   // Initialize the client library
  String ipAddress;       // create a string
  theClient.begin("http://api.ipify.org/?format=json");  // start listening for the incoming connection
  int httpCode = theClient.GET();  // make a HTTP request
  if (httpCode > 0) {     // if the response is not empty
    if (httpCode == 200) {  // check the connection to the endpoint                                
      DynamicJsonBuffer jsonBuffer;  // creates an entry point that handles
                                     //  the memory management and calls the parser
      String payload = theClient.getString();  // get the request response payload
      JsonObject& root = jsonBuffer.parse(payload);  // parse the json
      ipAddress = root["ip"].as<String>();   // return and store the ipaddress as strings
    } else {  // bad connection to the endpoint
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";    // error message
    }
  }
  return ipAddress;  // this function returns an IPaddress if the response is valid
}

void getGeo() {
  HTTPClient theClient;  // initialize the HTTPClient
  Serial.println("Making HTTP request");
  theClient.begin("http://api.ipstack.com/" + getIP() + "?access_key=" + key); // return IP as .json object
  int httpCode = theClient.GET();   // make a HTTP request
  if (httpCode > 0) {               // if the response is not empty
    if (httpCode == 200) {          // check the connection to the endpoint
      Serial.println("Received HTTP payload.");     
      DynamicJsonBuffer jsonBuffer;   //  intialized the JsonBuffer
      String payload = theClient.getString();  // get the request response payload
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);  // parse the json
      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }
      //Some debugging lines below:
      //      Serial.println(payload);
      //      root.printTo(Serial);
      //Using .dot syntax, we refer to the variable "location" which is of
      //type GeoData, and place our data into the data structure.
      location.ip = root["ip"].as<String>();            // cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();  // the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

void getMet(String city) {
  HTTPClient theClient;  // initialize the HTTPClient
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=Seattle"; // store the base url
  apiCall += "&units=imperial&appid=";  // add more conditions
  apiCall += weatherKey;  // add weather api key
  theClient.begin(apiCall);  // start listening for the incoming connection
  int httpCode = theClient.GET();  // make a HTTP request
  if (httpCode > 0) {    // if the response is not empty

    if (httpCode == HTTP_CODE_OK) {  // checks the connection to the endpoint
      String payload = theClient.getString();  // get the request response payload
      DynamicJsonBuffer jsonBuffer;   // intialize the JsonButton
      JsonObject& root = jsonBuffer.parseObject(payload);  // parse the json
      if (!root.success()) {  // if the parsing fails
        Serial.println("parseObject() failed in getMet().");
        return;
      }
      conditions.tp = root["main"]["temp"].as<String>();   // we cast the values as Strings b/c
      conditions.pr = root["main"]["pressure"].as<String>(); // the 'slots' in MetData are Strings
      conditions.hd = root["main"]["humidity"].as<String>(); 
      conditions.cd = root["weather"][0]["description"].as<String>();
      conditions.ws = root["wind"]["speed"].as<String>();
      int deg = root["wind"]["deg"].as<int>();    // store the degree as integers
      conditions.wd = getNSEW(deg);        // transform degrees to direction and store it to conditions
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");
  }
}

// transform a degree value to a direction from N, S, E, W
String getNSEW(int d) {
  String direct;

  //Conversion based upon http://climate.umn.edu/snow_fence/Components/winddirectionanddegreeswithouttable3.htm
  if (d > 348.75 && d < 360 || d >= 0  && d < 11.25) {
    direct = "north";
  };
  if (d > 11.25 && d < 33.75) {
    direct = "north northeast";
  };
  if (d > 33.75 && d < 56.25) {
    direct = "northeast";
  };
  if (d > 56.25 && d < 78.75) {
    direct = "east northeast";
  };
  if (d < 78.75 && d < 101.25) {
    direct = "east";
  };
  if (d < 101.25 && d < 123.75) {
    direct = "east southeast";
  };
  if (d < 123.75 && d < 146.25) {
    direct = "southeast";
  };
  if (d < 146.25 && d < 168.75) {
    direct = "south southeast";
  };
  if (d < 168.75 && d < 191.25) {
    direct = "south";
  };
  if (d < 191.25 && d < 213.75) {
    direct = "south southwest";
  };
  if (d < 213.25 && d < 236.25) {
    direct = "southwest";
  };
  if (d < 236.25 && d < 258.75) {
    direct = "west southwest";
  };
  if (d < 258.75 && d < 281.25) {
    direct = "west";
  };
  if (d < 281.25 && d < 303.75) {
    direct = "west northwest";
  };
  if (d < 303.75 && d < 326.25) {
    direct = "south southeast";
  };
  if (d < 326.25 && d < 348.75) {
    direct = "north northwest";
  };
  return direct;
}
