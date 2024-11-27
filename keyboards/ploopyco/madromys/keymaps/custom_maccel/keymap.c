/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
 * Copyright 2020 Ploopy Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

#ifdef MACCEL_ENABLE
    #include "maccel/maccel.h"
#endif

/*
enum maccel_keycodes {
    MA_TOGGLE = QK_USER,    // toggle mouse acceleration
    MA_TAKEOFF,   // mouse acceleration curve takeoff (initial acceleration) step key
    MA_GROWTH_RATE,         // mouse acceleration curve growth rate step key
    MA_OFFSET,              // mouse acceleration curve offset step key
    MA_LIMIT,               // mouse acceleration curve limit step key
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_maccel(keycode, record, MA_TOGGLE, MA_TAKEOFF, MA_GROWTH_RATE, MA_OFFSET, MA_LIMIT)) {
        return false;
    }
    // insert your own macros here
    return true;
}
*/

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // ...
#ifdef MACCEL_ENABLE
    return pointing_device_task_maccel(mouse_report);
#endif
}

// Tap Dance keycodes
enum td_keycodes {
    MSE_BTN4_DPI,             // Backward mouse button on tap, cycle dpi options on double tap
    MSE_BTN5_ACC  // Forward mouse button on tap, drag scroll on held, enable/disable acceleration on double tap
};

// Define a state for when we're holding down button 4
// this enters precison mode but also allows us to switch to another layer
//bool btn4_held = false;
//bool precision_mode = false;

// Define a type containing as many tapdance states as you need
typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_SINGLE_TAP,
    TD_DOUBLE_HOLD,
    TD_DOUBLE_TAP
} td_state_t;

// Create a global instance of the tapdance state type
static td_state_t td_state;

// Declare your tapdance functions:

// Function to determine the current tapdance state
td_state_t cur_dance(tap_dance_state_t *state);

// `finished` and `reset` functions for each tapdance keycode
void mseBtn4_finished(tap_dance_state_t *state, void *user_data);
void mseBtn4_reset(tap_dance_state_t *state, void *user_data);
void mseBtn5_finished(tap_dance_state_t *state, void *user_data);
void mseBtn5_reset(tap_dance_state_t *state, void *user_data);

// Define two layers
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        TD(MSE_BTN4_DPI),
        TD(MSE_BTN5_ACC),
        DRAG_SCROLL,
        KC_BTN2,
        KC_BTN1,
        KC_BTN3
    ),
};

// Determine the tapdance state to return
td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        // Interrupted means some other button was pressed in the tapping term
        if (state->interrupted || !state->pressed) {
            //xprintf("TD_SINGLE_TAP\n");
            return TD_SINGLE_TAP;
        } else {
            //xprintf("TD_SINGLE_HOLD\n");
            return TD_SINGLE_HOLD;
        }
    }

    if (state->count == 2) {
        if (state->interrupted) {
            //xprintf("TD_DOUBLE_SINGLE_TAP\n");
            return TD_DOUBLE_SINGLE_TAP;
        } else if (state->pressed) {
            //xprintf("TD_DOUBLE_HOLD\n");
            return TD_DOUBLE_HOLD;
        } else {
            //xprintf("TD_DOUBLE_TAP\n");
            return TD_DOUBLE_TAP;
        }

    } else {
        //xprintf("TD_UNKNOWN\n");
        return TD_UNKNOWN; // Any number higher than the maximum state value you return above
    }
}

void mseBtn4_finished(tap_dance_state_t *state, void *user_data) {
    td_state = cur_dance(state);
    switch (td_state) {
        case TD_SINGLE_TAP:
        case TD_SINGLE_HOLD:
            //xprintf("finished button 4 sending tap/hold code\n");
            register_code16(KC_BTN4);
        break;
        default:
        break;
    }
}

void mseBtn4_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_state) {
        case TD_SINGLE_TAP:
        case TD_SINGLE_HOLD:
            //xprintf("reset button 4 sending tap/hold code\n");
            unregister_code16(KC_BTN4);
        break;
        case TD_DOUBLE_TAP:
            //xprintf("reset button 4 sending double tap code\n");
            keyboard_config.dpi_config = (keyboard_config.dpi_config + 1) % dpi_array_size;
            eeconfig_update_kb(keyboard_config.raw);
            pointing_device_set_cpi(dpi_array[keyboard_config.dpi_config]);
        break;
        default:
        break;
    }
}

void mseBtn5_finished(tap_dance_state_t *state, void *user_data) {
    td_state = cur_dance(state);
    switch (td_state) {
        case TD_SINGLE_TAP:
        case TD_SINGLE_HOLD:
            //xprintf("Hold for button 5 finished\n");
            register_code16(KC_BTN5);
            //is_drag_scroll = true;
        break;
        default:
        break;
    }
}

void mseBtn5_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_state) {
        case TD_SINGLE_TAP:
            //xprintf("reset button 5 sending tap code\n");
            //tap_code16(KC_BTN5);
        //break;
        case TD_SINGLE_HOLD:
            //xprintf("Hold for button 5 reset\n");
            unregister_code16(KC_BTN5);
            //is_drag_scroll = false;
        break;
        case TD_DOUBLE_TAP:
            //xprintf("reset button 5 sending double tap code\n");
            maccel_toggle_enabled();
        break;
        default:
        break;
    }
}

// Define `ACTION_TAP_DANCE_FN_ADVANCED()` for each tapdance keycode, passing in `finished` and `reset` functions
tap_dance_action_t tap_dance_actions[] = {
    [MSE_BTN4_DPI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mseBtn4_finished, mseBtn4_reset),
    [MSE_BTN5_ACC] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mseBtn5_finished, mseBtn5_reset)
};