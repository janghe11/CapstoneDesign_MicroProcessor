?/*
* Switch / 4x4 Keypad
*  Created on: 2016. 3. 4.
*      Author: Taehee Jang
*
* LCD / Rotary Switch
*  Created on: 2016. 3. 16.
*      Author: Jongwoo Jun
* 
* Compiler : IAR
*/

#include "io8535.h"
// ����ġ �Է� ��������
#define X0 PINB_Bit0
#define X1 PINB_Bit1
#define X2 PINB_Bit2
#define X3 PINB_Bit3
// RS232 ���� ��������
#define    RXC      7 //���� �Ϸ� ǥ�� ��Ʈ
#define    TXC      6 //�۽� �Ϸ� ǥ�� ��Ʈ
#define    UDRE     5 //�۽� ������ �������� �غ� �Ϸ� ǥ�� ��Ʈ

__flash char KCODE[16] = {0x00, 0x04, 0x08, 0x0c, 0x01, 0x05, 0x09, 0x0d, 0x02, 0x06, 0x0a, 0x0e, 0x03, 0x07, 0x0b, 0x0f};
__flash char SPINANGLE[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09};


// LCD �ʱ� ��� ȭ��
__flash unsigned char msg1[]="**** Hello **** ";
__flash unsigned char msg2[]="PUSH Button Plz ";

// 1.����� OPEN LCD ��� ȭ��
__flash unsigned char Door_lock1[] = "1-Door Lock Fun?";
__flash unsigned char Door_lock2[] = "2-PASSWORD :    ";

#include "LCD4.h"

// LCD for�� ��� ����
unsigned char k;

// 4�ڸ� Ű�� ������� �����ϱ� ���� ����
unsigned int keyRotate = 0;
// Ű 1�� �Է� ���� Save 1Key Row (from set_doorlock.h)
unsigned char KEY;
// Ư�� Ű ���� ���� Check specific key code (from set_doorlock.h)
unsigned char FLAG;
unsigned char KEY2;
// ��й�ȣ ���� ���� Save password (default is 0000)
unsigned char password[4] = "0000";
unsigned char inputPassword[4] = "0000";
// ��й�ȣ Ȯ�� ���� Check Password
int passwordCorrect;
int passwordWrong;
// Step Motor count�� ���� ���� ����
unsigned int spinCount, spinStep;
/*
* Key Matrix codes for ATmega8535.
* If you want to compile in Eclipse Ubuntu 14.04,remove "__flash" in front of "unsigned".
*/
// RS232 ��� �ʱ�ȭ
int init_rs232(void)
{
  UBRR = 23; //UART Baud Rate Register 3.6854MHz�� ��� 9600bps
  UCR = 0x18; //UART Control Register -> RXEN, TXEN Enable
  
  return 0;
}

// RS232 ������ �۽� �Լ�
char set_rs232Data(char data)
{
  // �����Ͱ� ���ͼ� �۽� ��� ���϶�
  while(!UDRE);
  // ������ �۽�
  UDR = data;
  
  return 0; 
}

// RS232 ������ ���� �Լ�
char get_rs232Data(void)
{
  // ���� Ȯ�� �������Ϳ� �����Ͱ� ������ ��
  while(!RXC);
  // ������ ����
  return UDR;
}

int delay(unsigned int i) {
  while (i--);
  return 0;
}

// Rotate Step Motor 180 left
int spinLeft(void) {
  spinCount = 200;
  spinStep = 7;
  do {
    PORTD = SPINANGLE[spinStep];
    spinStep--;
    spinStep &= 0x07;
    delay(60000);
  }while(spinCount--);
  
  return 0;
}

// Rotate Step Motor 180 right
int spinRight(void) {
  spinCount = 200;
  spinStep = 0;
  do {
    PORTD = SPINANGLE[spinStep];
    spinStep++;
    spinStep &= 0x07;
    delay(60000);
  }while(spinCount--);
  
  return 0;
}

// Catch 4x4 Hex Keypad Input
void SCAN()
{
  unsigned char i, temp, key1;
  KEY = key1 = 0;
  FLAG = 1;
  PORTA = 0xfe;
  asm ("nop");
  asm ("nop");
  
  temp = PINA;
  temp = (temp >> 4) | 0xf0;
  for (i=0; i<4; i++) {
    if (!(temp & 0x01)) {
      key1 = KEY; 
      FLAG = 0;
    }
    temp >>= 1;
    KEY++;
  }
  KEY = key1 & 0x0f;
}

void SCAN2()
{
  unsigned char i, temp, key1;
  KEY = key1 = 4;
  FLAG = 1;
  PORTA = 0xfd;
  asm ("nop");
  asm ("nop");
  
  temp = PINA;
  temp = (temp >> 4) | 0xf0;
  for (i=0; i<4; i++) {
    if (!(temp & 0x01)) {
      key1 = KEY; FLAG = 0;
    }
    temp >>= 1;
    KEY++;
  }
  KEY = key1 & 0x0f;
}

unsigned char SCAN3()
{
  unsigned char i, temp, key1;
  KEY = key1 = 8;
  FLAG = 1;
  PORTA = 0xfb;
  asm ("nop");
  asm ("nop");
  
  temp = PINA;
  temp = (temp >> 4) | 0xf0;
  for (i=0; i<4; i++) {
    if (!(temp & 0x01)) {
      key1 = KEY; FLAG = 0;
    }
    temp >>= 1;
    KEY++;
  }
  KEY = key1 & 0x0f;
  
  return KCODE[KEY];
}

