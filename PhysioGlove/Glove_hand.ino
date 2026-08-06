// Hardware Pins
#define VALVE1_PIN 26      // Pressure Valve
#define VALVE2_PIN 25      // Release Valve

void setupGlove()
{
    pinMode(VALVE1_PIN, OUTPUT);
    pinMode(VALVE2_PIN, OUTPUT);

    digitalWrite(VALVE1_PIN, LOW); // initial set the valve off  
    digitalWrite(VALVE2_PIN, LOW); // initial set the valve off  

    Serial.println("Glove Hardware Ready");
}

// =======================
// 1. Full Hand Exercise
// =======================
void fullHand()
{
    Serial.println("=======================");   
    Serial.println("Full Hand Exercise");
    Serial.println("=======================");
    
    Serial.println("Exercise : Full Hand");

    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(VALVE2_PIN, LOW);
    delay(9000);

    digitalWrite(VALVE1_PIN, LOW);
    delay(500);

    digitalWrite(VALVE2_PIN, HIGH);
    delay(9000);

    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Full Hand Complete");
}

// =======================
// 2. Thumb Exercise
// =======================
void thumbExercise()
{
    Serial.println("=======================");  
    Serial.println("   Thumb Exercise      ");
    Serial.println("=======================");
    
    Serial.println("Exercise : Thumb");

    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(VALVE2_PIN, LOW);
    delay(5000);

    digitalWrite(VALVE1_PIN, LOW);
    delay(500);

    digitalWrite(VALVE2_PIN, HIGH);
    delay(5000);

    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Thumb Complete");
}

// =======================
// 3. Index Finger
// =======================
void indexFingerExercise()
{
    Serial.println("============================");   
    Serial.println("   Index Finger Exercise     ");
    Serial.println("=============================");

    Serial.println("Exercise : Index Finger");

    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(VALVE2_PIN, LOW);
    delay(4000);

    digitalWrite(VALVE1_PIN, LOW);
    delay(500);

    digitalWrite(VALVE2_PIN, HIGH);
    delay(4000);

    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Index Finger Complete");
}

// =======================
// 4. Middle Finger
// =======================
void middleFingerExercise()
{
    Serial.println("==============================");   
    Serial.println("   Middle Finger Exercise   ");
    Serial.println("===============================");
    
    Serial.println("Exercise : Middle Finger");

    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(VALVE2_PIN, LOW);
    delay(4000);

    digitalWrite(VALVE1_PIN, LOW);
    delay(500);

    digitalWrite(VALVE2_PIN, HIGH);
    delay(4000);

    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Middle Finger Complete");
}

// =======================
// 5. Ring + Little Finger
// =======================
void ringLittleExercise()
{
    Serial.println("===============================");   
    Serial.println("  Ring + Little Finger Exercise  ");
    Serial.println("================================");
    
    Serial.println("Exercise : Ring & Little Finger");

    digitalWrite(VALVE1_PIN, HIGH);
    digitalWrite(VALVE2_PIN, LOW);
    delay(4000);

    digitalWrite(VALVE1_PIN, LOW);
    delay(500);

    digitalWrite(VALVE2_PIN, HIGH);
    delay(4000);

    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Ring & Little Finger Complete");
}

// =======================
// Emergency Stop
// =======================
void stopSystem()
{
    Serial.println("=======================");   
    Serial.println("    Emergency Stop      ");
    Serial.println("=======================");
    
    digitalWrite(VALVE1_PIN, LOW);
    digitalWrite(VALVE2_PIN, LOW);

    Serial.println("Emergency Stop");
}
