// Sonoff R3 button state
extern int SonoffButton[];
extern int SonoffButBehavior[];

#define SONOFF_BUTTON_1 0
#define SONOFF_BUTTON_2 9
#define SONOFF_BUTTON_3 10

#define BUTTON_DONOTHING 0
#define BUTTON_RELAY 1
#define BUTTON_UDP 2

void initSonoffButtonData();
void sonoffLoop();
void readSonoffButtons(int *dest);
