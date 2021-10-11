#include "TIMING.h"
#include "DATA.h"
#include "IO.h"
#include "INST.h"
#include "CONTROL.h"
#include "SCRATCH.h"
#include "PINS.h"


#define RESET   A1
#define CLK1    12
#define CLK2    11
#define SYNC    10

TIMING TIMING(CLK1, CLK2, SYNC) ;
DATA DATA ;


void reset(){  
  TIMING.reset() ;
  DATA.reset() ;
  IO_reset() ;
  INST_reset() ;
  CONTROL_reset() ;
  SCRATCH_reset() ;
}


void setup(){
  Serial.begin(115200) ;
  Serial.println("4004") ;
  pinMode(RESET, INPUT) ;
  pinMode(CLK1, INPUT) ;
  pinMode(CLK2, INPUT) ;
  pinMode(SYNC, INPUT) ;
  
  IO_setup(&TIMING) ;
  INST_setup(&TIMING, &DATA) ;
  SCRATCH_setup(&TIMING, &DATA) ;
  CONTROL_setup(&TIMING, &DATA) ;
  reset() ;
}


void loop(){
  if (digitalRead(RESET)){
    return reset() ;
  }

  TIMING.loop() ;
}
