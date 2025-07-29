
const int PULSE_SENSOR_PIN = A0;  // Analog input for HW-827 sensor
const int RELAY_PIN = 2;          // Digital pin for solenoid relay
const int LED_PIN = 13;           // Built-in LED for heartbeat visualization

// Threshold and timing settings
int threshold = 550;              // Signal threshold (may need adjustment)
const unsigned long BPM_THRESHOLD = 20;  // BPM threshold to activate solenoid

// Signal processing variables
int signalValue = 0;              // Current signal value
boolean isPulse = false;          // True when pulse is detected
unsigned long lastBeatTime = 0;   // Time of last detected beat
unsigned long currentTime = 0;    // Current time
int BPM = 0;                      // Current BPM value
boolean validBPM = false;         // Flag for valid BPM calculation

// BPM calculation variables
const int BEATS_TO_AVERAGE = 3;   
unsigned long beatTimes[3];       
int beatIndex = 0;             

// Safety check variables
unsigned long lastGoodReading = 0;// Time of last valid beat detection
const unsigned long NO_PULSE_TIMEOUT = 5000; // 5 second timeout for no pulse

void setup() {

  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  for (int i = 0; i < BEATS_TO_AVERAGE; i++) {
    beatTimes[i] = 0;
  }
  
  Serial.println("BPM");  
}

void loop() {
  // Read the pulse sensor value
  signalValue = analogRead(PULSE_SENSOR_PIN);
  currentTime = millis();
  
  // Basic beat detection algorithm
  if (signalValue > threshold && !isPulse) {
    // Rising edge detected - this is a beat
    isPulse = true;
    digitalWrite(LED_PIN, HIGH);  // Flash LED with heartbeat
    
    // Record the time of this beat
    beatTimes[beatIndex] = currentTime;
    beatIndex = (beatIndex + 1) % BEATS_TO_AVERAGE;
    
    // Update last good reading time
    lastGoodReading = currentTime;
    
    // Calculate BPM if we have enough data
    if (lastBeatTime > 0) {
      // Calculate average beat interval
      unsigned long totalInterval = 0;
      unsigned long validIntervals = 0;
      
      for (int i = 0; i < BEATS_TO_AVERAGE - 1; i++) {
        int currentIdx = (beatIndex - i - 1 + BEATS_TO_AVERAGE) % BEATS_TO_AVERAGE;
        int prevIdx = (beatIndex - i - 2 + BEATS_TO_AVERAGE) % BEATS_TO_AVERAGE;
        
        if (beatTimes[currentIdx] > 0 && beatTimes[prevIdx] > 0) {
          unsigned long interval = beatTimes[currentIdx] - beatTimes[prevIdx];
          
          // Only use reasonable intervals (30-220 BPM)
          if (interval > 273 && interval < 2000) {
            totalInterval += interval;
            validIntervals++;
          }
        }
      }
      
      // Calculate BPM if we have valid intervals
      if (validIntervals > 0) {
        unsigned long avgInterval = totalInterval / validIntervals;
        BPM = 60000 / avgInterval;
        validBPM = true;
        
        Serial.println(BPM);
      }
    }
    
    lastBeatTime = currentTime;
  } 
  else if (signalValue < (threshold - 50) && isPulse) {
    // Falling edge - pulse has ended
    isPulse = false;
    digitalWrite(LED_PIN, LOW);
  }
  
  // Check for low heart rate or no pulse
  if (validBPM && BPM < BPM_THRESHOLD) {
    // Activate solenoid for low heart rate
    digitalWrite(RELAY_PIN, HIGH);
  } 
  else if (currentTime - lastGoodReading > NO_PULSE_TIMEOUT && lastGoodReading > 0) {
    // Activate solenoid for no pulse detected
    digitalWrite(RELAY_PIN, HIGH);
  } 
  else {
    // Normal heart rate - keep solenoid off
    digitalWrite(RELAY_PIN, LOW);
  }
  
  // Small delay for stability
  delay(10);
}