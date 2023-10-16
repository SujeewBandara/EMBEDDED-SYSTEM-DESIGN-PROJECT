#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#define E (1<<PD3)
#define RS (1<<PD2)

void adcinit();
int adcread(char);


void lcdinit();
void lcdcmd(uint8_t);
void lcdchar(uint8_t);
void lcdstr(unsigned char *);
void latch(void);
void lcdgoto(uint8_t , uint8_t );


// Define macros for ports and pins
#define TRIGGER_PIN PB0
#define ECHO_PIN PB1
#define PUMP_PIN PB4
#define SET_VAL_PIN PB2
#define AUTO_MODE_PIN PB3

// Variables
long duration;
long inches;
int set_val;
int percentage;
bool state, pump;
int Percent;
char myConstantString[2]; // Allocate enough space to hold the string representation

// Function prototypes
long microsecondsToInches(long microseconds);
void setup();
void loop();

int main(void) {
    setup();
   
    
    // Use sprintf to convert the constant to a string
    

    // Now myConstantString contains the string representation of the constant
    

    while (1) {
        loop();
    }

    return 0;
}

void setup() {
    lcdinit();     // Initialize LCD
    lcdclear();              // Clear LCD screen

    lcdgoto(1, 1);
    lcdstr("WATER LEVEL:");  // Print "WATER LEVEL:"

    lcdgoto(1, 2);
    lcdstr("PUMP:OFF MANUAL"); // Print "PUMP:OFF MANUAL"

    DDRB |= (1 << TRIGGER_PIN);  // Set TRIGGER_PIN as OUTPUT
    DDRB &= ~(1 << ECHO_PIN);    // Set ECHO_PIN as INPUT
    DDRB |= (1 << PUMP_PIN);     // Set PUMP_PIN as OUTPUT
    DDRB &= ~(1 << SET_VAL_PIN); // Set SET_VAL_PIN as INPUT
    DDRB &= ~(1 << AUTO_MODE_PIN); // Set AUTO_MODE_PIN as INPUT

    set_val = eeprom_read_byte((uint8_t*)1); // Read set_val from EEPROM at address 1
    if (set_val > 160)
        set_val = 160; // If the value is greater than 135, set it to 135.
}

void loop() {
    // Clear the trigger pin
    PORTB &= ~(1 << TRIGGER_PIN);
    _delay_us(2);

    // Send a 10 microsecond high pulse to the trigger pin
    PORTB |= (1 << TRIGGER_PIN);
    _delay_us(10);
    PORTB &= ~(1 << TRIGGER_PIN);

    // Measure the duration of the pulse
    duration = pulseIn(ECHO_PIN, HIGH);

    // Convert the duration to inches
    inches = microsecondsToInches(duration);

     percentage = (set_val - inches) * 100 / set_val;
    
      
    if (percentage < 2)
        percentage = 0;
        sprintf(myConstantString, "%d", percentage);
        lcdgoto(1, 1);
        lcdstr("WATER LEVEL:");  // Print "WATER LEVEL:"
        lcdgoto(14, 1);
        lcdstr(myConstantString);
        lcdstr("%   ");

    if (percentage < 2 && (PINB & (1 << AUTO_MODE_PIN)))
        pump = 1;
    if (percentage > 97)
        pump = 0;
    PORTB = (PORTB & ~(1 << PUMP_PIN)) | (pump << PUMP_PIN);

   
    if (pump == 1)
        {lcdgoto(1, 2);
        lcdstr("PUMP:"); // Print "PUMP:OFF MANUAL"
        lcdstr("ON ");
        }
    else if (pump == 0)
       {lcdgoto(1, 2);
        lcdstr("PUMP:");
        lcdstr("OFF");
       }


    if (!(PINB & (1 << AUTO_MODE_PIN)))
       {lcdgoto(10, 2);
        lcdstr("MANUAL");
       }
    else if (!(PINB & (0 << AUTO_MODE_PIN)))
         {lcdgoto(10, 2);
          lcdstr("AUTO   ");
         }

    if (!(PINB & (1 << SET_VAL_PIN)) && !state && (PINB & (1 << AUTO_MODE_PIN))) {
        state = 1;
        set_val = inches;
        eeprom_write_byte((uint8_t*)1, set_val);
    }

    if (!(PINB & (1 << SET_VAL_PIN)) && !state && !(PINB & (0 << AUTO_MODE_PIN))) {
        state = 1;
        pump = !pump;
    }

    if (PINB & (1 << SET_VAL_PIN))
        state = 0;

    // Delay for 50 milliseconds
    _delay_ms(50);
}

