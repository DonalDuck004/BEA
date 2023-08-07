#include <LiquidCrystal.h>
#include <IRremote.hpp>

#include "BluetoothA2DPSink.h"
#include "string_utilies/string_utilies.hpp"
#include "LedHandler/LedHandler.hpp"
#include "LCDHandler/LCDHandler.hpp"
#include "IRHandler/IRHandler.hpp"

// #define SERIAL_DEBUG
// #define DEBUG_IR_VALUE
#define TROLL_BUILD
#define MEM_DEBUG_BUILD

#define BL_META_TRACK_NAME 1
#define BL_META_SINGER 2

#define BL_LED_PIN 13
#define BL_DEVICE_NAME "BEA V1.9"

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
#define IR_SPARA_STRONZATA 22
#define IR_HEAP_DEBUG 25

#define TROLL_R0_ID  0b100000
#define TROLL_R1_ID  0b010000
#define TITLE_ID     0b001000
#define AUTHOR_ID    0b000100
#define VOLUNE_ID    0b000010
#define HEAP_ID      0b000001

#define TROLL_GENERIC_ID  TROLL_R0_ID | TROLL_R1_ID

#define BL_MAX_AUDIO 127

#ifdef TROLL_BUILD

static byte coffee[8] = {
  B00100,
  B01000,
  B00100,
  B01111,
  B01011,
  B01010,
  B11111,
  B01110,
};

const byte coffee_byte = 1;

#endif

static LCDHandler* lcd = new LCDHandler(new LiquidCrystal(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN), LCD_COLS, LCD_ROWS, LCD_TICK_DELAY);
static BluetoothA2DPSink bl;
static LedHandler* led = new LedHandler(BL_LED_PIN);
static IRHandler* ir = new IRHandler(IR_PIN, IR_TICK_DELAY);


inline int DecimalLength(int n) {
    return floor(log10f(n) + 1);
}

// TODO MERGE ProcessProgressBar and DisplayHeap
// TODO Use map where possible

void ProcessProgressBar(int current_vol) {
    char* str_number;
    char* buff;
    int filled;

    assert(current_vol >= 0 && current_vol <= 100);

    str_number = (char*)malloc(sizeof(char) * 5);
    sprintf(str_number, "%3d%%", current_vol);

    int bar_size = floor((float)(LCD_COLS - 3 - 3) / 5) * 5;
    filled = (float)current_vol / 100 * bar_size;
    buff = (char*)malloc(sizeof(char) * (LCD_COLS + 3 + 3 + 1));
    buff[0] = '|';
    buff[bar_size + 1] = '|';
    memmove(buff + bar_size + 2, str_number, strlen(str_number));
    memset(buff + 1, '#', filled);
    memset(buff + filled + 1, '-', bar_size - filled);
    
    LCDMessageStaticText* msg = new LCDMessageStaticText(1, buff, 3);
    msg->SetFlags(VOLUNE_ID);
    lcd->RemoveMessagesWithFlags(VOLUNE_ID, true, 1);
    lcd->AddMessage(msg);
    free(str_number);
}

void DisplayHeap() {
    const int bar_size = floor((float)(LCD_COLS - 2 - 4) / 5) * 5;
                                      // 2 for || 4 for 000%
   
    uint32_t total_heap = ESP.getHeapSize();
    uint32_t allocated_heap = total_heap - ESP.getFreeHeap();
    int percentage = map(allocated_heap, 0, total_heap, 0, 100);
    int filled = map(allocated_heap, 0, total_heap, 0, bar_size);
    char* buff = (char*)malloc(sizeof(char) * (LCD_COLS + 1));
    char* str_number = (char*)malloc(sizeof(char) * 5); // TODO Write directly to buff
    sprintf(str_number, "%3d%%", percentage);
    buff[0] = '|';
    buff[bar_size + 1] = '|';
    memmove(buff + bar_size + 2, str_number, strlen(str_number));
    memset(buff + 1, '#', filled);
    memset(buff + filled + 1, '-', bar_size - filled);
    free(str_number);

    int allocated_heap_kb = ((float)allocated_heap) / 1024;
    int total_heap_kb = ((float)total_heap) / 1024;
    
    LCDMessageStaticText* msg1 = new LCDMessageStaticText(1, buff, 3);
    buff = (char*)malloc(sizeof(char) * (10 + DecimalLength(allocated_heap_kb) + DecimalLength(total_heap_kb)));
    sprintf(buff, "Heap: %d/%dKB", allocated_heap_kb, total_heap_kb);
    LCDMessageStaticText* msg2 = new LCDMessageStaticText(0, buff, 3);
    msg1->SetFlags(HEAP_ID);
    msg2->SetFlags(HEAP_ID);
    lcd->RemoveMessagesWithFlags(HEAP_ID, true, 2);
    lcd->AddMessage(msg1);
    lcd->AddMessage(msg2);
}

