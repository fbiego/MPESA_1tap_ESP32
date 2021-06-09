#define PIEZO_PIN 2

/////////////////////////////////////////////////////////////
void ToneOut(int pitch, int duration) { // pitch in Hz, duration in ms
  int delayPeriod;
  long cycles, i;

  pinMode(PIEZO_PIN, OUTPUT);           // turn on output pin
  delayPeriod = (500000 / pitch) - 7;   // calc 1/2 period in us -7 for overhead
  cycles = ((long)pitch * (long)duration) / 1000; // calc. number of cycles for loop

  for (i = 0; i <= cycles; i++) {       // play note for duration ms
    digitalWrite(PIEZO_PIN, HIGH);
    delayMicroseconds(delayPeriod);
    digitalWrite(PIEZO_PIN, LOW);
    delayMicroseconds(delayPeriod - 1); // - 1 to make up for digitaWrite overhead
  }
  pinMode(PIEZO_PIN, INPUT);            // shut off pin to avoid noise from other operations
}
