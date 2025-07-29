/*
 * Heart Rate Demonstration System with Manual BPM Control
 * 
 * Description:
 * This program simulates different heart rates and activates a solenoid
 * via relay when the heart rate drops below 20 BPM during a demonstration.
 * 
 * Hardware:
 * - Arduino board
 * - Relay module connected to pin 2 (controlling solenoid)
 * - 11V lithium-ion battery powering the solenoid
 * - Optional potentiometer on A0 for manual BPM control
 */

// Pin definitions
const int SOLENOID_PIN = 2;       // Digital pin controlling relay for solenoid
const int LED_PIN = 13;           // Built-in LED for heartbeat visualization
const int POT_PIN = A0;           // Optional: potentiometer for manual BPM control

// Heart rate parameters
const int HEART_RATE_THRESHOLD = 20;    // Activation threshold in BPM

// Variables for heart rate simulation
int currentBPM = 75;              // Starting BPM (normal)
unsigned long lastBeatTime = 0;
unsigned long beatInterval = 0;

// Timing variables
unsigned long lastDisplayTime = 0;
const int DISPLAY_INTERVAL = 2000;     // Display interval in ms (2 seconds)

// Demonstration modes
enum DemoMode {NORMAL, DROPPING, CRITICAL};
DemoMode currentMode = NORMAL;
unsigned long modeStartTime = 0;
const unsigned long MODE_DURATION = 30000;  // Duration of each mode in ms (30 seconds)

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Configure pins
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initially turn off solenoid
  digitalWrite(SOLENOID_PIN, LOW);
  
  // Print startup message
  Serial.println("===== HEART RATE DEMONSTRATION SYSTEM =====");
  Serial.println("This system will automatically decrease heart rate");
  Serial.println("from 75 BPM and activate the solenoid when it drops");
  Serial.println("below 20 BPM");
  Serial.println();
  Serial.println("DEMONSTRATION WILL BEGIN IN 5 SECONDS...");
  delay(5000);
  
  modeStartTime = millis();
}

void loop() {
  // Update demonstration mode based on timing
  updateDemoMode();
  
  // Update simulated heart rate based on current mode
  updateHeartRate();
  
  // Visual heartbeat simulation
  simulateHeartbeat();
  
  // Display information every 2 seconds
  if (millis() - lastDisplayTime > DISPLAY_INTERVAL) {
    displayStatus();
    lastDisplayTime = millis();
    
    // Control solenoid based on heart rate
    if (currentBPM < HEART_RATE_THRESHOLD) {
      // Activate solenoid for low heart rate
      digitalWrite(SOLENOID_PIN, HIGH);
      Serial.println("ALERT! Heart rate critical - SOLENOID ACTIVATED!");
    } else {
      // Deactivate solenoid for normal heart rate
      digitalWrite(SOLENOID_PIN, LOW);
      Serial.println("Heart rate above threshold - Solenoid OFF");
    }
    
    Serial.println("---------------------------------------");
  }
}

void updateDemoMode() {
  // Check if it's time to change modes
  if (millis() - modeStartTime > MODE_DURATION) {
    modeStartTime = millis();
    
    // Progress to next mode
    switch(currentMode) {
      case NORMAL:
        currentMode = DROPPING;
        Serial.println("\n>>> DEMONSTRATION: Heart rate beginning to drop <<<\n");
        break;
      case DROPPING:
        currentMode = CRITICAL;
        Serial.println("\n>>> DEMONSTRATION: Heart rate now CRITICAL <<<\n");
        break;
      case CRITICAL:
        // Stay in critical mode
        Serial.println("\n>>> DEMONSTRATION: Maintaining critical heart rate <<<\n");
        break;
    }
  }
}

void updateHeartRate() {
  // Option 1: Use potentiometer for manual control if available
  int potValue = analogRead(POT_PIN);
  if (potValue > 10) {  // Use a threshold to detect if potentiometer is being used
    // Map potentiometer value to BPM range (0-120)
    currentBPM = map(potValue, 0, 1023, 0, 120);
    return;
  }
  
  // Option 2: Automatic demonstration sequence
  unsigned long elapsedTime = millis() - modeStartTime;
  float progressFactor = (float)elapsedTime / MODE_DURATION;
  
  switch(currentMode) {
    case NORMAL:
      currentBPM = 75;  // Normal heart rate
      break;
    case DROPPING:
      // Gradually decrease BPM from 75 to 15 over 30 seconds
      currentBPM = 75 - (int)(60 * progressFactor);
      break;
    case CRITICAL:
      currentBPM = 15;  // Critical heart rate (below threshold)
      break;
  }
}

void simulateHeartbeat() {
  // Calculate beat interval based on current BPM
  beatInterval = 60000 / max(currentBPM, 1); // Prevent division by zero
  
  // Flash LED to simulate heartbeat
  if (millis() - lastBeatTime > beatInterval) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    lastBeatTime = millis();
  }
}

void displayStatus() {
  Serial.print("Current Mode: ");
  switch(currentMode) {
    case NORMAL:
      Serial.println("NORMAL");
      break;
    case DROPPING:
      Serial.println("DROPPING");
      break;
    case CRITICAL:
      Serial.println("CRITICAL");
      break;
  }
  
  Serial.print("Heart Rate: ");
  Serial.print(currentBPM);
  Serial.println(" BPM");
  
  Serial.print("Solenoid Status: ");
  if (currentBPM < HEART_RATE_THRESHOLD) {
    Serial.println("ACTIVATED");
  } else {
    Serial.println("OFF");
  }
}