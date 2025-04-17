#include "UserInterface.h"
#define DEBUUG_UI 1

volatile bool UserInterface::touchDetected = false;

UserInterface::UserInterface() {}
UserInterface::~UserInterface() {}

void IRAM_ATTR UserInterface::gotTouchEvent() {
    static unsigned long lastDebounceTime = 0;
    unsigned long currentTime = millis();

    if ((currentTime - lastDebounceTime) > DEBOUNCE_DELAY &&
        !touchInterruptGetLastStatus(TOUCH_PIN)) {

        lastDebounceTime = currentTime;
        touchDetected = true;
    }
}

void UserInterface::beginUI() {
    #if (DEBUUG_UI) 
      Serial.println("Initializing User Interface...");
    #endif
    touchAttachInterrupt(TOUCH_PIN, gotTouchEvent, threshold);
    pinMode(BUZZER_PIN, OUTPUT);
}

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

void UserInterface::setSystemTransition(SystemTransitions_t transition) {
    switch (transition) {
        case SAMPLE_BEGIN:
            rtttl::begin(BUZZER_PIN, sample_begin_tone);
            break;
        case SAMPLE_END:
            rtttl::begin(BUZZER_PIN, sample_end_tone);
            break;
        case SERVER_MODE_ON:
            rtttl::begin(BUZZER_PIN, sample_begin_tone);
            break;
        case SERVER_MODE_OFF:
            rtttl::begin(BUZZER_PIN, sample_end_tone);
            break;
        default:
            break;
    }
}

void UserInterface::setUserInputEnabled(bool enabled) {
    userInputEnabled = enabled;
}

bool UserInterface::isThreeTouchesDetected() const {
    return threeTouchesDetected;
}

bool UserInterface::isFiveTouchesDetected() const {
    return fiveTouchesDetected;
}

void UserInterface::resetTouchFlags() {
    threeTouchesDetected = false;
    fiveTouchesDetected = false;
}
