/*
* Time-lapse project
* Author: Sergey Firsov
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Stepper.h"
#include "multiCameraIrControl.h"
#include "webPageContent.h"

// Wifi spot and the password
const char *SSID     = "phone_wifi_spot_name";
const char *PASSWORD = "wifi_password";

// Shooting mode
const char* NORMAL_MODE = "normal-mode";
const char* BULB_MODE   = "bulb-mode";

// Stepper constant settings.
// My stepper has a down gear. It takes 32 steps to make 360 degrees rotation of the actual stepper motor inside.
// But the down gear increases the external shaft steps count to 2048. So it takes 2048 steps to rotate a camera
// at 360 degrees. This parameter is specific for the motor and the value can be found in the motor specification.
const int STEPS_PER_REVOLUTION = 32;


// Pins
const int LED_CAMERA_CONTROL = D7;
const int LED_CONNECTED      = D1;
const int LED_POWER_ON       = D6;
const int STEPPER_PIN_1      = D2;
const int STEPPER_PIN_2      = D4;
const int STEPPER_PIN_3      = D3;
const int STEPPER_PIN_4      = D5;

// This structure represents instructions of how one iteration of time-lapse sequence should be made
struct Iteration {
    int allStepsSize;   // The steps count to which a camera should be rotated during the iteration
    int direction;      // Clockwise or counter clockwise
    int stepSize;       // To how many steps stepper motor will be turning a camera before taking each picture.
                        // The angle of one step is a motor specific.

    int stayFor;        // Specific for the "stay" iteration. Specifies how long a camera will be shooting while staying still.
    int timeScale;      // Multiplier in milliseconds. 1000 for a second, 60000 for a minute...

    Iteration() {
        allStepsSize = 0;
        direction    = 0;
        stepSize     = 0;
        stayFor      = 0;
        timeScale    = 1;
    }
};

// Control commands and state of the device
Iteration*      iterations          = 0;
int             currentIteration    = 0;
int             iterationsNumber    = 0;
String          shutterMode         = NORMAL_MODE;
bool            shutterOn           = false; // The command to make a photo
unsigned long   pauseBetweenShutter = 5000UL;
unsigned long   shutterOpenTime     = 0UL;
unsigned long   stepperDelay        = 1000UL;
bool            listenWiFi          = true; // This is the switch between wifi control and shooting

ESP8266WebServer server(80); // Initialize server with the port 80
Stepper myStepper(STEPS_PER_REVOLUTION, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);
Camera* camera = 0;

// returns true if delay is in progress, or false if delay time have passed
bool unblockedDelay(unsigned long delayInMillis) {
    static unsigned long delayStartTime = 0;

    // If delay was not started then initialize the start time
    if(delayStartTime == 0) {
        delayStartTime = millis();
    }

    // If delay time have not passed then return true
    if(millis() - delayStartTime < delayInMillis) {
        return true;
    }

    delayStartTime = 0;
    return false; // Delay time is over
}

// "go/" request handler
void go() {
    // reset state
    if(camera) {
        delete camera;
    }
    if(iterations != 0) {
        delete [] iterations;
    }
    currentIteration = 0;

    // read the control parameters sent from the client
    int cameraType      = atoi(server.arg("selectedCamera").c_str());
    camera = createCamera((CameraType)cameraType, LED_CAMERA_CONTROL);
    shutterMode         = server.arg("shutterMode").c_str();
    pauseBetweenShutter = atoi(server.arg("pauseBetweenShutter").c_str());
    shutterOpenTime     = atoi(server.arg("shutterOpenTime").c_str());
    iterationsNumber    = atoi(server.arg("iterationsNumber").c_str());
    iterations = new Iteration[iterationsNumber];

    for(int i=0; i<iterationsNumber; i++) {
        String stayForStr   = server.arg(String("stay") + String(i));
        String timeScaleStr = server.arg(String("timeScale") + String(i));
        String degreeStr    = server.arg(String("degree") + String(i));
        String directionStr = server.arg(String("direction") + String(i));
        String stepStr      = server.arg(String("step") + String(i));

        int degree    = atoi(degreeStr.c_str());

        Iteration iteration;
        iteration.timeScale = atoi(timeScaleStr.c_str());
        iteration.stayFor = atoi(stayForStr.c_str()) * iteration.timeScale;
        if(iteration.stayFor == 0) {
            iteration.direction = atoi(directionStr.c_str());
            iteration.stepSize = atoi(stepStr.c_str());
            iteration.allStepsSize = degree * 2048 / 360;
        }
        iterations[i] = iteration;
    }

    listenWiFi = false;
}

//-- Initialization functions ------------------------------------------------------------
void stepperSetup() {
    pinMode(STEPPER_PIN_1, OUTPUT);
    pinMode(STEPPER_PIN_2, OUTPUT);
    pinMode(STEPPER_PIN_3, OUTPUT);
    pinMode(STEPPER_PIN_4, OUTPUT);

    myStepper.setSpeed(100);
}

void ledsSetup() {
    pinMode(LED_CONNECTED, OUTPUT);
    pinMode(LED_POWER_ON, OUTPUT);
    pinMode(LED_CAMERA_CONTROL, OUTPUT);

    digitalWrite(LED_CONNECTED, 0);
    digitalWrite(LED_POWER_ON, 1);
    digitalWrite(LED_CAMERA_CONTROL, 0);
}

void wifiSetup() {
    WiFi.begin(SSID, PASSWORD);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void serverSetup() {
    // setup routing
    server.on("/", []{
        WebPageContent::send(&server);
    });
    server.on("/go", go);
    server.onNotFound([]{
        WebPageContent::pageNotFound(&server);
    });

    server.begin();
    Serial.println("HTTP server started");
}

void setup(void) {
    Serial.begin(115200);
    ledsSetup();
    wifiSetup();

    if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }

    serverSetup();
    stepperSetup();

    digitalWrite(LED_CONNECTED, 1);
}

//-- The main loop -------------------------------------------------------------------
void loop(void) {
    // If not in shooting mode handle client requests, otherwise don't listen for the client.
    // This is done to prevent latencies caused by the client-server communication during the process of making time-lapse.
    if(listenWiFi) {
        server.handleClient();
        return;
    }

    if(iterations != 0 && iterationsNumber > currentIteration) {

        Iteration& iteration   = iterations[currentIteration];
        int allStepsSize       = iteration.allStepsSize;
        int stepSize           = iteration.stepSize;
        int direction          = iteration.direction;
        unsigned long stayFor  = iteration.stayFor;

        if (shutterOn) {
            static bool stepperStopped = false;
            static bool shutterClosed = false;
            static bool pauseBetweenShutterMade = false;

            // Wait some time till stepper stop moving
            if(!stepperStopped && unblockedDelay(stepperDelay)) {
                return;
            }

            if (!stepperStopped) {
                stepperStopped = true;

                // Take a picture (or open the shutter if in bulb mode)
                camera->shutterNow();
            }

            // If in bulb mode close the shutter
            if(!shutterClosed && shutterMode == BULB_MODE) {
                if(unblockedDelay(shutterOpenTime)) {
                    return;
                }

                camera->shutterNow();
                shutterClosed = true;
            }

            // Wait till pauseBetweenShutter time is over
            if(unblockedDelay(pauseBetweenShutter - stepperDelay - shutterOpenTime)) {
                return;
            }

            shutterOn = false;
            stepperStopped = false;
            shutterClosed = false;
        }
        else if(stayFor > 0) {
            static unsigned long stayStartTime = 0;

            if (stayStartTime == 0) {
                stayStartTime = millis();
            }

            if (millis() - stayStartTime > stayFor) {
                stayStartTime = 0;
                currentIteration++;
            }
            else {
                shutterOn = true;
            }
        }
        else if (allStepsSize > 0) {
            int step = 0;

            if (allStepsSize > stepSize) {
                step = stepSize;
                iterations[currentIteration].allStepsSize -= stepSize;
            }
            else {
                step = allStepsSize;
                allStepsSize = 0;
                currentIteration++;
            }

            myStepper.step(step * direction);
            shutterOn = true;
        }
    }
    else {
        listenWiFi = true;
    }
}
