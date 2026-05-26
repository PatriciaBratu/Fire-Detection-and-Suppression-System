#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// --- MODIFICARE EXCLUSIVĂ: PARAMETRII SERVO (Arc 120° cu bisectoare la 60°) ---
#define SERVO_MIN   2000    // Limita stângă (0 grade)
#define SERVO_MID   2666    // BISECTOAREA / Poziția inițială de repaus (60 grade)
#define SERVO_MAX   3333    // Limita dreaptă (120 grade)
#define PAS_SERVO   20      // Pasul de mișcare pentru un arc mai larg

volatile uint8_t stare_alerta = 0;

// Inițializare ADC pentru PC0 (Senzor Analogic)
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

// Inițializare Timer 1 pentru Servo pe PB1
void PWM_init(void) {
    DDRB |= (1 << PB1);
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8
    ICR1 = 39999; // 50 Hz
    
    // Servo pornește direct pe poziția de repaus/centru (Bisectoarea)
    OCR1A = SERVO_MID;
}

// Inițializare Întrerupere Externă INT0 pe PD2
void INT0_init(void) {
    DDRD &= ~(1 << PD2);  // PD2 ca intrare (legat la DO de la senzor)
    PORTD |= (1 << PD2);  // Activăm pull-up intern pentru stabilitate

    // Configurăm INT0 să reacționeze pe flanc căzător (Falling Edge)
    EICRA |= (1 << ISC01); // ISC01=1, ISC00=0 -> Falling edge
    EIMSK |= (1 << INT0);  // Activăm masca pentru întreruperea INT0
}

// Vectorul de Întrerupere Hardware pentru INT0 (Declanșat de pinul PD2)
ISR(INT0_vect) {
    stare_alerta = 1;      // Setăm flag-ul de alertă
    PORTD |= (1 << PD3);   // PORNEȘTE INSTANT BUZZERUL (pe PD3)
}

int main(void) {
    // Inițializare periferice hardware
    ADC_init();
    PWM_init();
    INT0_init();

    // Configurare ieșiri: PD4 (Pompă), PB5 (LED), PD3 (Buzzer)
    DDRD |= (1 << PD4) | (1 << PD3);
    DDRB |= (1 << PB5);

    // Activăm întreruperile globale (Laboratorul 3)
    sei();

    uint16_t valoare_foc = 1023;
    
    // Inițializăm variabila de mișcare direct din poziția de mijloc
    uint16_t pozitie_actula_servo = SERVO_MID;
    int8_t directie = 1;

    while (1) {
        // Citim valoarea analogică pentru a monitoriza precis intensitatea focului
        valoare_foc = ADC_read(0);

        // Controlul fin bazat pe ADC
        if (valoare_foc < 400) {
            
            // Păstrăm comportamentul tău pentru Pompă și LED
            PORTD &= ~(1 << PD4);   // Pornește Pompa (Logic inversat)
            PORTB |= (1 << PB5);    // Aprinde LED-ul de stare

            // --- NOUA MIȘCARE: Măturare pe arcul extins de 120° ---
            pozitie_actula_servo += (directie * PAS_SERVO);
            
            if (pozitie_actula_servo >= SERVO_MAX) { 
                pozitie_actula_servo = SERVO_MAX; 
                directie = -1; // Schimbă direcția înapoi spre stânga
            }
            if (pozitie_actula_servo <= SERVO_MIN) { 
                pozitie_actula_servo = SERVO_MIN; 
                directie = 1;  // Schimbă direcția înapoi spre dreapta
            }
            OCR1A = pozitie_actula_servo;

        } else {
            // Păstrăm comportamentul tău pentru starea de repaus
            PORTD |= (1 << PD4);   // Oprește pompa (Logic inversat)
            PORTB &= ~(1 << PB5);  // Stinge LED-ul
            PORTD &= ~(1 << PD3);  // Oprește Buzzerul
            
            stare_alerta = 0;      // Resetăm starea de alertă
            
            // --- NOUA RESETARE: Servo revine la poziția de mijloc (Bisectoare) ---
            pozitie_actula_servo = SERVO_MID;
            OCR1A = SERVO_MID;     // Servo revine stabil la 60 grade
        }

        _delay_ms(20); 
    }
}