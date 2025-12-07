
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int MOTOR_PIN = 3; 

const float MAX_PWM = 100;    
const float NEAR_PWM  = 100;      
const float MID_PWM   = 100;     
const int FAR_PWM   = 80;      
const int RAMP_STEP = 8;       
const unsigned long ECHO_TIMEOUT = 30000UL; 

const int  BURST_NEAR_DISTANCE_CM = 30; 
const unsigned long BURST_PERIOD_MS = 1000UL; 
const unsigned long BURST_ON_MS_MIN = 100UL; 
const unsigned long BURST_ON_MS_MAX = 400UL;  

unsigned long calcBurstOnForDistance(int dist) {
  if (dist <= 0) return BURST_ON_MS_MAX;
  if (dist >= BURST_NEAR_DISTANCE_CM) return BURST_ON_MS_MIN;
  float frac = 1.0f - (float(dist) / float(BURST_NEAR_DISTANCE_CM)); 
  return (unsigned long)(BURST_ON_MS_MIN + frac * (BURST_ON_MS_MAX - BURST_ON_MS_MIN) + 0.5f);
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  analogWrite(MOTOR_PIN, 0);
  Serial.begin(9600);
}

int readDistanceCM(){
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long dur = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT); 
  if (dur == 0) return 999; 
  float cm = (dur * 0.0343f) / 2.0f;
  return (int)(cm + 0.5f);
}

void loop(){
  int dist = readDistanceCM();

  int targetPwm = 0;
  if(dist >= 300)         targetPwm = 0;    
  else if(dist >= 200)    targetPwm = FAR_PWM; 
  else if(dist >= 100)    targetPwm = MID_PWM;
  else if(dist<100)                  targetPwm = NEAR_PWM;
  else targetPwm = 0;


  if (targetPwm > MAX_PWM) targetPwm = MAX_PWM;

  static int curPwm = 0;
  if (curPwm < targetPwm) curPwm += RAMP_STEP;
  else if (curPwm > targetPwm) curPwm -= RAMP_STEP;
  curPwm = constrain(curPwm, 0, MAX_PWM);

  bool burstActive = (dist <= BURST_NEAR_DISTANCE_CM && curPwm > 0);

  static unsigned long cycleStart = 0;
  unsigned long now = millis();
  if (cycleStart == 0) cycleStart = now;

  unsigned long phase = (now - cycleStart) % BURST_PERIOD_MS;
  unsigned long onTime = calcBurstOnForDistance(dist);

  int outputPwm = 0;
  if (!burstActive) {
    outputPwm = curPwm;
  } else {
    if (phase < onTime) outputPwm = curPwm;
    else outputPwm = 0;
  }

  analogWrite(MOTOR_PIN, outputPwm);

  Serial.print("Dist(cm): ");
  Serial.print(dist);
  Serial.print("  TargetPWM: ");
  Serial.print(targetPwm);
  Serial.print("  CurPwm: ");
  Serial.print(curPwm);
  Serial.print("  OutPwm: ");
  Serial.print(outputPwm);
  Serial.print("  Burst: ");
  Serial.print(burstActive ? "ON" : "OFF");
  if (burstActive) {
    Serial.print("  onTime:");
    Serial.print(onTime);
    Serial.print("ms");
  }
  Serial.println();

  delay(60);
}
