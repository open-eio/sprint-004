
// Requires the Arduino-SerialCommand libarary, here: https://github.com/p-v-o-s/Arduino-SerialCommand
// In order to test in the Arduino IDE's serial monitor, make sure to use "Newline" instead of "No line ending"

//------- serial command stuff

#include <SerialCommand.h>
#define arduinoLED 13       // Arduino LED on board
SerialCommand sCmd(Serial); // The SerialCommand object, initialize with any Stream object
int LED_STAT=0;


//------------ thermistor stuff
// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    

int samples[NUMSAMPLES];



//---------------- depth stuff

#define CONDUCTIVITY_INTERRUPT 0  // assumes conductivity pin is connected to pin D2 (interrupt pin "0")
#define DEPTH_INTERRUPT 1     // assumes depth pin is connected to pin D3 (interrupt pin "1")

int samplingPeriod=1; // the number of seconds to measure 555 oscillations

// number of samples to average
#define DEFAULT_AVENUM 1
#define DEFAULT_SAMPLE_DURATION_MILLIS 1000
//int fivefivefive = 13; // the pin that powers the 555 subcircuit

int speakerPin = 9;



void setup()
{
  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("ON",          command_LED_on);          // Turns LED on
  sCmd.addCommand("OFF",         command_LED_off);         // Turns LED off
  sCmd.addCommand("LED_STATUS", command_LED_status);  //returns status of LED
  sCmd.setDefaultHandler(command_unrecognized);           // Handler for command that isn't matched  (says "What?")
  //startup comm
  sCmd.addCommand("MEAS_PULSE_MICROS",  dummy_MEAS_PULSE_MICROS);     // Measure average pulse period
 //sCmd.addCommand("MEAS_DEPTH_VALUES",  dummy_MEAS_DEPTH_VALUES);     // Measure average pulse period from pin D2
  sCmd.addCommand("MEAS_DEPTH_VALUES",  command_MEAS_DEPTH_VALUES);     // Measure average pulse period from pin D2


  sCmd.addCommand("MEAS_THERMISTOR", command_MEAS_THERMISTOR);

  //
  
  Serial.begin(9600);
           
  pinMode(speakerPin, OUTPUT);
               
}

void loop()
{
  
  sCmd.readSerial();     // We don't do much, just process serial commands
}

//------------------------------------------------------------------------------
// pulse measurement utilities
volatile long pulseCount = 0;  //a pulse counter variable
volatile unsigned long pulseTime,lastTime, duration, totalDuration;

float pulseAve=0.;
int pulseStats=0;


float measureAveragePeriod(unsigned int sampleDuration_millis){
  pulseCount=0;     //reset the pulse counter
  totalDuration=0;  //reset the totalDuration of all pulses measured
  pulseTime=micros(); // start the stopwatch
  attachInterrupt(0,irq_onPulse,RISING); //attach an interrupt counter to interrupt pin 0 (digital pin #2) -- the only other possible pin on the 328p is interrupt pin #1 (digital pin #3)
  //FIXME do NOT use delayMicroseconds, gives weird readings
  delay(sampleDuration_millis); //give ourselves samplingPeriod seconds to make this measurement, during which the "onPulse" function will count up all the pulses, and sum the total time they took as 'totalDuration' 
  detachInterrupt(0); //we've finished sampling, so detach the interrupt function -- don't count any more pulses
  return (totalDuration)/pulseCount;  //returns value as microseconds
}

float measureAveragePeriod(unsigned int sampleDuration_millis, int interrupt){  //interrupt = 0 (digital pin#2) or interrupt=1 (digital pin #3)

  
  pulseCount=0;     //reset the pulse counter
  totalDuration=0;  //reset the totalDuration of all pulses measured
  pulseTime=micros(); // start the stopwatch
  attachInterrupt(interrupt,irq_onPulse,RISING); //attach an interrupt counter to interrupt pin 0 (digital pin #2) -- the only other possible pin on the 328p is interrupt pin #1 (digital pin #3)
  //FIXME do NOT use delayMicroseconds, gives weird readings
  delay(sampleDuration_millis); //give ourselves samplingPeriod seconds to make this measurement, during which the "onPulse" function will count up all the pulses, and sum the total time they took as 'totalDuration' 
  detachInterrupt(interrupt); //we've finished sampling, so detach the interrupt function -- don't count any more pulses
  return (totalDuration)/pulseCount;  //returns value as microseconds
}



