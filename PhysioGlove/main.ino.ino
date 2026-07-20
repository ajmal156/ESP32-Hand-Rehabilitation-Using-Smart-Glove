#include <WiFi.h> // using the esp32 connect wifi
#include <WebServer.h> // acess in  local server
#include <ESPmDNS.h> // to create the link 
#include <HTTPClient.h> // to connect the data based 

// WiFi Details

const char* ssid = "Flash fiber 4G";
const char* password = "Phrase57297";

// Pins


#define TRIG_PIN 5
#define ECHO_PIN 18

#define GREEN_LED 25
#define RED_LED 26
#define BUZZER 27


WebServer server(80); // using the web server

long duration;
float distance;


float getDistance() // Measure Distance
{

  digitalWrite(TRIG_PIN, LOW); // first low 
  delayMicroseconds(2);


  digitalWrite(TRIG_PIN, HIGH); // 
  delayMicroseconds(10); 


  digitalWrite(TRIG_PIN, LOW);



  duration = pulseIn(ECHO_PIN, HIGH, 3000);



  distance = duration * 0.0343 / 2; // Distance in cm 


  return distance;

}

// Control LED and Buzzer

String checkStatus(float d)
{

  if(d < 200)
  {

    digitalWrite(GREEN_LED,HIGH);
    digitalWrite(RED_LED,LOW);
    digitalWrite(BUZZER,LOW);


    return "To SAFE";

  }

  else if(d > 40)
  {

    digitalWrite(GREEN_LED,HIGH);
    digitalWrite(RED_LED,LOW);
    digitalWrite(BUZZER,LOW);


    return "SAFE";

  }


  else if(d > 20)
  {

    digitalWrite(GREEN_LED,LOW);
    digitalWrite(RED_LED,HIGH);
    digitalWrite(BUZZER,LOW);


    return "WARNING";

  }


  else
  {

    digitalWrite(GREEN_LED,LOW);
    digitalWrite(RED_LED,HIGH);
    digitalWrite(BUZZER,HIGH);


    return "DANGER";

  }

}

// Send Data To Python Flask

void sendDataToDatabase(float d, String status)
{


  if(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    
    http.begin("http://192.168.1.8:5000/data"); // My PC IP Adress
    
    http.addHeader(
      "Content-Type",
      "application/json"
    );

    String json = "{";

    json += "\"distance\":";
    json += String(d);

    json += ",";

    json += "\"status\":\"";
    json += status;
    json += "\"";

    json += "}";

    int response = http.POST(json);

    Serial.print("Database Response: ");
    Serial.println(response);
    http.end();

  }

}

// Web Page

void handleRoot()
{


String html = R"rawliteral(


<!DOCTYPE html>

<html>


<head>

<title>
ESP32 Distance Monitor
</title>


</head>



<body>



<h1>
ESP32 Distance Monitoring System
</h1>



<h2>
Distance:

<span id="distance">
0
</span>

cm

</h2>



<h2>
Status:

<span id="status">
Loading
</span>

</h2>




<script>


function updateData()
{


fetch("/sensor")


.then(response => response.json())


.then(data => {


document.getElementById("distance").innerHTML = data.distance;


document.getElementById("status").innerHTML = data.status;


});


}



setInterval(updateData,1000);


updateData();



</script>



</body>


</html>


)rawliteral";



server.send(
200,
"text/html",
html
);


}

// Send Sensor Data To Web



void sendSensorData()
{


float d = getDistance();


String status = checkStatus(d);



String json = "{";


json += "\"distance\":";
json += String(d);



json += ",";



json += "\"status\":\"";
json += status;
json += "\"";



json += "}";



server.send(
200,
"application/json",
json
);


}

// Setup



void setup()
{


Serial.begin(115200);



// Pin setup

pinMode(TRIG_PIN,OUTPUT);
pinMode(ECHO_PIN,INPUT);


pinMode(GREEN_LED,OUTPUT);
pinMode(RED_LED,OUTPUT);
pinMode(BUZZER,OUTPUT);


// WiFi Connection

WiFi.begin(ssid,password);


Serial.print("Connecting WiFi");


while(WiFi.status() != WL_CONNECTED)
{

delay(500);

Serial.print(".");

}



Serial.println();


Serial.println("WiFi Connected");



Serial.print("ESP32 IP Address: ");

Serial.println(WiFi.localIP());




// mDNS

if(MDNS.begin("esp32"))
{

Serial.println("Open:");

Serial.println("http://esp32.local");

}


// Web routes

server.on("/", handleRoot);


server.on("/sensor", sendSensorData);


// Start server

server.begin();


Serial.println("Web Server Started");


}

// Loop

void loop()
{


server.handleClient();



float d = getDistance();


String status = checkStatus(d);


Serial.print("Distance: ");

Serial.print(d);

Serial.println(" cm");


// Send data every 5 seconds

sendDataToDatabase(d,status);


delay(5000);

}







