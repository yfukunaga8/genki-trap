#include <WioLTEforArduino.h>
#include <stdio.h>

// LTE
#define APN               "plus.4g"
#define USERNAME          "plus"
#define PASSWORD          "4g"

// IFTTT
#define WEBHOOK_EVENTNAME "hoge"
#define WEBHOOK_KEY       "huga"
#define WEBHOOK_URL       "https://maker.ifttt.com/trigger/"WEBHOOK_EVENTNAME"/with/key/"WEBHOOK_KEY

// I/O
#define MAGNETIC_SWITCH_PIN (WIOLTE_D38)
#define READ_TIMES (10)

// DELAY TIME (ms)
#define READ_MAGNETIC_SWITCH_INTERVAL (1000)
#define LTE_INTERVAL (5000)
#define TRAPPED_INTERVAL (6) // Convert to hour later

// TRAP STATE
#define SET_TRAP    (0)
#define HEALTH_CHEK (1)
#define TRAPPED     (2)

// MESSAGE
#define TRAP_NUMBER "01"
#define MESSAGE_BUFFER (64)
#define MESSAGE_FOR_SET_TRAP     "{\"value1\":\"You set Trap No." TRAP_NUMBER "\"}"
#define MESSAGE_FOR_HEALTH_CHECK "{\"value1\":\"Health check OK. at Trap No." TRAP_NUMBER "\"}"
#define MESSAGE_FOR_TRAPPED      "{\"value1\":\"You got target! Trap No." TRAP_NUMBER "\"}"

WioLTE Wio;

void setup() {
  pinMode(MAGNETIC_SWITCH_PIN, INPUT);
}

void enable_lte(){
  Wio.Init();
  Wio.PowerSupplyLTE(true);
  delay(LTE_INTERVAL);
  if !Wio.TurnOnOrReset() return;
  if !Wio.Activate(APN, USERNAME, PASSWORD) return;
  delay(LTE_INTERVAL);
}

void disable_lte(){
  delay(LTE_INTERVAL);
  if !Wio.TurnOff() return;
  delay(LTE_INTERVAL);
}

void send_message(int state){
  int response_code = 0;
  char data[MESSAGE_BUFFER];

  enable_lte();
  switch(state){
    case 0:
      sprintf(data, MESSAGE_FOR_SET_TRAP);
    case 1:
      sprintf(data, MESSAGE_FOR_HEALTH_CHECK);
    case 2:
      sprintf(data, MESSAGE_FOR_TRAPPED);
    default:
  }
  Wio.HttpPost(WEBHOOK_URL, data, &response_code);
  disable_lte();
}

int read_magnetic_switch_state(int *trap_presence){
  for(int i = 0; i < READ_TIMES; i++;){
    // attached:1(HIGH), unattached:0(LOW)
    if (trap_presence == digitalRead(MAGNETIC_SWITCH_PIN) return 0;
    delay(READ_MAGNETIC_SWITCH_INTERVAL);
  }
  return 1;
}

void loop() {
  static const unsigned long SECOND = 1000;
  static const unsigned long HOUR = 3600*SECOND;
  static unsigned long interval = READ_MAGNETIC_SWITCH_INTERVAL;
  static int trap_presence = 0;

  if (trap_presence){
    // Trapped
    if read_magnetic_switch_state(trap_presence){
      send_message(TRAPPED);
      trap_presence = 0;
      interval = 0;
    }
    // Health check
    else send_message(HEALTH_CHEK);
  }
  else{
    // Set Trap
    if read_magnetic_switch_state(trap_presence){
      send_message(SET_TRAP);
      trap_presence = 1;
      interval = TRAPPED_INTERVAL * HOUR;
    }
  }
  delay(interval);
}
