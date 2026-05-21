// Pin Definitions
const int NTC_PIN = A1;        // NTC thermistor
const int LDR_PIN = A0;        // Light dependent resistor
const int TRIG_PIN = 9;        // Ultrasonic trigger
const int ECHO_PIN = 8;        // Ultrasonic echo
const int IR_PIN = 7;          // IR-08H avoidance sensor
const int LED_PIN = 6;         // Status LED
const int BUZZER_PIN = 3;      // Passive buzzer

// Calibration Constants
const float B_COEFF = 3977;    // NTC beta coefficient
const float R_NOMINAL = 10000; // NTC resistance at 25°C
const int SAFE_DISTANCE = 20;  // Reduced to 20cm as requested
const int LIGHT_LOW = 300;     // Dark threshold
const int LIGHT_HIGH = 700;    // Bright threshold
const float TEMP_LOW = 18.0;   // Cold threshold (°C)
const float TEMP_HIGH = 30.0;  // Hot threshold (°C)

// System variables
bool systemActive = false;
unsigned long lastMotivation = 0;
unsigned long lastSuggestion = 0;
float ambientTemp = 0;
int ambientLight = 0;
float avgDistance = 0;
int readingCount = 0;

void setup() {
  Serial.begin(9600);
  
  // Pin configurations
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.println("=== SMART DESK MONITOR INITIALIZED ===");
  Serial.println("Place hand near IR sensor to activate system");
  
  // Startup sound
  playStartupTone();
}

void loop() {
  // Check if person is at desk
  bool personAtDesk = (digitalRead(IR_PIN) == LOW);
  
  if (personAtDesk) {
    if (!systemActive) {
      systemActive = true;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("\n>>> SYSTEM ACTIVATED - Person at desk <<<");
      playActivationTone();
      resetAmbientData();
    }
    
    // Monitor all environmental factors
    monitorEnvironment();
    
    // Provide periodic suggestions every 10 minutes
    if (millis() - lastSuggestion > 600000) { // 10 minutes
      providePeriodicSuggestions();
      lastSuggestion = millis();
    }
    
  } else {
    if (systemActive) {
      systemActive = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("\n>>> SYSTEM STANDBY - Desk empty <<<");
      playGoodbyeTone();
    }
    delay(2000);
  }
}

void monitorEnvironment() {
  // Read all sensors
  float temperature = readNTC();
  int lightLevel = analogRead(LDR_PIN);
  float distance = readUltrasonic();
  
  // Update ambient data for suggestions
  updateAmbientData(temperature, lightLevel, distance);
  
  // Safety checks
  bool distanceOK = checkDistance(distance);
  bool tempOK = checkTemperature(temperature);
  bool lightOK = checkLight(lightLevel);
  
  // Overall status
  bool allGood = distanceOK && tempOK && lightOK;
  
  // LED status indication
  if (allGood) {
    digitalWrite(LED_PIN, HIGH);
    // Motivational feedback every 30 seconds
    if (millis() - lastMotivation > 30000) {
      String mood = determineMood(temperature, lightLevel);
      playMoodTone(mood);
      Serial.print("💚 Perfect conditions! Your mood: ");
      Serial.println(mood);
      lastMotivation = millis();
    }
  } else {
    // Blink LED for warnings
    digitalWrite(LED_PIN, (millis() / 250) % 2);
  }
  
  // Print status dashboard
  printDashboard(temperature, lightLevel, distance, allGood);
  delay(500);
}

void updateAmbientData(float temp, int light, float dist) {
  // Update running averages
  ambientTemp = (ambientTemp * readingCount + temp) / (readingCount + 1);
  ambientLight = (ambientLight * readingCount + light) / (readingCount + 1);
  avgDistance = (avgDistance * readingCount + dist) / (readingCount + 1);
  readingCount++;
}

void resetAmbientData() {
  ambientTemp = 0;
  ambientLight = 0;
  avgDistance = 0;
  readingCount = 0;
}

String determineMood(float temp, int light) {
  if (temp < 20.0 && light < 400) return "Cozy Focus";
  if (temp > 26.0 && light > 600) return "Energetic";
  if (temp < 20.0) return "Calm";
  if (temp > 26.0) return "Intense";
  if (light < 400) return "Relaxed";
  if (light > 600) return "Creative";
  return "Productive";
}

void providePeriodicSuggestions() {
  Serial.println("\n===== PERIODIC WORKSPACE SUGGESTIONS =====");
  
  // Temperature suggestions
  Serial.print("🌡️  Temperature Trend: ");
  Serial.print(ambientTemp, 1);
  Serial.println("°C avg");
  if (ambientTemp < 20.0) {
    Serial.println("   → Consider warming up the space");
  } else if (ambientTemp > 26.0) {
    Serial.println("   → Try cooling or ventilating the area");
  } else {
    Serial.println("   → Ideal temperature range maintained");
  }
  
  // Lighting suggestions
  Serial.print("💡 Lighting Trend: ");
  Serial.print(ambientLight);
  Serial.println(" avg");
  if (ambientLight < 300) {
    Serial.println("   → Your space is consistently dim - add lighting");
  } else if (ambientLight > 700) {
    Serial.println("   → Consistent bright light - consider glare reduction");
  } else {
    Serial.println("   → Excellent lighting conditions maintained");
  }
  
  // Distance suggestions
  Serial.print("📏 Distance Trend: ");
  Serial.print(avgDistance, 1);
  Serial.println("cm avg");
  if (avgDistance < 25) {
    Serial.println("   → You're consistently close to your screen - check posture");
  } else {
    Serial.println("   → Good distance habits maintained");
  }
  
  // Mood encouragement
  String mood = determineMood(ambientTemp, ambientLight);
  Serial.print("💡 Mood Analysis: ");
  Serial.println(mood);
  Serial.println("   → Keep up the great work environment!");
  
  Serial.println("=========================================\n");
  
  // Play subtle notification tone
  tone(BUZZER_PIN, 880, 300); // A5 tone
  delay(400);
  noTone(BUZZER_PIN);
}

