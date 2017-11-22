#include <SPI.h>
#include <IRremote.h>
#include "LCD_Functions.h"
//#include "DS1032_Functions.h"
#include "CR_and_Pins_Mapping.h"

#define OPTION1 0
#define OPTION2 1
#define OPTION3 2
#define OPTION4 3

const int RECV_PIN = 8;           // IR receptor
IRrecv irrecv(RECV_PIN);
decode_results command; // Decoding of IR symbols
int option = OPTION1;
int cursor_pos = 0;
boolean state[] = {false,false,false}; // Pump1, Pump2
char lline[15] = "[MANUAL      ]";
int previous_screen = -1;
int screen = SCREEN_1;
boolean display_clock = true;
boolean backlight = true;
int flow;
volatile int NbTopsFan;
boolean changedLightState = false;

int encoding_menu[11][2] = {
  { SCREEN_1, 4   },    // SCREEN_1
  { SCREEN_1, 2   },    // SCREEN_11
  { SCREEN_11, 2  },   // SCREEN_111
  { SCREEN_111, 3 },  // SCREEN_1111
  { SCREEN_1111, 0}, // SCREEN_11111
  { SCREEN_1111, 0}, // SCREEN_11112
  { SCREEN_1111, 0}, // SCREEN_11113
  { SCREEN_111,  1},  // SCREEN_1112
  { SCREEN_11, 2  },   // SCREEN_112
  { SCREEN_112, 2 },  // SCREEN_1121
  { SCREEN_112, 2 }}; // SCREEN_1122

void setup()
{

  pinMode(PUMP1_ON, OUTPUT);
  pinMode(PUMP1_OFF, OUTPUT);
  pinMode(PUMP2, OUTPUT);
  pinMode(FLOWSENSOR, INPUT);
  digitalWrite(PUMP1_ON, HIGH); // Pump is activated in level LOW
  digitalWrite(PUMP1_OFF, HIGH); // Pump is activated in level LOW
  digitalWrite(PUMP2, HIGH); // Pump is activated in level LOW
  lcdBegin(); // This will setup our pins, and initialize the LCDbu
  updateDisplay(); 
  setContrast(40); // Good values range from 40-60
  delay(2000);
  Serial.begin(9600);
  for (int x=0; x<12; x++) // Transition between screens (optional)
  { // Shutter swipe
    setRect(0, 0, x, LCD_HEIGHT, 1, WHITE);
    setRect(11, 0, x+12, LCD_HEIGHT, 1, WHITE);
    setRect(23, 0, x+24, LCD_HEIGHT, 1, WHITE);
    setRect(35, 0, x+36, LCD_HEIGHT, 1, WHITE);
    setRect(47, 0, x+48, LCD_HEIGHT, 1, WHITE);
    setRect(59, 0, x+60, LCD_HEIGHT, 1, WHITE);
    setRect(71, 0, x+72, LCD_HEIGHT, 1, WHITE);
    updateDisplay();
    delay(25);
  }

  irrecv.enableIRIn();
  attachInterrupt(0, rpm, RISING);
}

void show_Menu(int screen, int selected, int cursor_pos, boolean state[]){

  clearDisplay(WHITE);
  if (screen == SCREEN_1) {
    setStr("1. Gerenciar", 0, 0,  selected==OPTION1?WHITE:BLACK);
    setStr("2. Iluminacao", 0, 10, selected==OPTION2?WHITE:BLACK);
    setStr("3. Consumo", 0, 20, selected==OPTION3?WHITE:BLACK);
    setStr("4. Modo Oper.", 0, 30, selected==OPTION4?WHITE:BLACK); 
    changedLightState = false;
  }
  else if (screen == SCREEN_11) {
    setStr("1. Sensores", 0, 0,  selected==OPTION1?WHITE:BLACK);
    setStr("2. Bombas", 0, 10, selected==OPTION2?WHITE:BLACK); 
  }
  else if (screen == SCREEN_111) {
    setStr("1. Verificar", 0, 0,  selected==OPTION1?WHITE:BLACK);
    setStr("2. Desativar", 0, 10, selected==OPTION2?WHITE:BLACK); 
  }
  else if (screen == SCREEN_112) {
    setStr("1. Bomba 1", 0, 0,  selected==OPTION1?WHITE:BLACK);
    setStr("1. Bomba 2", 0, 10,  selected==OPTION2?WHITE:BLACK);
  }
  else if (screen == SCREEN_1111) {
    setStr("1. Tnq Terreo", 0, 0,  selected==OPTION1?WHITE:BLACK);
    setStr("2. Tnq Laje", 0, 10, selected==OPTION2?WHITE:BLACK);
    setStr("3. Sen. Fluxo", 0, 20, selected==OPTION3?WHITE:BLACK);
    sei();
  }
  else if (screen == SCREEN_11111) {
    setStr("Nivel: ", 0, 0, BLACK);
    setStr("Volume: ", 0, 10, BLACK); 
  }
  else if (screen == SCREEN_11112) {
    setStr("Nivel: ", 0, 0, BLACK);
    setStr("Volume: ", 0, 10, BLACK); 
  }
  else if (screen == SCREEN_11113) {
    sei();
    char printed_flow[15] = "Fluxo: ";
    char current_flow[10] = "";
    NbTopsFan = 0;
    delay (1000);   //Wait 1 second
    cli();
    flow = (NbTopsFan * 60 / 5.5);
    itoa(flow, current_flow, 10);
    strcat(printed_flow, current_flow);
    Serial.print (flow, DEC);
    setStr(printed_flow, 0, 0, BLACK);
    setStr("L/h", 0, 20, BLACK);
  }
  else if (screen == SCREEN_12) {
      if (!changedLightState) {
        changedLightState = true;
        backlight = !backlight;
        analogWrite(blPin, backlight?255:0);
      }
    //if (display_clock) {
    //    char current_time[4] = "";
    //    char printed_time[9] = "";     
    //    DS1302_clock_burst_read( (uint8_t *) &rtc);
    //    itoa(bcd2bin(rtc.h24.Hour10, rtc.h24.Hour), current_time, 10);
    //    if (strlen (current_time) == 1) strcat(printed_time, "0"); 
    //    strcat(printed_time, current_time);
    //    strcat(printed_time, ":");
    //    itoa(bcd2bin(rtc.Minutes10, rtc.Minutes), current_time, 10);
    //    if (strlen (current_time) == 1) strcat(printed_time, "0");
    //    strcat(printed_time, current_time);
    //    //strcat(printed_time,"]");
    //    //strcat(lline, printed_time);
    //    display_clock = !display_clock;//}    
    //    setStr(printed_time, 0, 10, BLACK); 
  }
  else if (screen == SCREEN_1121) {
    char bon[] = "Bomba 1: ON";
    char boff[] = "Bomba 1: OFF";
    if (!state[2]) {
      if (!state[0]) {
        digitalWrite(PUMP1_ON, LOW);
        delay(400);
        digitalWrite(PUMP1_ON, HIGH);
      }
      else {
        digitalWrite(PUMP1_OFF, LOW);
        delay(400);
        digitalWrite(PUMP1_OFF, HIGH);
      }
      state[0] = !state[0];
      state[2] = true;
    }
    setStr(state[0]?bon:boff, 0, 0, BLACK);
  }
  else if (screen == SCREEN_1122) {
    char bon[] = "Bomba 2: ON";
    char boff[] = "Bomba 2: OFF";
    if (!state[2]) {
      if (!state[1])
        digitalWrite(PUMP2, LOW);
      else
        digitalWrite(PUMP2, HIGH);
      state[1] = !state[1];
      state[2] = true;
    }
    setStr(state[1]?bon:boff, 0, 0, BLACK);
  }

  setStr(lline, 0, 40, BLACK);
  updateDisplay();
}

