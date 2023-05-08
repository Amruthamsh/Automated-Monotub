
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32_CAM_Send_Photo_to_Google_Drive

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WARNING!!! Make sure that you have either selected Ai Thinker ESP32-CAM or ESP32 Wrover Module or another board which has PSRAM enabled.                   //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reference :                                                                                                                                                //
// - esp32cam-gdrive : https://github.com/gsampallo/esp32cam-gdrive                                                                                           //
// - ESP32 CAM Send Images to Google Drive, IoT Security Camera : https://www.electroniclinic.com/esp32-cam-send-images-to-google-drive-iot-security-camera/  //
// - esp32cam-google-drive : https://github.com/RobertSasak/esp32cam-google-drive                                                                             //
//                                                                                                                                                            //
// When the ESP32-CAM takes photos or the process of sending photos to Google Drive is in progress, the ESP32-CAM requires a large amount of power.           //
// So I suggest that you use a 5V power supply with a current of approximately 2A.                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//Preferencees: http://arduino.esp8266.com/stable/package_esp8266com_index.json, https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json


//======================================== Including the libraries.
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "esp_camera.h"

//======================================== 

//======================================== CAMERA_MODEL_AI_THINKER GPIO.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//======================================== 

//UDR(3) - 2
//UTR(1) - 3

// LED Flash PIN (GPIO 4)
#define FLASH_LED_PIN 4

//DEEP SLEEP MODE (in minutes) ---- 20 minutes (my plan)
#define DEEP_SLEEP_TIME 2 

void goToDeepSleep() {
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60 * 1000000);
  esp_deep_sleep_start();      
}



//RTC_DATA_ATTR unsigned int day = 0;

//======================================== Enter your WiFi ssid and password.
const char* ssid = "shiva";
const char* password = "prasad123";
//======================================== 

//======================================== Replace with your "Deployment ID" and Folder Name.
String myDeploymentID = "AKfycby9MlvWBpBm_jP0hyvhLoojnbCxBy-PO6wMsgo1gZ_5ClMDg1uJuZNQhAA3kXoUKJHmGQ"; // = "AKfycby9MlvWBpBm_jP0hyvhLoojnbCxBy-PO6wMsgo1gZ_5ClMDg1uJuZNQhAA3kXoUKJHmGQ";
String myMainFolderName = "ESP32-CAM";
//======================================== 


// Variable to set capture photo with LED Flash.
// Set to "false", then the Flash LED will not light up when capturing a photo.
// Set to "true", then the Flash LED lights up when capturing a photo.
bool LED_Flash_ON = true;

// Initialize WiFiClientSecure.
WiFiClientSecure client;

//SENDING SENSOR DATA TO GOOGLE SHEETS - https://stackoverflow.com/questions/69685813/problem-esp32-send-data-to-google-sheet-through-google-app-script

//const char* host = "script.google.com";
//const int httpsPort = 443;
String sheetID = "AKfycbygFq785P6reqL-fKJG0KHXqwFRbgFNpBkOXnFaYwoq6kiEn8FFPYnGt3BHPTl6sWyL"; //AKfycbxNyCYvRRHDYDFITyw37Z0JzpnavpdqC0gZuDIihgVCfDMLF0FjpFlTE_bd55bocg_B

//const String serverName = "https://script.google.com/macros/s/AKfycbxNyCYvRRHDYDFITyw37Z0JzpnavpdqC0gZuDIihgVCfDMLF0FjpFlTE_bd55bocg_B/exec?";



//======================================== Variables for Timer/Millis.
unsigned long previousMillis = 0; 
const int Interval = 120000; //--> Capture and Send a photo every 2 minutes
//======================================== 


//________________________________________________________________________________ Test_Con()
// This subroutine is to test the connection to "script.google.com".
void Test_Con() {
  const char* host = "script.google.com";
  while(1) {
    client.setInsecure();
  
    if (client.connect(host, 443)) {
      client.stop();
      break;
    } else {
      client.stop();
    }
  
    delay(1000);
  }
}
//________________________________________________________________________________ 

