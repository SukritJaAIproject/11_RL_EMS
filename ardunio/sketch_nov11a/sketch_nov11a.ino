#include <HTTPClient.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

//SHT31
#include <Wire.h>
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();


//CCS811
#include "Adafruit_CCS811.h"
Adafruit_CCS811 ccs;


//EGATWIFI
//#include "esp_wpa2.h" 
//#define EAP_ANONYMOUS_IDENTITY "596621"
//#define EAP_IDENTITY "596621"
//#define EAP_PASSWORD "Sj@scg19"
//const char *ssid = "EGATWIFI";  
//int counter = 0;


// Other WIFI Start 
//const char *ssid =     "mummai_owner_2.4G";         
//const char *password = "mummai0972470067";              
//IPAddress staticIP(192, 168, 1, 242);
//IPAddress gateway(192, 168, 1, 1);
//IPAddress subnet(255, 255, 255, 0);
//IPAddress dns(192, 168, 1, 1);
// Other WIFI End 


// Other WIFI Start 
const char *ssid =     "3bb-Boonsom2.4G";         
const char *password = "0870876026";              
IPAddress staticIP(192, 168, 1, 242);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);
// Other WIFI End 


WiFiClient client;
//################## S Mysql #######################
//MySQL_Connection conn((Client *)&client);
//################## E Mysql #######################

//Sent the Data
//const char* serverName = "http://10.40.62.79/EMS/post-esp-data.php";
const char* serverName = "http://35.202.157.132/EMS/post-esp-data.php";
String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "Amphawa home";
String sensorLocation = "Joroom";

#define SEALEVELPRESSURE_HPA (1013.25)

//################## S Mysql #######################
//byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress server_addr(35,202,157,132);  
//char dbuser[] = "root";        
//char dbpassword[] = "";      
//
//char query[] = "SELECT * FROM example_esp32.SensorData LIMIT 12";  
////const char QUERY_POP[] = "SELECT * FROM example_esp32.SensorData WHERE id='12';";
//################## E Mysql #######################

//Your Domain name with URL path or IP address with path
const char* serverName = "http://35.202.157.132/EMS/API/api_sensors.php";

String sensorReadings;
float sensorReadingsArr[3];


void setup() {
  
  Serial.begin(115200);

  //WIFI
//  Serial.println(); Serial.print("Connecting to network: ");
//  Serial.println(ssid);
//  WiFi.disconnect(true); //disconnect form wifi to set new wifi connection
//  WiFi.mode(WIFI_STA);   //init wifi mode
//  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY));
//  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
//  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
//  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
//  esp_wifi_sta_wpa2_ent_enable(&config);                 //set config settings to enable function
//  WiFi.begin(ssid);                                      //connect to wifi
//  
//  while (WiFi.status() != WL_CONNECTED)
//  {
//    delay(500);
//    Serial.print(".");
//    counter++;
//    if (counter >= 60)
//    { //after 30 seconds timeout - reset board
//      ESP.restart();
//    }
//  }
//  
//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.println("IP address set: ");
//  Serial.println(WiFi.localIP()); //print LAN IP


//######################### Other WIFI  ####################################################

  if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    Serial.println("Configuration failed.");
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); delay(100);
  Serial.println(); Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.println(F("WiFi connected :")); Serial.println(ip);
  
//######################### Other WIFI End  #######################################################
  

  // SHT31
  Serial.println("SHT31"); 
  if (! sht31.begin(0x44)) {
  Serial.println("Couldn't find SHT31");
  while (1) delay(1);
  }

  // CCS811
  Serial.println("CCS811");
  if(!ccs.begin()){
  Serial.println("Failed to start sensor! Please check your wiring.");
  while(1);
  }

  //  calibrate temperature sensor  
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0); 

  //########################### MySQL Connection ###############################
  MySQL_Connection conn((Client *)&client);
  Serial.println("Connecting...");
  if (conn.connect(server_addr, 3306, dbuser, dbpassword)) {
    delay(1000);
    Serial.println("MySQL Connected.");
  }
  else
    Serial.println("Connection failed.");
  //conn.close();

}


void loop() {
  
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    
    http.begin(serverName);
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    float temp = ccs.calculateTemperature();
    
    if(ccs.available()){
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
    Serial.print("CO2: = "); Serial.println(ccs.geteCO2());
    Serial.print("ppm, TVOC: "); Serial.println(ccs.getTVOC());
    Serial.print("ppb Temp:"); Serial.println(temp);
    delay(500);
    }
    else{
      Serial.println("ERROR!");
      while(1);}
    }      
    
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + String(sht31.readTemperature())
                          + "&value2=" + String(sht31.readHumidity())
                          + "&value3=" + String(ccs.geteCO2())
                          + "&value4=" + String(ccs.getTVOC())
                          + "&value5=" + String(temp)
                          + ""; 
                                                    
    Serial.print("httpRequestData: "); Serial.println(httpRequestData);
    
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
     
    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
    
    // If you need an HTTP request with a content type: application/json, use the following:
    //http.addHeader("Content-Type", "application/json");
    //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }


//  row_values *row = NULL;
//  //String head_count ; 
//  delay(10000);
//  
//  Serial.println("Selecting data.");
//
//  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
//  
//  cur_mem->execute(query); // Execute the query
//  column_names *columns = cur_mem->get_columns(); // Fetch the columns
//
//  // Read the row (we are only expecting the one)
//  do {
//    row = cur_mem->get_next_row();
//    if (row != NULL) {
//      Serial.println(row->values[1]); //ค่าที่ SELECT ได้จากฐานข้อมูล
//    }
//  } while (row != NULL);
//  delete cur_mem;  // Deleting the cursor also frees up memory used 


  
  //Send an HTTP POST request every 30 seconds
  delay(30000);  


  
}
