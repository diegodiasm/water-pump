

void power_on(int pumpName);
void power_off(int pumpName);
int current_flow(int pumpName);
int current_level(int tankName);
int currentTime();
int system_status();
boolean flood (int tankName);
char *decode_status(int system_status);
void show_welcome_msg();

// Number of PUMPS and TANKS in this implementation
const int UNDERGROUND_PUMP = 0
const int GROUND_PUMP = 1
const int FULL = 1;
const int TOTAL_PUMP = 2;
const int TOTAL_TANK = 2;


boolean active = false;
int sys_status;
int pump_relay_pin [TOTAL_PUMP];
int pump_flow_pin  [TOTAL_TANK];
int pump_overflow_pin [TOTAL_PUMP];
int activation_time;
int deactivation_time;
int input_keypad;
int interval_delay = 10000;
boolean toptank_full = false;
boolean basetank_full = false;


char *decode_status(){
/*
 * 0 : Working properly ("Ativado")
 * 1 : Inoperant by choice ("Desativado ")
 * 2 : Flooded ("Transbordamento")
 * 3 : Too Low ("Baixa Vazao")
 * 4 : Full capacity ("Tanque Cheio")
 * 5 : Sensor Failure ("Falha de Sensor")
 */
 return "";
}

void show_welcome_msg(){
   printf("Controle de Agua");
   printf(decode_status());
}

void automatic_activation_system(){
   //store_sensor_data();
   while (!toptank_full && currentTime()<deactivation_time) {
      if (!basetank_full) 
         power_on (UNDERGROUND_PUMP) 
      else power_off (UNDERGROUND_PUMP);
      delay (interval_delay);
      if (basetank_safe)
         power_on (GROUND_PUMP)
      else power_off (GROUND_PUMP)  
      delay (10000);
  }
  //store_sensor_data();   
}
 
void setup() {
  // put your setup code here, to run once:
  check_up(); // Set the system_status after checking sensors;
  show_welcome_msg();
}

void loop() {
   if ((activation_time >= getTime()) && 
       (getTime() <= deactivation_time) && !active)
      automatic_activation_system();  
   }
   input_keypad = read_keypad();
   switch (input_keypad){
      case FLOW_SENSOR: print_sensor(FLOW_SENSOR); break;
      case TOP_SENSOR: print_sensor(TOP_SENSOR); break;
      case GROUND_SENSOR: print_sensor(GROUND_SENSOR); break;
      default:
   }  
}