//________________________________________________________________________________ SendCapturedPhotos()
// Subroutine for capturing and sending photos to Google Drive.
void SendCapturedPhotos() {
  const char* host = "script.google.com";
  String url;
  client.setInsecure();

  /*
  //---------------------------------------- The Flash LED blinks once to indicate connection start.
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(100);
  digitalWrite(FLASH_LED_PIN, LOW);
  delay(100);
  //---------------------------------------- 
  */
  //---------------------------------------- The process of connecting, capturing and sending photos to Google Drive.
  if (client.connect(host, 443)) {
    if (LED_Flash_ON == true) {
      digitalWrite(FLASH_LED_PIN, HIGH);
      delay(100);
    }

    //.............................. Taking a photo.
    for (int i = 0; i <= 3; i++) {
      camera_fb_t * fb = NULL;
      fb = esp_camera_fb_get();
       if(!fb) {
          delay(1000);
          ESP.restart();
          return;
        } 
      esp_camera_fb_return(fb);
      delay(200);
    }
  
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if(!fb) {
      delay(1000);
      ESP.restart();
      return;
    } 
  
    if (LED_Flash_ON == true) digitalWrite(FLASH_LED_PIN, LOW);
    
    //.............................. 

    //.............................. Sending image to Google Drive.
    url = "/macros/s/" + myDeploymentID + "/exec?folder=" + myMainFolderName;

    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: " + String(host));
    client.println("Transfer-Encoding: chunked");
    client.println();

    int fbLen = fb->len;
    char *input = (char *)fb->buf;
    int chunkSize = 3 * 1000; //--> must be multiple of 3.
    int chunkBase64Size = base64_enc_len(chunkSize);
    char output[chunkBase64Size + 1];
    
    int chunk = 0;
    for (int i = 0; i < fbLen; i += chunkSize) {
      int l = base64_encode(output, input, min(fbLen - i, chunkSize));
      client.print(l, HEX);
      client.print("\r\n");
      client.print(output);
      client.print("\r\n");
      delay(100);
      input += chunkSize;
      chunk++;
    }
    client.print("0\r\n");
    client.print("\r\n");

    esp_camera_fb_return(fb);
    //.............................. 

    //.............................. Waiting for response.
    long int StartTime = millis();
    while (!client.available()) {
      delay(100);
      if ((StartTime + 10 * 1000) < millis()) {
        break;
      }
    }
    //.............................. 
    /*
    //.............................. Flash LED blinks once as an indicator of successfully sending photos to Google Drive.
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(500);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(500);
    //.............................. 
    */    
  }
  //---------------------------------------- 
  client.stop();

  
  //SEND SENSOR DATA and GET PHASE
  String val = "";
  char c;
  
  while(c != '=')
  {
    delay(20);
    while(Serial.available()) // look for all characters in the input buffer
    {
      c = Serial.read();
      val.concat(c);
    }  
  }    
  int n = 0;
  if(val.length() > 18)
  {
    val = val.substring(val.indexOf('=')+1,val.length()-1); 
  }     
    
  n = val.indexOf('&');
  String co2 = val.substring(0,n);  
  String t = val.substring(n+1,n+6);  
  String h = val.substring(n+7,n+12);  
  
    //Standard Test Values
    //float temp = 25.45; // from sensor
    //float hum = 50; // from sensor
    //float co2 = 555; // from sensor  
  
    //String queryString = serverName + String("temp=") + String(temp) + String("&hum=") + String(hum) + String("&co2=") + String(co2);
    //https://script.google.com/macros/s/AKfycbxNyCYvRRHDYDFITyw37Z0JzpnavpdqC0gZuDIihgVCfDMLF0FjpFlTE_bd55bocg_B/exec?
    //---- Post Data
  url = "";
  url += "https://script.google.com/macros/s/" + sheetID + "/exec?";
  url += "&temp=" + t;
  url += "&hum=" + h;
  url += "&co2=" + co2;
  
  HTTPClient http;
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET(); 

  String payload;
  if (httpCode > 0) { //Check for the returning code
    payload = http.getString();
    Serial.print(payload);   
  }
	http.end();  
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID SETUP()
void setup() {
  // Disable brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);

  delay(1000);

  pinMode(FLASH_LED_PIN, OUTPUT);
  
  // Setting the ESP32 WiFi to station mode.

  WiFi.mode(WIFI_STA);

  //---------------------------------------- The process of connecting ESP32 CAM with WiFi Hotspot / WiFi Router.
  WiFi.begin(ssid, password);
  
  // The process timeout of connecting ESP32 CAM with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 CAM has not been successfully connected to WiFi, the ESP32 CAM will restart.
  // I made this condition because on my ESP32-CAM, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(250);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(250);
    
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  digitalWrite(FLASH_LED_PIN, LOW);
  //Serial.print("ESP32-CAM IP Address: ");
  //Serial.println(WiFi.localIP());
  //---------------------------------------- 

  //---------------------------------------- Set the camera ESP32 CAM.
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 8;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    delay(1000);
    ESP.restart();
  }

  sensor_t * s = esp_camera_sensor_get();

  // Selectable camera resolution details :
  // -UXGA   = 1600 x 1200 pixels
  // -SXGA   = 1280 x 1024 pixels
  // -XGA    = 1024 x 768  pixels
  // -SVGA   = 800 x 600   pixels
  // -VGA    = 640 x 480   pixels
  // -CIF    = 352 x 288   pixels
  // -QVGA   = 320 x 240   pixels
  // -HQVGA  = 240 x 160   pixels
  // -QQVGA  = 160 x 120   pixels
  s->set_framesize(s, FRAMESIZE_UXGA);  //--> UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  delay(1000);

  Test_Con();
  delay(2000);


  //Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
}
//________________________________________________________________________________ 


//________________________________________________________________________________ VOID LOOP()
void loop() 
{ 
  SendCapturedPhotos();
  goToDeepSleep();
}
//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