float readNTC() {
  int adc = analogRead(NTC_PIN);
  if (adc == 0) return -999; // Error check
  
  float R = 10000.0 / (1023.0 / adc - 1);
  float tempK = 1 / (log(R / R_NOMINAL) / B_COEFF + 1/298.15);
  return tempK - 273.15;
}

float readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1; // No echo
  
  return duration * 0.034 / 2;
}

bool checkDistance(float distance) {
  if (distance > 0 && distance < SAFE_DISTANCE) {
    // Only alert if significantly too close
    if (distance < SAFE_DISTANCE - 5) {
      tone(BUZZER_PIN, 2000, 300);
      Serial.println("⚠️  WARNING: Too close to screen! Move back!");
    }
    return false;
  }
  return true;
}

bool checkTemperature(float temp) {
  if (temp < TEMP_LOW) {
    // Only alert for significant cold
    if (temp < TEMP_LOW - 2) {
      tone(BUZZER_PIN, 800, 400);
      Serial.println("🥶 COLD: Room temperature low");
    }
    return false;
  } else if (temp > TEMP_HIGH) {
    // Only alert for significant heat
    if (temp > TEMP_HIGH + 2) {
      tone(BUZZER_PIN, 1200, 400);
      Serial.println("🔥 HOT: Room temperature high");
    }
    return false;
  }
  return true;
}

bool checkLight(int lightLevel) {
  if (lightLevel < LIGHT_LOW) {
    // Only alert for significant darkness
    if (lightLevel < LIGHT_LOW - 100) {
      tone(BUZZER_PIN, 600, 300);
      Serial.println("🌙 DIM: Low lighting detected");
    }
    return false;
  } else if (lightLevel > LIGHT_HIGH) {
    // Only alert for significant brightness
    if (lightLevel > LIGHT_HIGH + 100) {
      tone(BUZZER_PIN, 1000, 300);
      Serial.println("☀️ BRIGHT: High glare detected");
    }
    return false;
  }
  return true;
}

void printDashboard(float temp, int light, float dist, bool status) {
  Serial.println("════════════════════════════════");
  Serial.println("       SMART DESK MONITOR       ");
  Serial.println("════════════════════════════════");
  Serial.print("🌡️  Temperature: "); 
  Serial.print(temp, 1); 
  Serial.print(" °C (");
  if (temp < 20.0) Serial.print("Cool");
  else if (temp > 26.0) Serial.print("Warm");
  else Serial.print("Ideal");
  Serial.println(")");
  
  Serial.print("💡 Light Level: "); 
  Serial.print(light);
  Serial.print(" (");
  if (light < 400) Serial.print("Soft");
  else if (light > 600) Serial.print("Bright");
  else Serial.print("Balanced");
  Serial.println(")");
  
  Serial.print("📏 Distance: "); 
  Serial.print(dist); 
  Serial.print("cm (");
  Serial.print(dist < 25 ? "Close" : "Good");
  Serial.println(")");
  
  Serial.print("🎭 Current Mood: ");
  Serial.println(determineMood(temp, light));
  
  Serial.print("📊 Status: "); 
  Serial.println(status ? "✅ OPTIMAL" : "⚠️ NEEDS ATTENTION");
  Serial.println("════════════════════════════════\n");
}

void playStartupTone() {
  int startupMelody[] = {523, 659, 784}; // C5, E5, G5
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, startupMelody[i], 200);
    delay(250);
  }
  noTone(BUZZER_PIN);
}

void playActivationTone() {
  tone(BUZZER_PIN, 1319, 150); // E6
  delay(200);
  noTone(BUZZER_PIN);
}

void playGoodbyeTone() {
  tone(BUZZER_PIN, 1047, 150); // C6
  delay(200);
  noTone(BUZZER_PIN);
}

void playMoodTone(String mood) {
  if (mood == "Cozy Focus") {
    tone(BUZZER_PIN, 523, 300); // C5
  } 
  else if (mood == "Energetic") {
    tone(BUZZER_PIN, 784, 200); // G5
  }
  else if (mood == "Calm") {
    tone(BUZZER_PIN, 659, 300); // E5
  }
  else if (mood == "Intense") {
    tone(BUZZER_PIN, 988, 200); // B5
  }
  else { // Default motivational tone
    int motivationMelody[] = {659, 698, 784}; // E5, F5#, G5
    for (int i = 0; i < 3; i++) {
      tone(BUZZER_PIN, motivationMelody[i], 150);
      delay(180);
    }
  }
  delay(100);
  noTone(BUZZER_PIN);
}