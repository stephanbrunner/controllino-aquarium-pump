#include <Arduino.h>
#include <Controllino.h>

// pinout
uint8_t PIN_SWITCH_OFF_0 = CONTROLLINO_AI0;
uint8_t PIN_SWITCH_ON_0 = CONTROLLINO_AI1;
uint8_t PIN_SWITCH_OFF_1 = CONTROLLINO_AI2;
uint8_t PIN_SWITCH_ON_1 = CONTROLLINO_AI3;
uint8_t PIN_SWITCH_OFF_2 = CONTROLLINO_AI4;
uint8_t PIN_SWITCH_ON_2 = CONTROLLINO_AI5;
uint8_t PIN_SWITCH_OFF_3 = CONTROLLINO_AI6;
uint8_t PIN_SWITCH_ON_3 = CONTROLLINO_AI7;
uint8_t PIN_SWITCH_OFF_4 = CONTROLLINO_AI8;
uint8_t PIN_SWITCH_ON_4 = CONTROLLINO_AI9;
uint8_t PIN_MODE_STRICT = CONTROLLINO_AI10;
uint8_t PIN_MODE_EXPO = CONTROLLINO_AI11;
uint8_t PIN_PUMP_MONITOR_0 = CONTROLLINO_DO0;
uint8_t PIN_PUMP_MONITOR_1 = CONTROLLINO_DO1;
uint8_t PIN_PUMP_MONITOR_2 = CONTROLLINO_DO2;
uint8_t PIN_PUMP_MONITOR_3 = CONTROLLINO_DO3;
uint8_t PIN_PUMP_MONITOR_4 = CONTROLLINO_DO4;
uint8_t PIN_PUMP_0 = CONTROLLINO_R0;
uint8_t PIN_PUMP_1 = CONTROLLINO_R1;
uint8_t PIN_PUMP_2 = CONTROLLINO_R2;
uint8_t PIN_PUMP_3 = CONTROLLINO_R3;
uint8_t PIN_PUMP_4 = CONTROLLINO_R4;

// general settings
const uint8_t NUMBER_OF_PUMPS = 5;

// random settings
const uint32_t PUMP_MIN_DURATION = 5; // in seconds
const uint32_t PUMP_MAX_DURATION = 40; // in seconds

// strict settings
const uint32_t PUMP_STRICT_LOOP_DURATION = 45; // in seconds
const uint32_t PUMP_STRICT_TIMING_0[] = {5, 10, 15, 20, 25, 30, 35, 40}; // in seconds
const uint32_t PUMP_STRICT_TIMING_1[] = {5, 10, 15, 20, 25, 30, 35, 40}; // in seconds
const uint32_t PUMP_STRICT_TIMING_2[] = {5, 10, 15, 20, 25, 30, 35, 40}; // in seconds
const uint32_t PUMP_STRICT_TIMING_3[] = {5, 10, 15, 20, 25, 30, 35, 40}; // in seconds
const uint32_t PUMP_STRICT_TIMING_4[] = {5, 10, 15, 20, 25, 30, 35, 40}; // in seconds

// expo settings
const uint32_t PUMP_EXPO_LOOP_DURATION = 600; // in seconds
const uint32_t PUMP_EXPO_TIMING_0[] = {0, 10, 15, 20, 25, 30, 35, 40, 150}; // in seconds
const uint32_t PUMP_EXPO_TIMING_1[] = {0, 10, 15, 20, 25, 30, 35, 40, 55, 60, 75, 80, 100, 130, 150}; // in seconds
const uint32_t PUMP_EXPO_TIMING_2[] = {0, 10, 15, 20, 25, 30, 35, 40, 60, 75, 100, 130, 150}; // in seconds
const uint32_t PUMP_EXPO_TIMING_3[] = {0, 10, 15, 20, 25, 30, 35, 40, 150}; // in seconds
const uint32_t PUMP_EXPO_TIMING_4[] = {0, 10, 15, 20, 25, 30, 35, 40, 45, 90, 100, 130, 150}; // in seconds

