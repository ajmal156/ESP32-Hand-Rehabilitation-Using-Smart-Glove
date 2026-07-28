#include <Keypad.h>

// ---------------- Keypad ----------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] =
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13,12,14,27};
byte colPins[COLS] = {26,25,33,32};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Initialize Keypad


void setupKeypad(){
    Serial.println("Keypad Ready");
}


// Read Keypad

void readKeypad() {
    char key = keypad.getKey();

    if(key) {

        Serial.print("Key Pressed : ");
        Serial.println(key);

        switch(key) {
            
            case '1':
                fullGrip();
                sendData("Full Grip","3 Seconds");
                break;

            case '2':
                thumbExercise();
                sendData("Thumb Exercise","2 Seconds");
                break;

            case '3':
                releaseHand();
                sendData("Release Hand","1 Second");
                break;

            case '0':
                stopSystem();
                break;

            default:
                Serial.println("Invalid Key");
                break;
        }
    }
} 