void HandleIR(IRHandler* self, uint16_t cmd) {
    bool ret = true;
   
#ifdef DEBUG_IR_VALUE
    char* tmp = (char*)malloc(4 * sizeof(char));
    sprintf(tmp, "%d", cmd);
    lcd->AddMessage(new LCDMessageStaticText(1, tmp, 3));
#endif

#if defined(TROLL_BUILD) || defined(MEM_DEBUG_BUILD)
    char* buff_r0 = NULL;
    char* buff_r1 = NULL;
#endif

#ifdef TROLL_BUILD
    static int last = -1;
    int val;
#endif

    switch (cmd) {
#ifdef TROLL_BUILD
        case IR_SPARA_STRONZATA:
            if (lcd->RemoveMessagesWithFlags(TROLL_R0_ID, true, 1))
                lcd->RemoveMessagesWithFlags(TROLL_R1_ID, true, 1);
            val = rand() % 5;
            if (val == last)
                val = (val + 1) % 5;

            switch (last = val) {
                case 0:
                    buff_r0 = (char*)malloc(sizeof(char) * 17);
                    sprintf(buff_r0, "Women %c detected", coffee_byte);
                    lcd->AddMessage((new LCDMessageStaticText(0, buff_r0, 6))->SetFlags(TROLL_R0_ID));
                    buff_r0 = NULL;

                    buff_r1 = "Opinion rejected";
                    break;
                case 1:
                    buff_r0 = "Chiesi?         ";
                    buff_r1 = "Non paresse     ";
                    break;
                case 2:
                    buff_r0 = "Null deferencing";
                    buff_r1 = "GODOOOOOOOOOOOOO";
                    break;
                case 3:
                    buff_r0 = "Maradona        ";
                    buff_r1 = "Sniffer         ";
                    break;
                case 4:
                    break;
                    // TODO FIX
                    lcd->AddMessage((new LCDMessageText(0, "zio mattone, zio yogurt, zio banana", true, LCDMessageFreeOpt::NO_CLEAN))->SetFlags(TROLL_R0_ID));
                    lcd->AddMessage((new LCDMessageText(1, "Sono tutti zii ma diverse categorie", true, LCDMessageFreeOpt::NO_CLEAN))->SetFlags(TROLL_R1_ID));
                    break;
            }

            if (buff_r0 != NULL)
                lcd->AddMessage((new LCDMessageStaticText(0, buff_r0, 6, LCDMessageFreeOpt::NO_CLEAN))->SetFlags(TROLL_R0_ID));

            if (buff_r1 != NULL)
                lcd->AddMessage((new LCDMessageStaticText(1, buff_r1, 6, LCDMessageFreeOpt::NO_CLEAN))->SetFlags(TROLL_R1_ID));

            break;
#endif
#ifdef MEM_DEBUG_BUILD
        case IR_HEAP_DEBUG:
            DisplayHeap();;
            break;
#endif
        default:
            ret = false;
    }

    if (!bl.is_connected() || ret)
        return;

    int current_vol = ((float)bl.get_volume() / BL_MAX_AUDIO * 100) / 10 * 10;

    switch (cmd) {
        case IR_VOL_PLUS:
            current_vol = current_vol + 10;
            if (current_vol > 100) current_vol = 100;
            bl.set_volume(map(current_vol, 0, 100, 0, BL_MAX_AUDIO));

            ProcessProgressBar(current_vol);
            break;
        case IR_VOL_MINUS:
            current_vol = current_vol - 10;
            if (current_vol < 0) current_vol = 0;
            bl.set_volume(map(current_vol, 0, 100, 0, BL_MAX_AUDIO));

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

        BaseLCDMessageText* lcd_msg;
        if (strlen(msg) <= LCD_COLS)
            lcd_msg = new LCDMessageStaticText(meta_type - 1, msg);
        else 
            lcd_msg = new LCDMessageText(meta_type - 1, msg);

        lcd->AddMessage(lcd_msg);
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
        SetDisconnectedMessage();
        led->SetDelay(BL_DISCONNECTED_BLINK_DELAY);
    }
    else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_CONNECTING)
        led->SetDelay(BL_CONNECTING_BLINK_DELAY);
    else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_DISCONNECTING)
        led->SetDelay(BL_DISCONNECTING_BLINK_DELAY);
}

void SetDisconnectedMessage() {
    BaseLCDMessageText* lcd_msg;
    lcd->ClearAll();

    if (DISCONNECTED_MESSAGE_R0_STATIC)
        lcd_msg = new LCDMessageStaticText(0, DISCONNECTED_MESSAGE_R0, LCDMessageFreeOpt::NO_CLEAN);
    else
        lcd_msg = new LCDMessageText(0, DISCONNECTED_MESSAGE_R0, LCDMessageFreeOpt::NO_CLEAN);
    lcd->AddMessage(lcd_msg);

    if (DISCONNECTED_MESSAGE_R1)
        lcd_msg = new LCDMessageStaticText(1, DISCONNECTED_MESSAGE_R1, LCDMessageFreeOpt::NO_CLEAN);
    else
        lcd_msg = new LCDMessageText(1, DISCONNECTED_MESSAGE_R1, LCDMessageFreeOpt::NO_CLEAN);

    lcd->AddMessage(lcd_msg);
}

void setup() {
#ifdef SERIAL_DEBUG
    Serial.begin(9600);
    Serial.write("Test");
#endif
#ifdef TROLL_BUILD
    lcd->GetRaw()->createChar(coffee_byte, coffee);
    lcd->GetRaw()->clear();
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

    if (!bl.is_connected())  // In a rapid reboot the connection can still be active
        SetDisconnectedMessage();
}

void loop() {
    lcd->Tick();
    led->Tick();
    ir->Tick();
}
