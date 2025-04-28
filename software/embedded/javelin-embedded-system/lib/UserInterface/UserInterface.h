#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Arduino.h>
#include <NonBlockingRtttl.h>


#define BUZZER_PIN 2
#define TOUCH_PIN 1

#define TOUCH_THRESHOLD 10000
#define MAX_TOUCH_INTERVAL 500//ms

#define ISR_DEBOUNCE_DELAY     150   // ms: tu debounce original en la ISR
#define VALIDATE_DELAY         50    // ms: tiempo para validar en loop

enum DebounceState { IDLE, VALIDATING };

typedef enum {
    NO_INPUT,
    INPUT_1,
    INPUT_2,
    INPUT_3
} UserAction_t;

typedef enum {
    INPUT_1_COUNT = 2,
    INPUT_2_COUNT = 4,
    INPUT_3_COUNT = 6
} InputCount_t;

typedef enum {
    NONE,
    POWER_ON,
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
    DebounceState debounceState = IDLE;
    unsigned long validationTime = 0;

    uint8_t touchCount = 0;
    unsigned long lastTouchTime = 0; // Para medir intervalo entre toques

    bool threeTouchesDetected = false;
    bool fiveTouchesDetected = false;

    bool userInputEnabled = true;
    const int threshold = TOUCH_THRESHOLD;

    const char * touch_tone = "touch_tone:d=4,o=6,b=250:16b6";
    const char * power_on_tone = "power_on_tone:d=4,o=6,b=225:4d#4,4e4,4f4";
    const char * sample_begin_tone = "sample_begin_tone:d=4,o=5,b=140:8g#6,8a6,4a#6";
    const char * sample_end_tone = "sample_end_tone:d=4,o=5,b=140:8a#6,8a6,4g#6";
    const char * sample_error_tone = "sample_error_tone:d=4,o=5,b=140:";
    const char * server_mode_on_tone = "server_on_tone:d=4,o=6,b=140:16d#5,16e5,16f5,16f#5,16g5";
    const char * server_mode_off_tone = "server_off_tone:d=4,o=6,b=140:16g5,16f#5,16f5,16e5,16d#5";

};

#endif