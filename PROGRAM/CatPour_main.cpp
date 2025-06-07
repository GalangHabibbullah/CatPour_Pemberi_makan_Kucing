#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// === Pin Definition ===
#define SERVO_PIN PB0
#define HX711_DT PD4
#define HX711_SCK PD5
#define TRIG_PIN PD2
#define ECHO_PIN PD3

// === Global Variable ===
volatile uint8_t feeding = 0;
volatile int currentAngle = 0;
char serialBuffer[32];
uint8_t serialIndex = 0;

// === Function Declarations ===
void USART_init();
void USART_send(char data);
void USART_print(const char* str);
void init_servo();
void set_servo_angle(uint8_t angle);
void hx711_init();
float hx711_read_grams();
float read_distance_cm();
void handle_command(const char* cmd);
void send_data();

// === USART Init ===
void USART_init() {
  UBRR0H = 0;
  UBRR0L = 103; // 9600 baud
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_send(char data) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = data;
}

void USART_print(const char* str) {
  while (*str) USART_send(*str++);
}

// === Servo PWM via Timer1 ===
void init_servo() {
  DDRB |= (1 << SERVO_PIN);
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // prescaler 8
  ICR1 = 40000; // 20ms period
  OCR1A = 1000; // default 0 deg
}

void set_servo_angle(uint8_t angle) {
  uint16_t pulse = 1000 + ((uint32_t)angle * 1000) / 180; // 1000-2000us
  OCR1A = pulse;
  currentAngle = angle;
}

// === HX711 Reading ===
void hx711_init() {
  DDRD &= ~(1 << HX711_DT); // input
  DDRD |= (1 << HX711_SCK); // output
  PORTD &= ~(1 << HX711_SCK);
}

long hx711_read_raw() {
  while (!(PIND & (1 << HX711_DT))); // tunggu data ready
  long value = 0;
  for (uint8_t i = 0; i < 24; i++) {
    PORTD |= (1 << HX711_SCK);
    _delay_us(1);
    value = (value << 1) | ((PIND & (1 << HX711_DT)) ? 1 : 0);
    PORTD &= ~(1 << HX711_SCK);
    _delay_us(1);
  }

  // 25th pulse untuk gain 128
  PORTD |= (1 << HX711_SCK);
  _delay_us(1);
  PORTD &= ~(1 << HX711_SCK);
  _delay_us(1);

  if (value & 0x800000) value |= 0xFF000000; // tanda negatif
  return value;
}

float hx711_read_grams() {
  long raw = hx711_read_raw();
  float calibrated = (raw - 8300000) / 984.0; // kalibrasi manual
  return calibrated;
}

// === Ultrasonik ===
float read_distance_cm() {
  // TRIG: Output, ECHO: Input
  DDRD |= (1 << TRIG_PIN);
  DDRD &= ~(1 << ECHO_PIN);
  PORTD &= ~(1 << TRIG_PIN);
  _delay_us(2);
  PORTD |= (1 << TRIG_PIN);
  _delay_us(10);
  PORTD &= ~(1 << TRIG_PIN);

  // Wait for echo high
  uint32_t timeout = 30000;
  while (!(PIND & (1 << ECHO_PIN)) && timeout--) _delay_us(1);

  uint32_t count = 0;
  while ((PIND & (1 << ECHO_PIN)) && count < 30000) {
    _delay_us(1);
    count++;
  }

  float distance = (count * 0.0343) / 2;
  return distance;
}

// === Perintah Serial ===
void handle_command(const char* cmd) {
  if (strcmp(cmd, "beri") == 0) {
    set_servo_angle(180);
    feeding = 1;
    USART_print("Memberi makan...\r\n");
  } else {
    USART_print("Perintah tidak dikenal.\r\n");
  }
}

// === Kirim Data ===
void send_data() {
  float jarak = read_distance_cm();
  float berat = hx711_read_grams();
  float kapasitas = (14.0 - jarak) / 12.0 * 100.0;
  if (kapasitas < 0) kapasitas = 0;
  if (kapasitas > 100) kapasitas = 100;

  if (feeding && berat >= 200.0) {
    set_servo_angle(0);
    feeding = 0;
    USART_print("Berat 200g tercapai. Servo ditutup.\r\n");
  }

  char buffer[64];
  USART_print("======================================\r\n");
  sprintf(buffer, "Servo           : %s (%d derajat)\r\n", currentAngle == 0 ? "TUTUP" : "BUKA", currentAngle);
  USART_print(buffer);
  sprintf(buffer, "Kapasitas       : %.1f %%\r\n", kapasitas);
  USART_print(buffer);
  sprintf(buffer, "Berat Timbangan : %.2f gram\r\n", berat);
  USART_print(buffer);
  sprintf(buffer, "Jarak Sensor    : %.2f cm\r\n", jarak);
  USART_print(buffer);
  USART_print("======================================\r\n\r\n");
}

int main(void) {
  USART_init();
  init_servo();
  set_servo_angle(0);
  hx711_init();
  _delay_ms(100);

  USART_print("Sistem siap. Ketik 'beri' untuk memberi makan.\r\n");

  while (1) {
    // Serial input
    if (UCSR0A & (1 << RXC0)) {
      char c = UDR0;
      if (c == '\n' || c == '\r') {
        if (serialIndex > 0) {
          serialBuffer[serialIndex] = '\0';
          handle_command(serialBuffer);
          serialIndex = 0;
        }
      } else if (serialIndex < sizeof(serialBuffer) - 1) {
        serialBuffer[serialIndex++] = c;
      }
    }

    send_data();
    _delay_ms(1000);
  }
}
