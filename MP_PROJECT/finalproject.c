#include <xc.h>
#define _XTAL_FREQ 8000000

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF


#define RS RB0
#define EN RB1
#define D4 RB2
#define D5 RB3
#define D6 RB4
#define D7 RB5


char card[13];   


const char VALID_IDS[3][13] =
 {
    "0000023l6162",
    "0000023l6163",
    "0000023l6164"
};

unsigned char attendance[3] = {0, 0, 0}; 


void LCD_Pulse(){
    EN = 1;
    __delay_us(40);
    EN = 0;
}

void LCD_Command(unsigned char cmd){
    RS = 0;
    D4 = (cmd >> 4) & 1;
    D5 = (cmd >> 5) & 1;
    D6 = (cmd >> 6) & 1;
    D7 = (cmd >> 7) & 1;
    LCD_Pulse();

    D4 = cmd & 1;
    D5 = (cmd >> 1) & 1;
    D6 = (cmd >> 2) & 1;
    D7 = (cmd >> 3) & 1;
    LCD_Pulse();
}

void LCD_Char(unsigned char data)
{
    RS = 1;
    D4 = (data >> 4) & 1;
    D5 = (data >> 5) & 1;
    D6 = (data >> 6) & 1;
    D7 = (data >> 7) & 1;
    LCD_Pulse();

    D4 = data & 1;
    D5 = (data >> 1) & 1;
    D6 = (data >> 2) & 1;
    D7 = (data >> 3) & 1;
    LCD_Pulse();
}

void LCD_String(const char *str)
{
    while(*str)
        LCD_Char(*str++);
}

void LCD_Clear()
{
    LCD_Command(0x01);
    __delay_ms(2);
}

void LCD_Init(){
    TRISB = 0x00;
    __delay_ms(20);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
}


void UART_Init()
{
    TRISC7 = 1;   
    TRISC6 = 0;   

    SPBRG = 51;   
    BRGH = 1;

    SYNC = 0;
    SPEN = 1;
    CREN = 1;
    TXEN = 1;
}

char UART_Read()
{
    while(!RCIF);
    return RCREG;
}


void main()
{
    unsigned char i, j, match;

    TRISC0 = 0; 
    TRISC1 = 0;  
    RC0 = 0;
    RC1 = 0;

    LCD_Init();
    UART_Init();

    LCD_String(" RFID SYSTEM");
    __delay_ms(1000);
    LCD_Clear();

    while(1){

        // --- Read 12 chars ---
        for(i = 0; i < 12; i++){
            card[i] = UART_Read();
        }
        card[12] = '\0';

        LCD_Clear();
        LCD_String("Card:");
        LCD_Command(0xC0);
        LCD_String(card);
        __delay_ms(600);

       
        unsigned char found = 0;

        for(i = 0; i < 3; i++)
        {
            match = 1;

            for(j = 0; j < 12; j++)
            {
                if(card[j] != VALID_IDS[i][j])
                {
                    match = 0; 
                    break;
                }
            }

            if(match == 1)  
            {
                found = 1;
                attendance[i] = 1;

                LCD_Clear();
                LCD_String("Attendance");
                LCD_Command(0xC0);
                LCD_String("Marked!");

                RC1 = 1;
                __delay_ms(400);
                RC1 = 0;

                break; 
            }
        }

        if(!found)
{
            LCD_Clear();
            LCD_String("Invalid Card!");
            RC0 = 1;
            __delay_ms(400);
            RC0 = 0;
        }

     
        LCD_Clear();
        LCD_String("Record:");
        for(i = 0; i < 3; i++)
         {
            LCD_Command(0xC0 + i);
            if(attendance[i] == 1)
                LCD_Char('P');
            else
                LCD_Char('A');
        }

        __delay_ms(800);
    }
}