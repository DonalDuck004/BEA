#include <LiquidCrystal.h>
#include <IRremote.hpp>

#include "BluetoothA2DPSink.h"
#include "string_utilies/string_utilies.hpp"
#include "LedHandler/LedHandler.hpp"
#include "LCDHandler/LCDHandler.hpp"
#include "IRHandler/IRHandler.hpp"

// #define SERIAL_DEBUG

#define BL_META_TRACK_NAME 1
#define BL_META_SINGER 2

#define BL_LED_PIN 13
#define BL_DEVICE_NAME "BEA V1.5"

#define BL_DISCONNECTED_BLINK_DELAY 500
#define BL_CONNECTING_BLINK_DELAY 250
#define BL_DISCONNECTING_BLINK_DELAY 750

#define LCD_RS_PIN 19
#define LCD_ENABLE_PIN 23
#define LCD_D0_PIN 18
#define LCD_D1_PIN 17
#define LCD_D2_PIN 16
#define LCD_D3_PIN 15

#define LCD_ROWS 2
#define LCD_COLS 16

#define LCD_TICK_DELAY 750

#define IR_PIN 34

#define IR_TICK_DELAY 250

#define DISCONNECTED_MESSAGE_R0 "Disconnesso"
#define DISCONNECTED_MESSAGE_R0_STATIC true
#define DISCONNECTED_MESSAGE_R1 BL_DEVICE_NAME
#define DISCONNECTED_MESSAGE_R1_STATIC true


#define IR_DISCONNECT 69
#define IR_VOL_PLUS 70
#define IR_VOL_MINUS 21
#define IR_PAUSE 64
#define IR_NEXT 67
#define IR_BACK 68

#define TITLE_ID  0b100
#define AUTHOR_ID 0b010
#define VOLUNE_ID 0b001

#define BL_MAX_AUDIO 127

static LCDHandler* lcd = new LCDHandler(new LiquidCrystal(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN), LCD_COLS, LCD_ROWS, LCD_TICK_DELAY);
static BluetoothA2DPSink bl;
static LedHandler* led = new LedHandler(BL_LED_PIN);
static IRHandler* ir = new IRHandler(IR_PIN, IR_TICK_DELAY);

void ProcessProgressBar(int current_vol) {
    char* str_number;
    char* buff;
    int bar_size;
    int filled;

    assert(current_vol >= 0 && current_vol <= 100);

    str_number = (char*)malloc(sizeof(char) * 5);
    sprintf(str_number, "%3d%%", current_vol);

    bar_size = floor((float)(LCD_COLS - 3 - 3) / 5) * 5;
    filled = (float)current_vol / 100 * bar_size;
    buff = (char*)malloc(sizeof(char) * (LCD_COLS + 3 + 3 + 1));
    buff[0] = '|';
    buff[bar_size + 1] = '|';
    memmove(buff + bar_size + 2, str_number, strlen(str_number));

    memset(buff + 1, '#', filled);
    memset(buff + filled + 1, '-', bar_size - filled);

    lcd->AddMessage(buff, 1, 3, true, VOLUNE_ID);
}

void HandleIR(IRHandler* self, uint16_t cmd) {
    if (!bl.is_connected())
        return;

    int current_vol = ((float)bl.get_volume() / BL_MAX_AUDIO * 100) / 10 * 10;
    switch (cmd) {
        case IR_VOL_PLUS:
            current_vol = current_vol + 10;
            if (current_vol > 100) current_vol = 100;
            bl.set_volume((float)current_vol / 100 * BL_MAX_AUDIO);

            ProcessProgressBar(current_vol);
            break;
        case IR_VOL_MINUS:
            current_vol = current_vol - 10;
            if (current_vol < 0) current_vol = 0;
            bl.set_volume((float)current_vol / 100 * BL_MAX_AUDIO);

            ProcessProgressBar(current_vol);
            break;
        case IR_PAUSE: // get_audio_state ?
            if (self->playing)
                bl.play();
            else
                bl.pause();

            self->playing = !self->playing;
            break;
        case IR_NEXT:
            bl.next();
            break;
        case IR_BACK:
            bl.previous();
            break;
        case IR_DISCONNECT:
            bl.disconnect();
            break;
    }
}



void avrc_metadata_callback(uint8_t meta_type, const uint8_t* meta) {
    if (meta_type == BL_META_TRACK_NAME || meta_type == BL_META_SINGER) {
        char* msg = str_copy((char*)meta);
        if (meta_type == BL_META_TRACK_NAME) {
            lcd->RemoveMessages();
            lcd->ClearAll();
        }

        lcd->AddMessage(msg, meta_type - 1, PLAY_FOR_TICKS_DISABLED, strlen(msg) <= LCD_COLS);
    }
#ifdef SERIAL_DEBUG
    Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", meta_type, meta);
#endif
}

void connection_state_changed_callback(esp_a2d_connection_state_t state, void* meta) {
    if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_CONNECTED) {
        ir->DeviceConnected();
        led->SetOn();
    }
    else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        lcd->RemoveMessages();
        lcd->ClearAll();
        lcd->AddMessage(DISCONNECTED_MESSAGE_R0, 0, PLAY_FOR_TICKS_DISABLED, DISCONNECTED_MESSAGE_R0_STATIC, LCDMessageFreeOpt::NO_CLEAN);
        lcd->AddMessage(DISCONNECTED_MESSAGE_R1, 1, PLAY_FOR_TICKS_DISABLED, DISCONNECTED_MESSAGE_R1_STATIC, LCDMessageFreeOpt::NO_CLEAN);
        led->SetDelay(BL_DISCONNECTED_BLINK_DELAY);
    }
    else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_CONNECTING) {
        led->SetDelay(BL_CONNECTING_BLINK_DELAY);
    }
    else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_DISCONNECTING) {
        led->SetDelay(BL_DISCONNECTING_BLINK_DELAY);
    }
}

void setup() {
#ifdef SERIAL_DEBUG
    Serial.begin(9600);
    Serial.write("Test");
#endif
    ir->SetCallback(HandleIR);

    static i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100, // updated automatically by A2DP
        .bits_per_sample = (i2s_bits_per_sample_t)32,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = true,
        .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
    };
    bl.set_i2s_config(i2s_config);

    i2s_pin_config_t my_pin_config = {
        .bck_io_num = 25,
        .ws_io_num = 26,
        .data_out_num = 33,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    bl.set_pin_config(my_pin_config);
    bl.set_avrc_metadata_callback(avrc_metadata_callback);
    bl.set_on_connection_state_changed(connection_state_changed_callback);
    bl.start(BL_DEVICE_NAME);

    if (!bl.is_connected()) { // In a rapid reboot the connection can still be active
        lcd->ClearAll();
        lcd->AddMessage(DISCONNECTED_MESSAGE_R0, 0, PLAY_FOR_TICKS_DISABLED, DISCONNECTED_MESSAGE_R0_STATIC, LCDMessageFreeOpt::NO_CLEAN);
        lcd->AddMessage(DISCONNECTED_MESSAGE_R1, 1, PLAY_FOR_TICKS_DISABLED, DISCONNECTED_MESSAGE_R1_STATIC, LCDMessageFreeOpt::NO_CLEAN);
    }

}

void loop() {
    lcd->Tick();
    led->Tick();
    ir->Tick();
}