// expo settings
const uint32_t RANDOM_DURATION = 30; // in seconds
const uint32_t OFF_DURATION = 10; // in seconds

// helpers
uint8_t PINS_SWITCH_OFF[NUMBER_OF_PUMPS] = {
    PIN_SWITCH_OFF_0,
    PIN_SWITCH_OFF_1,
    PIN_SWITCH_OFF_2,
    PIN_SWITCH_OFF_3,
    PIN_SWITCH_OFF_4
};
uint8_t PINS_SWITCH_ON[NUMBER_OF_PUMPS] = {
    PIN_SWITCH_ON_0,
    PIN_SWITCH_ON_1,
    PIN_SWITCH_ON_2,
    PIN_SWITCH_ON_3,
    PIN_SWITCH_ON_4
};
uint8_t PINS_PUMP[NUMBER_OF_PUMPS] = {
    PIN_PUMP_0,
    PIN_PUMP_1,
    PIN_PUMP_2,
    PIN_PUMP_3,
    PIN_PUMP_4
};
uint8_t PINS_PUMP_MONITOR[NUMBER_OF_PUMPS] = {
    PIN_PUMP_MONITOR_0,
    PIN_PUMP_MONITOR_1,
    PIN_PUMP_MONITOR_2,
    PIN_PUMP_MONITOR_3,
    PIN_PUMP_MONITOR_4
};

// enums
enum State{
    STATE_MANUAL, 
    STATE_STRICT,
    STATE_EXPO
};

// vars
uint8_t state = STATE_MANUAL;
uint32_t next_random_switch[NUMBER_OF_PUMPS] = {0, 0, 0, 0, 0};
uint16_t strict_timestamp_index[NUMBER_OF_PUMPS] = {0, 0, 0, 0, 0};
uint32_t strict_timestamp_start = 0;
uint32_t expo_timestamp_start = 0;

void toggle_pump(uint8_t pump) {
    uint8_t state = digitalRead(PINS_PUMP_MONITOR[pump]);
    digitalWrite(PINS_PUMP_MONITOR[pump], !state);
    digitalWrite(PINS_PUMP[pump], !state);
}

void set_pump(uint8_t pump, uint8_t state) {
    digitalWrite(PINS_PUMP[pump], state);
    digitalWrite(PINS_PUMP_MONITOR[pump], state);
}

void set_random_timestamp(uint8_t pump) {
    next_random_switch[pump] = millis() + random(PUMP_MIN_DURATION * 1000, PUMP_MAX_DURATION * 1000);
}

State get_state() {
    State s = STATE_MANUAL;
    if (digitalRead(PIN_MODE_STRICT) == HIGH) {
        s = STATE_STRICT;
    } else if (digitalRead(PIN_MODE_EXPO) == HIGH) {
        s = STATE_EXPO;
    }
    return s;
}

void enter_state_manual() {
    Serial.println("enter_state_manual");
    state = STATE_MANUAL;

    // init pump states
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        set_pump(i, LOW);
    }
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        set_random_timestamp(i);
    }
}

void while_state_manual() {
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        if (digitalRead(PINS_SWITCH_OFF[i]) == HIGH) {
            set_pump(i, LOW);
        } else if (digitalRead(PINS_SWITCH_ON[i]) == HIGH) {
            set_pump(i, HIGH);
        } else {
            if (millis() > next_random_switch[i]) {
                set_random_timestamp(i);
                toggle_pump(i);
            }
        }
    }
}

void enter_state_strict() {
    Serial.println("enter_state_strict");
    state = STATE_STRICT;

    // init vars
    strict_timestamp_start = millis();
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        strict_timestamp_index[i] = 0;
    }

    // init all pumps
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        set_pump(i, LOW);
    }
}

