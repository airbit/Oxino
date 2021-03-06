#include "Oxino.h"

#define PULLUP false        	//To keep things simple, we use the Arduino's internal pullup resistor.
#define INVERT false        	//Since the pullup resistor will keep the pin high unless the
							//switch is closed, this is negative logic, i.e. a high state
							//means the button is NOT pressed. (Assuming a normally open switch.)
#define DEBOUNCE_MS 20     	//A debounce time of 20 milliseconds usually works well for tactile button switches.

#define LONG_PRESS 1000    	//We define a "long press" to be 1000 milliseconds.
#define BLINK_INTERVAL 100 	//In the BLINK state, switch the LED every 100 milliseconds.

#ifdef __CC3200R1M1RGC__
const uint8_t BUTTON_PIN = PUSH2;
const uint8_t LED_PIN = RED_LED;
#else
const uint8_t BUTTON_PIN = 2;
const uint8_t LED_PIN = 13;
#endif

Button myBtn(BUTTON_PIN, INVERT, PULLUP);    //Declare the button

//The list of possible states for the state machine. This state machine has a fixed
//sequence of states, i.e. ONOFF --> TO_BLINK --> BLINK --> TO_ONOFF --> ONOFF
//Note that while the user perceives two "modes", i.e. ON/OFF mode and rapid blink mode,
//two extra states are needed in the state machine to transition between these modes.
enum {
	ONOFF, TO_BLINK, BLINK, TO_ONOFF
};
uint8_t STATE = ONOFF;                   //The current state machine state
boolean ledState;                //The current LED status
unsigned long ms;                //The current time from millis()
unsigned long msLast;            //The last time the LED was switched

//Reverse the current LED state. If it's on, turn it off. If it's off, turn it on.
void switchLED() {
    msLast = ms;                 //record the last switch time
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
}

//Switch the LED on and off every BLINK_INETERVAL milliseconds.
void fastBlink() {
    if (ms - msLast >= BLINK_INTERVAL)
        switchLED();
}

void setup(void) {
    myBtn.debounceTime = DEBOUNCE_MS;

	pinMode(LED_PIN, OUTPUT);    //Set the LED pin as an output
}

void loop(void) {
	ms = millis();               //record the current time
	myBtn.read();                //Read the button

	switch (STATE) {

	//This state watches for short and long presses, switches the LED for
	//short presses, and moves to the TO_BLINK state for long presses.
	case ONOFF:
		if (myBtn.wasReleased())
			switchLED();
		else if (myBtn.pressedFor(LONG_PRESS))
			STATE = TO_BLINK;
		break;

		//This is a transition state where we start the fast blink as feedback to the user,
		//but we also need to wait for the user to release the button, i.e. end the
		//long press, before moving to the BLINK state.
	case TO_BLINK:
		if (myBtn.wasReleased())
			STATE = BLINK;
		else
			fastBlink();
		break;

		//The fast-blink state. Watch for another long press which will cause us to
		//turn the LED off (as feedback to the user) and move to the TO_ONOFF state.
	case BLINK:
		if (myBtn.pressedFor(LONG_PRESS)) {
			STATE = TO_ONOFF;
			digitalWrite(LED_PIN, LOW);
			ledState = false;
		} else
			fastBlink();
		break;

		//This is a transition state where we just wait for the user to release the button
		//before moving back to the ONOFF state.
	case TO_ONOFF:
		if (myBtn.wasReleased())
			STATE = ONOFF;
		break;
	}
}
