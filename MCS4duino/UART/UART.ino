#define RESET_1   A1
#define RESET_2   A3
#define CLK1_1    0b000010000   // PORTB
#define CLK1_2    0b000010000   // PORTD
#define CLK2_1    0b000001000   // PORTB
#define CLK2_2    0b000001000   // PORTD
#define SYNC_1    10
#define SYNC_2    2
#define CM_ROM    A0
#define CM_RAM    A4
#define SEND_KEY  A2
#define TEST      7
#define DATA_32   0b00000011    // PORTB
#define DATA_10   0b01100000    // PORTD

#define CMD_RESET   0b0001
#define CMD_CLK1    0b0010
#define CMD_CLK2    0b0011
#define CMD_SYNC    0b0100
#define CMD_CM      0b0101
#define CMD_SEND_KEY 0b0110
#define CMD_TEST    0b0111
#define CMD_DATA_R  0b1000
#define CMD_DATA_W  0b1001


void reset(){
  DDRB = DDRB | CLK1_1 | CLK2_1 ;
  DDRD = DDRD | CLK1_2 | CLK2_2 ;
  
  digitalWrite(RESET_1, HIGH) ;
  digitalWrite(RESET_2, HIGH) ;
  PORTB = PORTB & ~(CLK1_1 | CLK2_1) ;
  PORTD = PORTD & ~(CLK1_2 | CLK2_2) ;
  digitalWrite(SYNC_1, LOW) ;
  digitalWrite(SYNC_2, LOW) ;
  digitalWrite(CM_ROM, LOW) ;
  digitalWrite(CM_RAM, LOW) ;
  digitalWrite(SEND_KEY, LOW) ;
  DDRB = DDRB & ~DATA_32 ;
  PORTB = PORTB & ~DATA_32 ;
  DDRD = DDRD & ~DATA_10 ;
  PORTD = PORTD & ~DATA_10 ;
}


void setup(){
  Serial.begin(2000000) ;
  pinMode(RESET_1, OUTPUT) ;
  pinMode(RESET_2, OUTPUT) ;
  pinMode(SYNC_1, OUTPUT) ;
  pinMode(SYNC_2, OUTPUT) ;
  pinMode(CM_ROM, OUTPUT) ;
  pinMode(CM_RAM, OUTPUT) ;
  pinMode(SEND_KEY, OUTPUT) ;
  pinMode(TEST, INPUT) ;
  reset() ;
  delay(1000) ;
  Serial.write(byte(1)) ;
}


void loop(){
  if (Serial.available()){ 
    int cmd = Serial.read() ;
    byte opr = cmd >> 4 ;
    byte opa = cmd & 0xF ;
    switch (opr){      
      case CMD_RESET:
        if (opa){
          reset() ;
        }
        else {
          digitalWrite(RESET_1, LOW) ;
          digitalWrite(RESET_2, LOW) ;
        }
        break ;
      case CMD_CLK1:
        if (opa){
          PORTB = PORTB | CLK1_1 ;
          PORTD = PORTD | CLK1_2 ;         
        }
        else {
          PORTB = PORTB & ~CLK1_1 ;
          PORTD = PORTD & ~CLK1_2 ;
        }
        break ;
      case CMD_CLK2:
        if (opa){
          PORTB = PORTB | CLK2_1 ;
          PORTD = PORTD | CLK2_2 ;         
        }
        else {
          PORTB = PORTB & ~CLK2_1 ;
          PORTD = PORTD & ~CLK2_2 ;
        }
        break ;
      case CMD_SYNC:
        digitalWrite(SYNC_1, opa) ;
        digitalWrite(SYNC_2, opa) ;
        break ;
      case CMD_CM:
        digitalWrite(CM_ROM, opa) ;
        digitalWrite(CM_RAM, opa) ;
        break ;
      case CMD_SEND_KEY:
        digitalWrite(SEND_KEY, opa) ;
        break ;
      case CMD_TEST:
        Serial.write(digitalRead(TEST)) ;
        break ;
      case CMD_DATA_R:
        if (opa){ // Z
          DDRB = DDRB & ~DATA_32 ;
          PORTB = PORTB & ~DATA_32 ;
          DDRD = DDRD & ~DATA_10 ;
          PORTD = PORTD & ~DATA_10 ;        
        }
        else {
          byte data = ((PINB & DATA_32) << 2) | ((PIND & DATA_10) >> 5) ; 
          Serial.write(data) ; 
        }
        break ;
      case CMD_DATA_W:
        DDRB = DDRB | DATA_32 ;
        DDRD = DDRD | DATA_10 ;
        PORTB = (PORTB & ~DATA_32) | (((opa >> 3) & 1) << 1) | ((opa >> 2) & 1) ;  
        PORTD = (PORTD & ~DATA_10) | (((opa >> 1) & 1) << 6) | ((opa & 1) << 5) ;  
        break ;
    }
  }
}