void while_state_strict() {
    if (strict_timestamp_index[0] < sizeof(PUMP_STRICT_TIMING_0) / sizeof(uint32_t) && PUMP_STRICT_TIMING_0[strict_timestamp_index[0]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(0);
        strict_timestamp_index[0]++;
    }
    if (strict_timestamp_index[1] < sizeof(PUMP_STRICT_TIMING_1) / sizeof(uint32_t) && PUMP_STRICT_TIMING_1[strict_timestamp_index[1]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(1);
        strict_timestamp_index[1]++;
    }
    if (strict_timestamp_index[2] < sizeof(PUMP_STRICT_TIMING_2) / sizeof(uint32_t) && PUMP_STRICT_TIMING_2[strict_timestamp_index[2]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(2);
        strict_timestamp_index[2]++;
    }
    if (strict_timestamp_index[3] < sizeof(PUMP_STRICT_TIMING_3) / sizeof(uint32_t) && PUMP_STRICT_TIMING_3[strict_timestamp_index[3]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(3);
        strict_timestamp_index[3]++;
    }
    if (strict_timestamp_index[4] < sizeof(PUMP_STRICT_TIMING_4) / sizeof(uint32_t) && PUMP_STRICT_TIMING_4[strict_timestamp_index[4]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(4);
        strict_timestamp_index[4]++;
    }

    if (PUMP_STRICT_LOOP_DURATION * 1000 < (millis() - strict_timestamp_start)) {
        // init vars and pump states
        strict_timestamp_start = millis();
        for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
            set_pump(i, LOW);
            strict_timestamp_index[i] = 0;
        }
    }
}

void enter_state_expo() {
    Serial.println("enter_state_expo");
    state = STATE_EXPO;

    // init vars
    strict_timestamp_start = millis();
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        strict_timestamp_index[i] = 0;
    }

    // init all pumps
    for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
        set_pump(i, LOW);
    }
}

void while_state_expo() {
    if (strict_timestamp_index[0] < sizeof(PUMP_EXPO_TIMING_0) / sizeof(uint32_t) && PUMP_EXPO_TIMING_0[strict_timestamp_index[0]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(0);
        strict_timestamp_index[0]++;
    }
    if (strict_timestamp_index[1] < sizeof(PUMP_EXPO_TIMING_1) / sizeof(uint32_t) && PUMP_EXPO_TIMING_1[strict_timestamp_index[1]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(1);
        strict_timestamp_index[1]++;
    }
    if (strict_timestamp_index[2] < sizeof(PUMP_EXPO_TIMING_2) / sizeof(uint32_t) && PUMP_EXPO_TIMING_2[strict_timestamp_index[2]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(2);
        strict_timestamp_index[2]++;
    }
    if (strict_timestamp_index[3] < sizeof(PUMP_EXPO_TIMING_3) / sizeof(uint32_t) && PUMP_EXPO_TIMING_3[strict_timestamp_index[3]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(3);
        strict_timestamp_index[3]++;
    }
    if (strict_timestamp_index[4] < sizeof(PUMP_EXPO_TIMING_4) / sizeof(uint32_t) && PUMP_EXPO_TIMING_4[strict_timestamp_index[4]] * 1000 < (millis() - strict_timestamp_start)) {
        toggle_pump(4);
        strict_timestamp_index[4]++;
    }

    if (PUMP_EXPO_LOOP_DURATION * 1000 < (millis() - strict_timestamp_start)) {
        // init vars and pump states
        strict_timestamp_start = millis();
        for (uint8_t i = 0; i < NUMBER_OF_PUMPS; i++) {
            set_pump(i, LOW);
            strict_timestamp_index[i] = 0;
        }
    }
}

void loop() {
    // execute enter routines if state changed
    uint8_t current_state = get_state();
    if (current_state != state) {
        if (current_state == STATE_MANUAL) {
            enter_state_manual();
        } else if (current_state == STATE_STRICT) {
            enter_state_strict();
        } else if (current_state == STATE_EXPO) {
            enter_state_expo();
        }
    }

    // execute while routines
    if (state == STATE_MANUAL) {
        while_state_manual();
    } else if (state == STATE_STRICT) {
        while_state_strict();
    } else if (state == STATE_EXPO) {
        while_state_expo();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("controllino is up...");
}