#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Arduino.h>
#include <NonBlockingRtttl.h>


#define BUZZER_PIN 2
#define TOUCH_PIN 1

#define TOUCH_THRESHOLD 25000
#define DEBOUNCE_DELAY 150//ms
#define MAX_TOUCH_INTERVAL 500//ms

typedef enum {
    NO_INPUT,
    THREE_TOUCHES,
    FIVE_TOUCHES
} UserAction_t;

typedef enum {
    NONE,
    SAMPLE_BEGIN,
    SAMPLE_END,
    SERVER_MODE_ON,
    SERVER_MODE_OFF   
} SystemTransitions_t;


class UserInterface {
public:
    UserInterface();
    ~UserInterface();

    void beginUI();
    UserAction_t updateUI();
    void setUserInputEnabled(bool enabled);

    void setSystemTransition(SystemTransitions_t transition);


private:
    static void IRAM_ATTR gotTouchEvent();

    static volatile bool touchDetected; // Flag compartido con la ISR

    uint8_t touchCount = 0;
    unsigned long lastTouchTime = 0; // Para medir intervalo entre toques

    bool threeTouchesDetected = false;
    bool fiveTouchesDetected = false;

    bool userInputEnabled = true;
    const int threshold = TOUCH_THRESHOLD;

    const char * touch_tone = "touch_tone:d=4,o=6,b=250:16b6";
    const char * sample_begin_tone = "sample_begin_tone:d=4,o=5,b=140:8g#6,8a6,4a#6";
    const char * sample_end_tone = "sample_end_tone:d=4,o=5,b=140:8a#6,8a6,4g#6";
    const char * sample_error_tone = "sample_error_tone:d=4,o=5,b=140:";
    const char * server_mode_on_tone = "server_on_tone:d=4,o=6,b=140:16d#5,16e5,16f5,16f#5,16g5";
    const char * server_mode_off_tone = "server_off_tone:d=4,o=6,b=140:16g5,16f#5,16f5,16e5,16d#5";

};

#endif