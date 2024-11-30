// a remote-controlled watering system using ESP8266 as
// a web server and Cloudfare for DNS
//
// by mufasa159 (https://github.com/mufasa159)


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <DNSServer.h>

#define MOTOR_1 0
#define MOTOR_2 5
#define MOTOR_3 4

#define MOTOR_1_RUN_TIME 10000  // 10 seconds
#define MOTOR_2_RUN_TIME 5000   // 5 seconds
#define MOTOR_3_RUN_TIME 3000   // 3 seconds

// replace with your network credentials
const char* ssid = "my-wifi-ssid";
const char* password = "my-wifi-password";

// replace with your domain and secret code
const String domain = "https://example.com";
const String secret = "my-secret-code";

const byte DNS_PORT = 53;
ESP8266WebServer serverHTTP(80);
BearSSL::ESP8266WebServerSecure server(443);
DNSServer dnsServer;

// make sure you have SSL option set to "Full" in Cloudflare, then
// generate an "Origin Certificate" for your domain and paste
// their content below

static const char serverCert[] PROGMEM = R"EOF(
    <---- Paste your certificate here ---->
)EOF";


static const char serverKey[] PROGMEM =  R"EOF(
    <---- Paste your private key here ---->
)EOF";


const char HOME[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OASIS REMOTE</title>
    <style>
        body{margin:0;padding: 30px;font-family:monospace}
        .container{max-width:400px;margin:50px auto}
        .container h1{text-align:center;margin-bottom:30px}
        .row{display:flex;justify-content:space-between;align-items:center;margin:10px 0}
        .row p{font-size:20px;margin:10px 0;letter-spacing:1px}
        button{font-size:20px;font-family:monospace;text-decoration:none;color:#000;background-color:#f0f0f0;padding:5px 20px;border-radius:5px;border:1px solid #cdcdcd;letter-spacing:1px}
        input{padding:10px;font-size:20px;font-family:monospace;border-radius:5px;border:1px solid #cdcdcd;width:100%;margin-bottom:30px}
    </style>
</head>
<body>
    <div class="container">
        <h1>OASIS REMOTE</h1>
        <div class="row"><input type="password" name="wise" id="wise" oninput="handleCode()" /></div>
        <div class="row"><p>PLANT 1</p><button id="btn_w1" onclick="handleRun()">RUN</button></div>
        <div class="row"><p>PLANT 2</p><button id="btn_w2" onclick="handleRun()">RUN</button></div>
        <div class="row"><p>PLANT 3</p><button id="btn_w3" onclick="handleRun()">RUN</button></div>
    </div>
    <script>
        let code = "";

        function handleCode() {
            let e = document.getElementById("wise").value;
            e.length > 0 && (code = e)
        }
        
        function handleRun() {
            let e = event.target.id;
            code.length > 0 && (
                "btn_w1" === e ? window.location.href = `w1?c=${code}` : 
                "btn_w2" === e ? window.location.href = `w2?c=${code}` :
                "btn_w3" === e && (window.location.href = `w3?c=${code}`)
            )
        }
    </script>
</body>
</html>
)=====";


void errorpage() {
  server.send(200, "text/html", "<p style='font-family:monospace;'>404, Page not found</p>");
}

void homepage() {
  String s = HOME;
  server.send(200, "text/html", s);
}

void water_plant(int motor_pin, int run_time, const char* plant_name) {
  if (server.hasArg("c") && server.arg("c") == secret) {
    String response = "<p>Watering " + String(plant_name) + "</p>";
    server.send(200, "text/html", response);
    digitalWrite(motor_pin, HIGH);
    delay(run_time);
    digitalWrite(motor_pin, LOW);
  } else {
    server.send(200, "text/html", "<p>403, Invalid secret</p>");
  }
}

void setup(void) {
  // Serial.begin(9600);
  WiFi.begin(ssid, password);

  pinMode(MOTOR_1, OUTPUT);
  pinMode(MOTOR_2, OUTPUT);
  pinMode(MOTOR_3, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Serial.println("WiFi connected");
  // Serial.println(WiFi.localIP());

  IPAddress Ip(192, 168, 1, 1);

  serverHTTP.on("/", secureRedirect);
  serverHTTP.begin();

  dnsServer.start(DNS_PORT, domain.substring(8), Ip);

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.on("/", homepage);
  server.on("/w1", []() { water_plant(MOTOR_1, MOTOR_1_RUN_TIME, "Plant 1"); });
  server.on("/w2", []() { water_plant(MOTOR_2, MOTOR_2_RUN_TIME, "Plant 2"); });
  server.on("/w3", []() { water_plant(MOTOR_3, MOTOR_3_RUN_TIME, "Plant 3"); });
  server.onNotFound(errorpage);

  server.begin();
}

void loop() {
  serverHTTP.handleClient();
  server.handleClient();
  dnsServer.processNextRequest();
}

void secureRedirect() {
  serverHTTP.sendHeader("Location", domain, true);
  serverHTTP.send(301, "text/plain", "");
}
