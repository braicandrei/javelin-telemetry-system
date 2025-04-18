#include "UserInterface.h"
#define DEBUUG_UI 1

volatile bool UserInterface::touchDetected = false;

/**
 * @brief UserInterface constructor
 * 
 * The constructor initializes the user interface object
 */
UserInterface::UserInterface() {}
/**
 * @brief UserInterface destructor
 * 
 * The destructor cleans up the user interface object
 */
UserInterface::~UserInterface() {}

/**
 * @brief Interrupt handler for touch events
 * 
 * This function is called when a touch event occurs, setting the touchDetected flag to true.
 */
void IRAM_ATTR UserInterface::gotTouchEvent() {
    static unsigned long lastDebounceTime = 0;
    unsigned long currentTime = millis();

    if ((currentTime - lastDebounceTime) > DEBOUNCE_DELAY &&
        !touchInterruptGetLastStatus(TOUCH_PIN)) {

        lastDebounceTime = currentTime;
        touchDetected = true;
    }
}

/**
 * @brief Initialize the user interface
 * 
 * This function initializes the user interface by setting up the touch interrupt and initializing the RTTTL player.
 */
void UserInterface::beginUI() {
    #if (DEBUUG_UI) 
      Serial.println("Initializing User Interface...");
    #endif
    touchAttachInterrupt(TOUCH_PIN, gotTouchEvent, threshold);
    pinMode(BUZZER_PIN, OUTPUT);
}

/**
 * @brief Update the user interface
 * 
 * This function updates the user interface by checking for touch events and updating the RTTTL player.
 * 
 * @return UserAction_t The user action detected by the user interface
 */
UserAction_t UserInterface::updateUI() {
    rtttl::play(); //update the RTTTL player
    if (!userInputEnabled) return NO_INPUT;

    unsigned long currentTime = millis();
    const unsigned long maxTouchInterval = MAX_TOUCH_INTERVAL;
    UserAction_t userAction = NO_INPUT;
    
    if (touchDetected) {
        touchDetected = false;
        touchCount++;
        lastTouchTime = currentTime;
        if (!rtttl::isPlaying()) {
          rtttl::begin(BUZZER_PIN, touch_tone);
        }
        #if (DEBUUG_UI) 
          Serial.println("Toque detectado!");
        #endif
    }
    if ((currentTime - lastTouchTime) > maxTouchInterval && touchCount > 0) {
        if (touchCount == 3) {
            userAction = THREE_TOUCHES;
            #if (DEBUUG_UI) 
              Serial.println("¡Tres toques detectados!");
            #endif
        } else if (touchCount == 5) {
            userAction = FIVE_TOUCHES;
            #if (DEBUUG_UI) 
              Serial.println("¡Cinco toques detectados!");
            #endif
        }

        touchCount = 0;
    }
    return userAction;
}

/**
 * @brief Set the system transition
 * 
 * This function sets the system transition based on the provided transition type.
 * 
 * @param transition The system transition type
 */
void UserInterface::setSystemTransition(SystemTransitions_t transition) {
    switch (transition) {
        case SAMPLE_BEGIN:
            rtttl::begin(BUZZER_PIN, sample_begin_tone);
            break;
        case SAMPLE_END:
            rtttl::begin(BUZZER_PIN, sample_end_tone);
            break;
        case SERVER_MODE_ON:
            rtttl::begin(BUZZER_PIN, server_mode_on_tone);
            break;
        case SERVER_MODE_OFF:
            rtttl::begin(BUZZER_PIN, server_mode_off_tone);
            break;
        default:
            break;
    }
}

/**
 * @brief Set the user input enabled flag
 * 
 * This function sets the user input enabled flag to the provided value.
 * 
 * @param enabled The user input enabled flag value
 */
void UserInterface::setUserInputEnabled(bool enabled) {
    userInputEnabled = enabled;
}
