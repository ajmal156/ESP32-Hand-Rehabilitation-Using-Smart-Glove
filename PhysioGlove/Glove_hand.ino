//  Hardware Pins 

#define PUMP_PIN    18

#define VALVE1_PIN  19      // Full Grip Valve
#define VALVE2_PIN  21      // Thumb Valve

// Initialize Hardware


void setupGlove() {
    pinMode(PUMP_PIN, OUTPUT);

    pinMode(VALVE1_PIN, OUTPUT);
    pinMode(VALVE2_PIN, OUTPUT);

    digitalWrite(PUMP_PIN, LOW);

    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Glove Hardware Ready");
}

// Full Hand Grip

void fullGrip()
{
    Serial.println("-----------------------");
    Serial.println("Exercise : Full Grip");

    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(VALVE1_PIN, HIGH);

    delay(3000);

    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(VALVE1_PIN, LOW);

    Serial.println("Full Grip Complete");
}

// Thumb Exercise


void thumbExercise()
{
    Serial.println("-----------------------");
    Serial.println("Exercise : Thumb");

    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(VALVE2_PIN, HIGH);

    delay(2000);

    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Thumb Exercise Complete");
}

//------------------------------------------------
// Release Hand
//------------------------------------------------

void releaseHand()
{
    Serial.println("-----------------------");
    Serial.println("Release Hand");

    digitalWrite(PUMP_PIN, LOW);

    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);

    delay(1000);

    Serial.println("Hand Released");
}

// Emergency Stop


void stopSystem()
{
    Serial.println("-----------------------");
    Serial.println("Emergency Stop");

    digitalWrite(PUMP_PIN, LOW);

    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("All Outputs OFF");
}    



