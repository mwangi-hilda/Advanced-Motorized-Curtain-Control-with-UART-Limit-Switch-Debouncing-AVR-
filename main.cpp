#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>

#define MOTOR_DDR DDRB
#define MOTOR_PORT PORTB
#define MOTOR_IN1 PB5
#define MOTOR_IN2 PB7

#define MOTOR_SWITCH_DDR DDRJ
#define MOTOR_SWITCH_PORT PORTJ
#define MOTOR_SWITCH_PIN PINJ
#define MOTOR_OPEN_SWITCH PJ0
#define MOTOR_CLOSE_SWITCH PJ1

#define RX_BUFFER_SIZE 32

#define DEBOUNCE_TIME_MS 20

//curtain switches' global variables
volatile uint8_t open_switch_counter = 0;
volatile uint8_t close_switch_counter = 0;
bool open_switch_triggered = false;
bool close_switch_triggered = false;

//curtain state flags global variables
bool curtain_fully_open = false;
bool curtain_fully_closed = false;

//uart0 global variables
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t new_message_flag = 0;

//motor global variables
bool curtain_is_opening = false;
bool curtain_is_closing = false;

//uart0 function prototypes
void uart0_init(uint32_t baud_rate);
void uart0_transmit(char data);
void uart0_send_string(const char *str);

//motor function prototypes
void open_curtain();
void close_curtain();
void stop_motor();

ISR(USART0_RX_vect){
	char received_char = UDR0;
	
	if (received_char == '\r' || received_char == '\n')
	{
		if (rx_index > 0)
		{
			rx_buffer[rx_index] = '\0';
			new_message_flag = 1;
			rx_index = 0;
		}
	} else if (rx_index < (RX_BUFFER_SIZE - 1))
	{
		rx_buffer[rx_index++] = received_char;
	}
}

int main(void)
{
	MOTOR_DDR |= (1<<MOTOR_IN1) | (1<<MOTOR_IN2);
	
	MOTOR_SWITCH_DDR &= ~((1<<MOTOR_OPEN_SWITCH) | (1<<MOTOR_CLOSE_SWITCH));
	MOTOR_SWITCH_PORT |= (1<<MOTOR_OPEN_SWITCH) | (1<<MOTOR_CLOSE_SWITCH);
	
    uart0_init(9600);
	
	sei();
	
	uart0_send_string("MCU Ready!\r\n");
    while (1) 
    {
		if (new_message_flag)
		{
			new_message_flag = 0;
			
			if (strstr((const char*)rx_buffer, "open curtain") != NULL)
			{
				if (!curtain_fully_open)
				{
					open_curtain();
					uart0_send_string("Opening curtain......\r\n");
				} else
				{
					uart0_send_string("Curtain is already open.\r\n");
				}
			} else if (strstr((const char*)rx_buffer, "close curtain") != NULL)
			{
				if (!curtain_fully_closed)
				{
					close_curtain();
					uart0_send_string("Closing curtain......\r\n");
				} else
				{
					uart0_send_string("Curtain is already closed.\r\n");
				}
			} else if (strstr((const char*)rx_buffer, "stop curtain") != NULL)
			{
				stop_motor();
				uart0_send_string("Stopping the curtain......\r\n");
			} else
			{
				uart0_send_string("Unknown command!\r\n");
			}
			
			memset((void*)rx_buffer, 0, RX_BUFFER_SIZE);
		}
		
		if (!(MOTOR_SWITCH_PIN & (1<<MOTOR_OPEN_SWITCH)) && curtain_is_opening)
		{
			if (open_switch_counter < DEBOUNCE_TIME_MS)
			{
				open_switch_counter++;
			} else if (!open_switch_triggered)
			{
				stop_motor();
				open_switch_triggered = true;
				uart0_send_string("Curtain fully open.\r\n");
				curtain_fully_open = true;
				curtain_fully_closed = false;
			}
		} else
		{
			open_switch_counter = 0;
			open_switch_triggered = false;
		}
		
		if (!(MOTOR_SWITCH_PIN & (1<<MOTOR_CLOSE_SWITCH)) && curtain_is_closing)
		{
			if (close_switch_counter < DEBOUNCE_TIME_MS)
			{
				close_switch_counter++;
			} else if (!close_switch_triggered)
			{
				stop_motor();
				close_switch_triggered = true;
				uart0_send_string("Curtain fully closed.\r\n");
				curtain_fully_open = false;
				curtain_fully_closed = true;
			}	
		} else
		{
			close_switch_triggered = false;
			close_switch_counter = 0;
		}
    }
}

void uart0_init(uint32_t baud_rate){
	uint16_t ubrr_value = (F_CPU / (8 * baud_rate)) - 1;
	UBRR0H = (uint8_t) (ubrr_value>>8);
	UBRR0L = (uint8_t) ubrr_value;
	
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
	UCSR0B &= ~(1<<UCSZ02);
	
	UCSR0C &= ~((1<<UMSEL01) | (1<<UMSEL00) | (1<<UPM01) | (1<<UPM00) | (1<<USBS0));
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
	
	UCSR0A |= (1<<U2X0);
}

void uart0_transmit(char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void uart0_send_string(const char *str){
	while(*str){
		uart0_transmit(*str++);
	}
}

void open_curtain(){
	if(!curtain_is_opening)
	{
		MOTOR_PORT |= (1<<MOTOR_IN1);
		MOTOR_PORT &= ~(1<<MOTOR_IN2);
		_delay_ms(50);
		curtain_is_opening = true;
		curtain_is_closing = false;
	}
}

void close_curtain(){
	if(!curtain_is_closing)
	{
		MOTOR_PORT |= (1<<MOTOR_IN2);
		MOTOR_PORT &= ~(1<<MOTOR_IN1);
		_delay_ms(50);
		curtain_is_opening = false;
		curtain_is_closing = true;
	}
}

void stop_motor(){
	MOTOR_PORT &= ~((1<<MOTOR_IN1) | (1<<MOTOR_IN2));
	curtain_is_opening = false;
	curtain_is_closing = false;
}