#include "TIMING.h"

#define DEBUG

#define RESET_ON                    PINC &   0b00000010
#define RESET_INPUT                 DDRC &= ~0b00000010
#define CM_ON                       PINC &   0b00000001
#define CM_INPUT                    DDRC &= ~0b00000001
#define DATA                        0b00111100 // PORTD
#define READ_DATA                   ((PIND & DATA) >> 2)
#define WRITE_DATA(data)            PORTD = ((PORTD & ~DATA) | (data << 2))  
#define DATA_INPUT                  DDRD &= ~DATA
#define DATA_OUTPUT                 DDRD |=  DATA

#define PRN_ADV_FIRE_COLOR          0b00111000
#define WRITE_ADV_FIRE_COLOR(data)  PORTC =  ((PORTC & ~PRN_ADV_FIRE_COLOR) | (data << 3))  
#define PRN_ADV_FIRE_COLOR_OUTPUT   DDRC  |= PRN_ADV_FIRE_COLOR

void io_write(byte data) ;
byte io_read() ;

TIMING TIMING ;

byte reg = 0 ;
byte chr = 0 ;
bool src = 0 ;
byte opa = 0 ;
byte opr = 0 ;
int chip_select = -1 ;
bool ram_inst = 0 ;
byte RAM[2][4][16] ;
byte STATUS[2][4][4] ;
unsigned long max_dur = 0 ;
byte dump_data = 255 ;


void reset(){
  DATA_INPUT ;   
  
  TIMING.reset() ;
  reg = 0 ;
  chr = 0 ;
  src = 0 ;
  opa = 0 ;
  chip_select = -1 ;
  max_dur = 0 ;
  dump_data = 255 ;
  
  for (byte i = 0 ; i < 2 ; i++){
    for (byte j = 0 ; j < 4 ; j++){
      for (byte k = 0 ; k < 16 ; k++){
        RAM[i][j][k] = 0 ; 
        if (k < 4){
          STATUS[i][j][k] = 0 ; 
        }
      }
    }
  }
  
  WRITE_ADV_FIRE_COLOR(0) ;
}


void dump_reg(byte reg){
  Serial.print(chip_select) ;
  Serial.print("/") ;
  Serial.print(reg) ;
  Serial.print(":") ;
  for (int k = 0 ; k < 16 ; k++){
    Serial.print(RAM[chip_select][reg][k], HEX) ; 
  }
  Serial.print("/") ;
  for (int k = 0 ; k < 4 ; k++){
    Serial.print(STATUS[chip_select][reg][k], HEX) ; 
  }
  Serial.println() ;
}


void setup(){
  #ifdef DEBUG
    Serial.begin(2000000) ;
    Serial.println("4002") ;
  #endif
  RESET_INPUT ;
  CM_INPUT ;
  PRN_ADV_FIRE_COLOR_OUTPUT ;
  TIMING.setup() ;
  reset() ;

  /* TIMING.A12clk1([]{
    if (dump_data != 255){ 
      Serial.print(opr, HEX) ; 
      Serial.print(opa, HEX) ; 
      Serial.print(chip_select, HEX) ;
      Serial.print(reg, HEX) ;
      Serial.print(chr, HEX) ; 
      Serial.println(dump_data, HEX) ;
    }
  }) ; */  


  TIMING.M22clk2([]{
    // Grab opa
    opa = READ_DATA ;
    if ((chip_select != -1)&&(CM_ON)){
      // If we are the selected chip for RAM/I/O and cm is on, the CPU is telling us that we are processing a RAM/I/O instruction
      ram_inst = 1 ;
    }
    else {
      ram_inst = 0 ;
    }
  }) ;


  TIMING.X22clk1([]{
    dump_data = 255 ;
    src = 0 ;
    if (CM_ON){
      // An SRC instruction is in progress
      byte data = READ_DATA ;
      byte chip = data >> 2 ;
      Serial.print("!") ;
      if (chip < 2){
        chip_select = chip ;
        // Grab the selected RAM register
        reg = data & 0b0011 ;
        src = 1 ;
        dump_data = 0 ;
        Serial.print("*") ;
      }
      else {
        chip_select = -1 ;
      }
    }
    else {                 
      if (ram_inst){
        // A RAM/I/O instruction is in progress, execute the proper operation according to the value of opa
        
        if ((opa & 0b1000) == 0){   // Write instructions
          io_write(READ_DATA) ;
        }
        else {
          byte data = io_read() ;
          if (data != 255){
            DATA_OUTPUT ;
            WRITE_DATA(data) ;
          }
        }
      }
    }
  }) ;

  
  /* TIMING.X22clk2([]{
    if (src){
      Serial.print("x") ;
    }
  }) ; */


  TIMING.X32clk1([]{
    // Disconnect from bus
    DATA_INPUT ;
    if (src){
      //Serial.print("y") ;
      chr = READ_DATA ;
      //Serial.print("c") ;
      //Serial.print(chr, HEX) ;
      //Serial.print(":") ;
    }
  }) ;
}


void io_write(byte data){
  byte prev = 255 ;

  if (opa == 0b0000){
    prev = RAM[chip_select][reg][chr] ;
    RAM[chip_select][reg][chr] = data ;
  }
  else if (opa == 0b0001){
    if (chip_select == 0){
      byte d = 0 ;
      if ((data >> 3) & 1){ // A3
        d |= 0b001 ;
      }
      if ((data >> 1) & 1){ // A4
        d |= 0b010 ;
      }
      if (data & 1){        // A5
        d |= 0b100 ;
      }
      WRITE_ADV_FIRE_COLOR(d) ;
    }
  }
  else if (opa >= 0b0100){
    byte i = opa & 0b0011 ;
    prev = STATUS[chip_select][reg][i] ;
    STATUS[chip_select][reg][i] = data ;
  }
  else {
    // The instruction is for the ROM chips or is not implemented.
    // Nothing happened from our perspective
    prev = data ;  
  }
  
  dump_data = (prev != data ? data : 255) ;
}


byte io_read(){
  byte data = 255 ;

  if ((opa == 0b1000)||(opa == 0b1001)||(opa == 0b1011)){
      data = RAM[chip_select][reg][chr] ;
  }
  else if (opa >= 0b1100){
    byte i = opa & 0b0011 ;
    data = STATUS[chip_select][reg][i] ;
  }

  dump_data = data ;

  return data ;
}


void loop(){
  while (1){
    #ifdef DEBUG
      unsigned long start = micros() ;
    #endif
    if (RESET_ON){
      return reset() ;
    }

    TIMING.loop() ;

    #ifdef DEBUG
      unsigned long dur = micros() - start ;
      if (dur > max_dur){
        max_dur = dur ;
        //Serial.print("Max:") ;
        //Serial.println(max_dur) ;
      }
    #endif
  }
}
