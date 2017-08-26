#include <ESP8266WiFi.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1

const char* ssid = "...";
const char* password = "...";

IPAddress ip(192,168,1,55);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  Serial1.begin(9600);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial1.print("\nConnecting to "); Serial1.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    Serial1.print("Could not connect to"); Serial1.println(ssid);
    while(1) delay(500);
  }
  //start UART and the server
  Serial.begin(9600);
  server.begin();
  server.setNoDelay(true);
  
  Serial1.print("Ready! Use 'telnet ");
  Serial1.print(WiFi.localIP());
  Serial1.println(" 23' to connect");
}

void loop() {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial1.print("New client: "); Serial1.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      }
    }
  }
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
}