void irq_onPulse()
{
  pulseCount++;
  //Serial.print("pulsecount=");
  //Serial.println(pulseCount);
  lastTime = pulseTime;
  pulseTime = micros();
  duration=pulseTime-lastTime;
  totalDuration+=duration;
  //Serial.println(totalDuration);
}

//therimstor

float measure_thermistor() {
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  //Serial.print("Average analog reading "); 
  //Serial.println(average);
 
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  //Serial.print("Temperature "); 
 // Serial.print(steinhart);
  //Serial.println(" *C");
  return(steinhart);
  
}

//------------------------------------------------------------------------------

void command_MEAS_PULSE_MICROS(SerialCommand this_scmd) {
  int period; 
  int sampleDuration_millis = DEFAULT_SAMPLE_DURATION_MILLIS;
  char *arg = this_scmd.next();
  if (arg != NULL) {
    sampleDuration_millis = atoi(arg);
  }
  period = measureAveragePeriod(sampleDuration_millis);
  //send back result
  this_scmd.println(period);
}


void command_MEAS_DEPTH_VALUES(SerialCommand this_scmd) {
  int period_depth; 
  int period_conductivity; 
  float temperature;
  
  int sampleDuration_millis = DEFAULT_SAMPLE_DURATION_MILLIS;
  char *arg = this_scmd.next();
  if (arg != NULL) {
    sampleDuration_millis = atoi(arg);
  }
  
  period_depth = measureAveragePeriod(sampleDuration_millis, DEPTH_INTERRUPT);
  period_conductivity = measureAveragePeriod(sampleDuration_millis, CONDUCTIVITY_INTERRUPT);
  temperature = measure_thermistor();
  
  //send back result
  this_scmd.print("{\"depth\":");
  this_scmd.print(period_depth);
  this_scmd.print(",\"conductivity\":");
  this_scmd.print(period_conductivity);
  this_scmd.print(",\"temperature\":");
  this_scmd.print(temperature);
  this_scmd.println("}");
  
  this_scmd.println(period_depth);
}

void dummy_MEAS_DEPTH_VALUES(SerialCommand this_scmd) {
  int period_depth=3;
  int period_conductivity=4;
 float temperature=3.; 
  //send back result
    int sampleDuration_millis = DEFAULT_SAMPLE_DURATION_MILLIS;
  char *arg = this_scmd.next();
  if (arg != NULL) {
    sampleDuration_millis = atoi(arg);
  }
  //simulate delay of period measurement
  delay(sampleDuration_millis);
  //period = measureAveragePeriod(sampleDuration_millis);
  period_depth = 90 + random(0,20);
  period_conductivity = 90 + random(0,20);
  temperature = 90 + random(0,2);
  this_scmd.print("{\"depth\":");
  this_scmd.print(period_depth);
  this_scmd.print(",\"conductivity\":");
  this_scmd.print(period_conductivity);
  this_scmd.print(",\"temperature\":");
  this_scmd.print(temperature);
  this_scmd.println("}");
}




void command_MEAS_THERMISTOR(SerialCommand this_scmd) {
  float temp_celsius; 
  temp_celsius=measure_thermistor();
  this_scmd.println(temp_celsius);
}


void dummy_MEAS_PULSE_MICROS(SerialCommand this_scmd) {
  int period;
  int sampleDuration_millis = DEFAULT_SAMPLE_DURATION_MILLIS;
  char *arg = this_scmd.next();
  if (arg != NULL) {
    sampleDuration_millis = atoi(arg);
  }
  //simulate delay of period measurement
  delay(sampleDuration_millis);
  //period = measureAveragePeriod(sampleDuration_millis);
  period = 90 + random(0,20);
  //send back result
  this_scmd.println(period);
}






// serial command handlers
void command_LED_on(SerialCommand this_scmd) {
  this_scmd.println("LED on");
  digitalWrite(arduinoLED, HIGH);
  LED_STAT=1;
}

void command_LED_off(SerialCommand this_scmd) {
  this_scmd.println("LED off");
  digitalWrite(arduinoLED, LOW);
  LED_STAT=0;
}


void command_LED_status(SerialCommand this_scmd) {
  this_scmd.println(LED_STAT);
}


// This gets set as the default handler, and gets called when no other command matches.
void command_unrecognized(const char *command, SerialCommand this_scmd) {
  this_scmd.print("***ERROR *** Did not recognize \"");
  this_scmd.print(command);
  this_scmd.println("\" as a command.");
}