void rpm ()     //This is the function that the interupt calls 
{ 
  NbTopsFan++;
} 

void turnonoff_Pump1() {
  boolean succeed = false;
  int nattempts = 1;
  if (!state[0]) {
      digitalWrite(PUMP1_ON, LOW);
      delay(650);
      digitalWrite(PUMP1_ON, HIGH);
      delay(1500);
      NbTopsFan = 0;
      sei();
      delay (1000);   //Wait 1 second
      cli();
      if (NbTopsFan != 0)
        state[0] = !state[0];
  }
  else {
      int previous, after;
      digitalWrite(PUMP1_OFF, LOW);
      delay(650);
      digitalWrite(PUMP1_OFF, HIGH);
      delay(1500);
      previous = NbTopsFan;
      after = NbTopsFan;
      sei();
      delay (1000);   //Wait 1 second
      cli();
      if ((previous - after)>=0)
        state[0] = !state[0];
  }
  sei();
}

void turnonoff_Pump2() {
      if (!state[1])
        digitalWrite(PUMP2, LOW);
      else
        digitalWrite(PUMP2, HIGH);
      state[1] = !state[1];
    }

void loop() {
  show_Menu(screen, option, cursor_pos, state);
  // Automatic activation mechanism starts here
  if (irrecv.decode(&command)) {
    switch (command.value) {
      case(CR_TEMP_INCREASE) :  
      option = max (0, option - 1);
      break;
      case(CR_TEMP_DECREASE) :  
      option = min (encoding_menu[screen][1]-1, option + 1);
      break;
      case(CR_CENTRAL_FAN)   :  
      if (screen == SCREEN_1) {
        if (option == OPTION1)
          screen = SCREEN_11;
        else if (option == OPTION2)
          screen = SCREEN_12;
        else if (option == OPTION3)
          screen = SCREEN_13;
        else if (option == OPTION4)
          screen = SCREEN_14;                        
      } 
      else if (screen == SCREEN_11) {
        if (option == OPTION1)
          screen = SCREEN_111;
        else if (option == OPTION2)
          screen = SCREEN_112; 
      } 
      else if (screen == SCREEN_111) {
        if (option == OPTION1)
          screen = SCREEN_1111;
        else if (option == OPTION2)
          screen = SCREEN_1112;
      } 
      else if (screen == SCREEN_1111) {
        if (option == OPTION1)
          screen = SCREEN_11111;
        else if (option == OPTION2)
          screen = SCREEN_11112;
        else if (option == OPTION3)
          screen = SCREEN_11113;                                     
      } 
      else if (screen == SCREEN_112) {
        if (option == OPTION1)
          screen = SCREEN_1121;
        else if (option == OPTION2)
          screen = SCREEN_1122;
      }
      option = OPTION1;
      break;
      case(CR_POWER_ON_OFF)   :  
      option = OPTION1;
      screen = encoding_menu[screen][0];
      display_clock = true;
      state[2] = false;
      break;
      case (CR_LEFT_TIMMER) : 
      turnonoff_Pump1(); 
      break;
      case (CR_RIGHT_HUMIDITY):
      turnonoff_Pump2(); 
      default: 
      break;
    }
    irrecv.resume(); // Receive the next value
  }
  if (screen != previous_screen) {
    Serial.print("Screen=");
    Serial.println(screen);
    previous_screen = screen;
  }
}