void SCAN4()
{
  unsigned char i, temp, key1;
  KEY = key1 = 12;
  FLAG = 1;
  PORTA = 0xf7;
  asm ("nop");
  asm ("nop");
  
  temp = PINA;
  temp = (temp >> 4) | 0xf0;
  for (i=0; i<4; i++) {
    if (!(temp & 0x01)) {
      key1 = KEY; FLAG = 0;
    }
    temp >>= 1;
    KEY++;
  }
  KEY = key1 & 0x0f;
}

int passwordCheck()
{
  passwordWrong = 0;
  // keyRotate�� 4�� ���� �н����� �˻�
  for (keyRotate = 0; keyRotate < 4; keyRotate++) {
    // password�� ������ correct�� 1��
    if (password[keyRotate] != inputPassword[keyRotate])
      passwordWrong += 1;
  }
  // �н����尡 Ʋ���� 0�� ��ȯ, ������ 1 ��ȯ
  if(passwordWrong > 0)
    return 0;
  else
    return 1;
}

int main(void) {
  //DISPLAY function for MDA_Multi (Not working in regular ATmega header files.)
  L_INIT();
  DISPLAY();
  /*
  * DDRA : Keypad L0 ~ L3 (Input PA7 ~ PA4), C3 ~ C0 (Output PA3 ~ PA0)
  * DDRB : Switch (Input PB7 ~ PB4), Rotary Switch (Input PB3 ~ PB0)
  * DDRC : Speaker (Output PC7), Character LCD (Output PC6 ~ PC0)
  * DDRD : Debug LED (Output PD7, PD6), Tx / Rx (Output PD5 Input PD4), Step motor (Output PD3 ~ PD0)
  */
  DDRA = 0x0f;
  DDRB = 0x00;
  DDRC = 0xff;
  DDRD = 0xff;
  
  do {
    //1. Door lock & Step Motor Open Process
    if (X0) {
      // X0 Enable Debug LED Off
      PORTD = 0xff;
      // LCD Display -> Door lock and Password
      // LCD Ŭ����
      COMMAND(0x01);
      // 1���� ������ ���
      COMMAND(0x02);  // Ŀ���� Ȩ���� ��
      for (k = 0; k < 16; k++) {
        CHAR_O(Door_lock1[k]);   // �����͸� LCD�� ������ ���
      }
      // 2���� ������ ���
      COMMAND(0xc0);  // Ŀ���� ���� 2�� ��
      for (k = 0; k < 16; k++) {
        CHAR_O(Door_lock2[k]);   // �����͸� LCD�� ������ ���
      }
      // a�� ������ ��й�ȣ �Է� ��� ȭ��
      keyRotate = 0;
      while (1) 
      {
        //e ��ư�� ������ ��� �� ��й�ȣ �Է� ȭ��
        if(SCAN3() == 0x0b)
        {
          // Key�� ���۵����� �ι� ���� ���� ���� ���� delay
          delay(60000);
          keyRotate = 0;
          // a�� �����ų� ���� 4���� �Է��ϱ� ������ ���� �ݺ� ���
          while(1)
          {
            SCAN();
            if (!(FLAG == 1)) 
            {
              KEY2 = KCODE[KEY];
              // Key�� ���۵����� �ι� ���� ���� ���� ���� delay
              delay(60000);
            }
            
            SCAN2();
            if (!(FLAG == 1)) 
            {
              KEY2 = KCODE[KEY];
              delay(60000);
            }
            
            SCAN3();
            if (!(FLAG == 1)) 
            {
              KEY2 = KCODE[KEY];
              delay(60000);
            }
            
            SCAN4();
            if (!(FLAG == 1)) 
            {
              KEY2 = KCODE[KEY];
              delay(60000);
            }
            // ���ο� �н����� �Է�
            password[keyRotate] = KEY2;
            // a�� ������ �Է� ����
            if(SCAN3() == 0x0a)
              break;
            else
              keyRotate++;
          }
        }
        
        SCAN();
        if (!(FLAG == 1)) 
        {
          KEY2 = KCODE[KEY];
          // Key�� ���۵����� �ι� ���� ���� ���� ���� delay
          delay(60000);
        }
        
        SCAN2();
        if (!(FLAG == 1)) 
        {
          KEY2 = KCODE[KEY];
          delay(60000);
        }
        
        SCAN3();
        if (!(FLAG == 1)) 
        {
          KEY2 = KCODE[KEY];
          delay(60000);
        }
        
        SCAN4();
        if (!(FLAG == 1)) 
        {
          KEY2 = KCODE[KEY];
          delay(60000);
        }
        
        inputPassword[keyRotate] = KEY2;
        // a�� ������ �Է� ����
        if(SCAN3() == 0x0a)
        {
          // �н����� �˻�
          if(passwordCheck())
          {
            spinRight();
            //�ѹ��� Ȯ���� �Ǹ� �� ���ĺ��ʹ� �ۿ��� Ű�� ������ ������ ���� �ʱ� ������ X0 ��ư�� ������ ������ break�� ���� �������� ����.
            break;
          }
        } 
        else
          keyRotate++;
      }
      // 2. Door lock & Step Motor Close Process
    } 
    else if (X1) {
      spinLeft();
      // 3.Rotary Switch Boiler Process
    } 
    else if (X2) {
      // 4.Heating Gas Valve On/Off Process
    } 
    else if (X3) {
      
    }
  } while (1);
}


