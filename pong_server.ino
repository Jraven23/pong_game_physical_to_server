#include <Arduino.h>
#include "pong.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include "SPIFFS.h"

#define RST_PIN 23
#define POT_PIN1 32
#define POT_PIN2 33

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Replace with your network credentials
const char* ssid = "MOVISTAR_BC19";
const char* password = "QDS4YJvgYNCZpRVrjKc4";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}

Position init_pos_ball(0,0);
Velocity init_vel_ball(1,1);
Ball ball(init_pos_ball,init_vel_ball); 
Pong pong(ball,0,0,10,0,0); //ball, pos player 1, 2, raquetLength, Points player 1, 2
Pong pong_copy = pong;
float Ball_Speed=0.015;

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;  
unsigned long sendInfoDelay = 10;

String getPongJson(Pong p){
  
  readings["ballPosX"] = String(p.ball.pos.x);
  readings["ballPosY"] = String(p.ball.pos.y);
  readings["player1Pos"] = String(p.player1Pos);
  readings["player2Pos"] = String(p.player2Pos);
  readings["player1Points"] = String(p.player1Points);
  readings["player2Points"] = String(p.player2Points);
  readings["ballSpeed"] = String(Ball_Speed);
  //readings["raquetLength"] = String(pong.raquetLength);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

TaskHandle_t Task_wifi;
//data access control signals
bool writtingData=false;
bool sendingData=false;

void loop_wifi(void* parameters)
{
  while(1){
    if(!writtingData)
    {
      sendingData = true;
      pong_copy = pong; //make a pong copy, there should be no memory conflict..
      sendingData=false;
    }
    if ((millis() - lastTime) > sendInfoDelay) {
      // Send Events to the Web Server with the Sensor Readings
      events.send(getPongJson(pong_copy).c_str(),"pong_data",millis()); 
      lastTime = millis();
    }
    //Serial.println("En nucleo -> "+String(xPortGetCoreID()));
    //delay(500);
    vTaskDelay(10);
  }
  vTaskDelete(NULL);
}
  
void setup() {

  //nvs_flash_init();
  xTaskCreatePinnedToCore(loop_wifi,"Task_Server",4096,NULL,1,&Task_wifi,0);
  
  pinMode(RST_PIN, INPUT);
  Serial.begin(115200);
  resetBall(&ball);

  initWiFi();
  initSPIFFS();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
    pong.player1Points = pong.player2Points = 0;
    resetBall(&pong.ball);
    request->send(200, "text/plain", "OK");
  });

  server.on("/increaseSpeed", HTTP_GET, [](AsyncWebServerRequest *request){
    Ball_Speed = Ball_Speed+0.005;
    request->send(200, "text/plain", "OK");
  });

  server.on("/decreaseSpeed", HTTP_GET, [](AsyncWebServerRequest *request){
    Ball_Speed = Ball_Speed-0.005;
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("Hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
  Serial.println("Server initialized");
}

//Loop in core1: will compute the position of rackets and ball
void loop() {
  // check if rst is pressed.
  if (digitalRead(RST_PIN)) 
  {
    writtingData = true;
    pong.player1Points = pong.player2Points = 0;
    resetBall(&pong.ball);
    writtingData = false;
  } 
  else if(!sendingData)
  {
    // read the value from the sensor:
    writtingData = true;
    pong.player1Pos = analogRead(POT_PIN1)*(800-0)/4096; //max: 4096, min: 0
    //pong.player1Pos = touchRead(POT_PIN1); //max: 4096, min: 0
    pong.player2Pos = analogRead(POT_PIN2)*(800-0)/4096;
    //Serial.println("Player 1: " + String(pong.player1Pos) + " -- Player 2: " + String(pong.player2Pos));
    player_within_limits(pong.player1Pos,pong.raquetLength);
    player_within_limits(pong.player2Pos,pong.raquetLength);
    //Serial.println("[LIMITED] Player 1: " + String(pong.player1Pos) + " -- Player 2: " + String(pong.player2Pos));
    goal_check(&pong);
    //Serial.println("Player 1 GOALs: " + String(pong.player1Points));
    ball_update(&pong,Ball_Speed); //second param is dt
    writtingData = false;
    /*if ((millis() - lastTime) > sendInfoDelay) {
      // Send Events to the Web Server with the Sensor Readings
      events.send(getPongJson(pong).c_str(),"pong_data",millis());
      lastTime = millis();
    }*/
    
  }
  delayMicroseconds(1000);
  
}