long microsecondsToInches(long microseconds) {
    return microseconds / 74 / 2;
}

void adcinit(){ 
  //make PA0 an analog input
  DDRC &= ~(1<<PC0);          /* Make ADC port as input */
  //enable ADC module, set prescalar of 128 which gives CLK/128   
  ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  //set the voltage reference using REFS1 and REFS0 bits and select the ADC channel using the MUX bits
  ADMUX = 0b01000000;      // set REFS1 = 0 |REFS0 = 1 (Vref as AVCC pin) | ADLAR = 0(right adjusted) |  MUX4 to MUX0 is 0000 for ADC0
}

int adcread(char channel)             
{
  /* set input channel to read */
  ADMUX = 0x40 | (channel & 0x07);   // 0100 0000 | (channel & 0000 0100)
  /* Start ADC conversion */
  ADCSRA |= (1<<ADSC);      
    /* Wait until end of conversion by polling ADC interrupt flag */
  while (!(ADCSRA & (1<<ADIF)));    
   /* Clear interrupt flag */
  ADCSRA |= (1<<ADIF);               
  _delay_ms(1);                      /* Wait a little bit */
    /* Return ADC word */
  return ADCW;                      
}

void lcdinit(){
   
   //initialize PORTs for LCD
   DDRD |= (1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7); 
   
   _delay_ms(50);
   PORTD &= ~E;    //send low
   _delay_ms(50);  //delay for stable power
   lcdcmd(0x33);
   //_delay_us(100);
  lcdcmd(0x32);
  //_delay_us(100);
   lcdcmd(0x28);  // 2 lines 5x7 matrix dot
  // _delay_us(100);
    lcdcmd(0x0C);  // display ON, Cursor OFF
 // _delay_us(100);
   lcdcmd(0x01);  //clear LCD
 //  _delay_us(2000);
   lcdcmd(0x06);  //shift cursor to right
   _delay_us(1000);
   }
   
   void lcdcmd(unsigned char cmd){
      PORTD = (PORTD & 0x0F) | (cmd & 0xF0);  // send high nibble
     // PORTD &= ~RW; //send 0 for write operation
      PORTD &= ~RS; //send 0 to select command register
      PORTD |= E;   //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
   //   _delay_us(20);    //wait
      
      PORTD = (PORTD & 0x0F) | (cmd<<4);  //send low nibble 
       PORTD |= E;    //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
//_delay_us(20);    //wait
      }
      
  void lcdchar(unsigned char data){
      
      PORTD = (PORTD & 0x0F) | (data & 0xF0);  // send high nibble
      //PORTD &= ~RW; //send 0 for write operation
      PORTD |= RS;  //send 1 to select data register
      PORTD |= E;   //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
     
      PORTD = (PORTD & 0x0F) | (data<<4);  // send low nibble
      PORTD |= E;   //send high
      _delay_ms(50);    //wait
      PORTD &= ~E;    //send low
      
      }
      
 void lcdstr(unsigned char *str){
    unsigned char k=0;
    while(str[k] != 0){
   lcdchar(str[k]);
       k++;
       }
    }
 
 void lcdgoto(unsigned char x, unsigned char y){
      unsigned char firstcharadr[] = {0x80, 0xC0, 0x94, 0xD4};
      lcdcmd(firstcharadr[y-1] + x-1);
      _delay_us(100);
    }

void lcdclear(){
  lcdcmd(0x01);
  _delay_ms(1);
}