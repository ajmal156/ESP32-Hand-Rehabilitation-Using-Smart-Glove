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

byte rowPins[ROWS] = {13,12,14,27}; // to set the Row ON KEYPAD
byte colPins[COLS] = {18,19,21,22}; // to set the COLUMN ON KEYPAD

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Initialize Keypad

void setupKeypad(){
    Serial.println("Keypad Ready");
}

// Read Keypad to controll on the keypad 
void readKeypad() {
    char key = keypad.getKey();

    if(key) {

        Serial.print("Key Pressed : ");
        Serial.println(key);

        switch(key){
            case '1': // set the full group 
            fullHand();
            sendData("Full Hand", "9 Seconds");
            break;

            case '2': // set the thumb exercise 
                thumbExercise();
                sendData("Thumb", "5 Seconds");
                break;

            case '3': // set the index finger exercise
                indexFingerExercise();
                sendData("Index Finger", "4 Seconds");
                break;

            case '4': // set the middle finger exercise 
                middleFingerExercise();
                sendData("Middle Finger", "4 Seconds");
                break;

            case '5': // set the ring + little finger exercise
                ringLittleExercise();
                sendData("Ring & Little Finger", "4 Seconds");
                break;

            case '0':
                stopSystem();
                break;
        }
    }
}
