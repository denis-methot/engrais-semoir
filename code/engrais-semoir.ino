/*
Controle Débit Engrais Semoir Hari-Grains
(c) Denis Méthot, Ing. 2020
*/

#include <AccelStepper.h>
#include <stdint.h>

// Pins et définitions du stepper
// Concu pour driver TB6600, 4-microsteps, 800 steps/tour
const uint16_t STEPS_PAR_TOUR = (200 * 4);
const uint8_t MAX_TOUR_PAR_SEC = 3;
const uint16_t MAX_STEPS_PAR_SEC = (MAX_TOUR_PAR_SEC * STEPS_PAR_TOUR);
const uint8_t PIN_PUL = 3; // Step
const uint8_t PIN_DIR = 4; // Direction (En réserve)

AccelStepper stepper(AccelStepper::DRIVER, PIN_PUL, PIN_DIR);

// Entrées
const uint8_t AUGMENTE_V = 5;         // D5 = Augmente vitesse
const uint8_t DIMINUE_V = 7;          // D7 = Diminue vitesse
const uint8_t STOP = 9;               // D9 = Stop moteur
const uint16_t POLL_TIME_MS = 100;    // Lecture des entrees 10 fois par secondes
const uint8_t FULL_SCALE_SECONDS = 5; // Nombre de secondes pour atteindre la vitesse maximale
const uint16_t DELTA_SPEED_CHANGE_PER_POLL = ((POLL_TIME_MS / 100) * (MAX_STEPS_PAR_SEC / FULL_SCALE_SECONDS)) / 10;
const float MULTIPLICATEUR_MAX = 5.0; // Multiplicateur maximal à appliquer sur DELTA_SPEED_CHANGE_PER_POLL

void setup()
{
   // Inputs
   pinMode(AUGMENTE_V, INPUT);
   pinMode(DIMINUE_V, INPUT);
   pinMode(STOP, INPUT);

   // Setup le stepper
   stepper.setMaxSpeed(MAX_STEPS_PAR_SEC); // 3 tours par secondes max
   stepper.setSpeed(0);
}

void loop()
{
   stepper.runSpeed();
   check_inputs();
}

void check_inputs()
{
   static float curSpeed = 0.0;
   static float multiplicateur = 1.0;
   static unsigned long lastTimeMs = 0;
   int8_t cur_speed_dir = 0;
   static int8_t old_speed_dir = 0;
   unsigned long curTimeMs = millis();
   long deltaTimeMs;

   if (lastTimeMs > curTimeMs)
   {
      lastTimeMs = curTimeMs;
      return;
   }

   deltaTimeMs = curTimeMs - lastTimeMs;
   if (deltaTimeMs < POLL_TIME_MS)
   {
      return;
   }
   lastTimeMs += POLL_TIME_MS;

   // Sample Inputs
   if (digitalRead(AUGMENTE_V) == 1)
   {
      curSpeed = min(curSpeed + (DELTA_SPEED_CHANGE_PER_POLL*multiplicateur), MAX_STEPS_PAR_SEC);
      multiplicateur = min(multiplicateur+0.2,MULTIPLICATEUR_MAX);
      cur_speed_dir=1;
   }
   else if (digitalRead(DIMINUE_V) == 1)
   {
      curSpeed = max(curSpeed - (DELTA_SPEED_CHANGE_PER_POLL*multiplicateur), 0);
      multiplicateur = min(multiplicateur+0.2,MULTIPLICATEUR_MAX);
      cur_speed_dir=-1;
   }
   else if (digitalRead(STOP) == 1)
   {
      curSpeed = 0;
      multiplicateur = 1.0;
      cur_speed_dir=0;
   }
   else
   {
      multiplicateur = 1.0;
      cur_speed_dir=0;
   }

   if (old_speed_dir != cur_speed_dir)
   {
      multiplicateur = 1.0;
   }
   old_speed_dir = cur_speed_dir;

   stepper.setSpeed(curSpeed);
}
