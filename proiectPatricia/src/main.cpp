#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define SERVO_MIN   2000    
#define SERVO_MID   2666    
#define SERVO_MAX   3333    
#define PAS_SERVO   20      

volatile uint8_t stare_alerta = 0;

void ADC_init(void) {
    ADMUX = (1 << REFS0); 
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}

uint16_t ADC_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC); 
    while (ADCSRA & (1 << ADSC)); 
    return ADC;
}


void PWM_init(void) {
    DDRB |= (1 << PB1);
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); 
    ICR1 = 39999; 
    
   
    OCR1A = SERVO_MID;
}

void INT0_init(void) {
    DDRD &= ~(1 << PD2);  
    PORTD |= (1 << PD2);  

   
    EICRA |= (1 << ISC01); 
    EIMSK |= (1 << INT0);  
}


ISR(INT0_vect) {
    stare_alerta = 1;      
    PORTD |= (1 << PD3);   
}

int main(void) {
    
    ADC_init();
    PWM_init();
    INT0_init();

    
    DDRD |= (1 << PD4) | (1 << PD3);
    DDRB |= (1 << PB5);

   
    sei();

    uint16_t valoare_foc = 1023;
    
    
    uint16_t pozitie_actula_servo = SERVO_MID;
    int8_t directie = 1;

    while (1) {

        valoare_foc = ADC_read(0);

        
        if (valoare_foc < 400) {
            
            
            PORTD &= ~(1 << PD4);   
            PORTB |= (1 << PB5);    

            
            pozitie_actula_servo += (directie * PAS_SERVO);
            
            if (pozitie_actula_servo >= SERVO_MAX) { 
                pozitie_actula_servo = SERVO_MAX; 
                directie = -1; 
            }
            if (pozitie_actula_servo <= SERVO_MIN) { 
                pozitie_actula_servo = SERVO_MIN; 
                directie = 1;  
            }
            OCR1A = pozitie_actula_servo;

        } else {
            
            PORTD |= (1 << PD4);   
            PORTB &= ~(1 << PB5);  
            PORTD &= ~(1 << PD3);  
            
            stare_alerta = 0;      
            
            
            pozitie_actula_servo = SERVO_MID;
            OCR1A = SERVO_MID;     
        }

        _delay_ms(20); 
    }
}
