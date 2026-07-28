#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "Smart-Campus-STD";
const char* password = "pafiast##std2021";

// Python Server
String pythonServer = "http://10.1.79.205:5000";

WebServer server(80);

bool loginSuccess = false;


// HTML Login Page


String loginPage() {
  return R"rawliteral(

  <!DOCTYPE html>

  <html>

  <head>
  
  <title> Physiotherapy Glove Login </title>
  
  </head>

  <body>

  <h2> Physiotherapy Glove System </h2>

  <form action="/login" method="POST">

  Username <br> <input type="text" name="username"> 
  <br><br>
  Password<br>
  
  <input type="password" name="password"><br>
  
  <br><input type="submit" value="Login">

  </form>

  </body>

  </html>

  )rawliteral";
}

// Login Request


void handleLogin() {

  String username = server.arg("username");
  String userPassword = server.arg("password");

  HTTPClient http;

  http.begin(pythonServer + "/login");

  http.addHeader("Content-Type","application/json");

  String json = "{\"username\":\"" + username + "\",\"password\":\""+ userPassword + "\"}";

  int code = http.POST(json);

  String response = http.getString();

  http.end();

  if(code==200 && response=="SUCCESS") {
    
    loginSuccess=true;

    server.send(200,"text/html",
    "<h2> Login Successful </h2> <p> You can now use the keypad. </p>");

  }
  else {
    server.send(200,"text/html",
    "<h2> Invalid Username or Password </h2>");
  }

}

// Send Therapy Data

void sendData(String exercise , String duration) {

  if(!loginSuccess) {
   
  Serial.println("Patient not logged in.");
  return;
  }

  HTTPClient http;

  http.begin(pythonServer + "/save");

  http.addHeader("Content-Type","application/json");

  String json="{\"exercise\":\""+ exercise + "\",\"duration\":\"" + duration + "\",\"status\":\"Completed\"}";

  int code=http.POST(json);

  Serial.print("Save Response: ");
  Serial.println(code);

  http.end();

}

// WiFi Setup

void setupWiFi() {

  WiFi.begin(ssid,password);

  Serial.print("Connecting");

  while(WiFi.status()!=WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  server.on("/",HTTP_GET,[]() {server.send(200 , "text/html" , loginPage());});

  server.on("/login",HTTP_POST,handleLogin);

  server.begin();

  Serial.println("Web Server Started");

}
  





