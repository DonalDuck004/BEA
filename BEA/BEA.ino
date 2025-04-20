#include <IRremote.hpp>

#include "BluetoothA2DPSink.h"
#include "string_utilies/string_utilies.hpp"
#include "LedHandler/LedHandler.hpp"
#include "LCDHandler/LCDHandler.hpp"
#include "IRHandler/IRHandler.hpp"

#pragma region BuildVars

// #define SERIAL_DEBUG
// #define DEBUG_IR_VALUE
#define TROLL_BUILD
#define MEM_DEBUG_BUILD
// #define MEM_EXTREME_DEBUG_BUILD

#if defined(MEM_EXTREME_DEBUG_BUILD) && defined(MEM_DEBUG_BUILD)
#   undef MEM_DEBUG_BUILD
#endif

#define BL_TRACK_ROW 0
#define BL_SINGER_ROW 1
#define BL_META_TRACK_NAME 1
#define BL_META_SINGER 2
#define BL_TRACK_NAME_MAX_LEN LCD_COLS * 4
#define BL_TRACK_SINGER_MAX_LEN LCD_COLS * 2

#define BL_LED_PIN 13
#define BL_DEVICE_NAME "BEA V2.10"

#define DISCONNECTED_MESSAGE_R0 "Disconnesso"
#define DISCONNECTED_MESSAGE_R0_STATIC true
#define DISCONNECTED_MESSAGE_R1 BL_DEVICE_NAME
#define DISCONNECTED_MESSAGE_R1_STATIC true

#define TROLL_R0_ID  0b100000
#define TROLL_R1_ID  0b010000
#define TITLE_ID     0b001000
#define AUTHOR_ID    0b000100
#define VOLUNE_ID    0b000010
#define HEAP_ID      0b000001

#define TROLL_GENERIC_ID  TROLL_R0_ID | TROLL_R1_ID

#define BL_MAX_AUDIO 127
#pragma endregion

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

constexpr byte coffee_byte = 1;

#endif

static BluetoothA2DPSink bl;
static LCDHandler* lcd = new LCDHandler();
static LedHandler* led = new LedHandler(BL_LED_PIN);
static IRHandler* ir = new IRHandler();

#pragma region PogressBars

LCDMessageStaticText* AddProgressBar(int val, int max_val, int play_for_ticks = PLAY_FOR_TICKS_DISABLED){
    static LCDMessageStaticText* msg = nullptr;
    static const int bar_size = floor((float)(LCD_COLS - 2 - 4) / 5) * 5;
    static char* buff = (char*)malloc(sizeof(char) * LCD_COLS);
    // 2 for || 4 for 000%
    memset(buff, ' ', LCD_COLS);

    int percentage = map(val, 0, max_val, 0, 100);
    int filled = map(val, 0, max_val, 0, bar_size);
    sprintf(buff + bar_size + 2, "%3d%%", percentage);
    buff[0] = '|';
    buff[bar_size + 1] = '|';
    memset(buff + 1, '#', filled);
    memset(buff + filled + 1, '-', bar_size - filled);

    if (msg == nullptr) {
        msg = new LCDMessageStaticText(1, buff, play_for_ticks, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT);
        lcd->AddMessage(msg);
    }

    msg->src_play_for_x_ticks = play_for_ticks;
    msg->Reset(false);

    return msg;
}

void ProcessProgressBar(int current_vol) {
    AddProgressBar(current_vol, 100, 3)->SetFlags(VOLUNE_ID)->priority = 1;
}

void DisplayHeap() {
#ifdef MEM_EXTREME_DEBUG_BUILD
    constexpr int display_time = 1;
#else
    constexpr int display_time = 3;
#endif

    static char* buff_r0 = (char*)malloc(sizeof(char) * LCD_COLS);

    memset(buff_r0, 0, LCD_COLS);
    uint32_t total_heap = ESP.getHeapSize();
    uint32_t allocated_heap = total_heap - ESP.getFreeHeap();


    int allocated_heap_kb = ((float)allocated_heap) / 1024;
    int total_heap_kb = ((float)total_heap) / 1024;
    sprintf(buff_r0, "Heap: %d/%dKB", allocated_heap_kb, total_heap_kb);
        
    AddProgressBar(allocated_heap, total_heap, display_time)->SetFlags(HEAP_ID)->priority = 2;

    static LCDMessageStaticText* msg0 = nullptr; 
    
    if (msg0 == nullptr) {
        msg0 = new LCDMessageStaticText(0, buff_r0, display_time, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT, 2);
        msg0->SetFlags(HEAP_ID);
        lcd->AddMessage(msg0);
    } else
        msg0->Reset();
}
#pragma endregion

#pragma region Callbacks

void HandleIR(IRHandler* self, uint16_t cmd) {
   
#ifdef DEBUG_IR_VALUE
    char* tmp = (char*)malloc(4 * sizeof(char));
    sprintf(tmp, "%d", cmd);
    lcd->AddMessage(new LCDMessageStaticText(1, tmp, 3));
#endif


#if defined(TROLL_BUILD) || defined(MEM_DEBUG_BUILD)
    bool ret = true;
#   ifdef TROLL_BUILD
    static char* troll_buff_r0 = (char*)malloc(sizeof(char) * (LCD_COLS + 2));
    static char* troll_buff_r1 = (char*)malloc(sizeof(char) * (LCD_COLS + 2));
    static LCDMessageStaticText* troll_msg_r0 = nullptr;
    static LCDMessageStaticText* troll_msg_r1 = nullptr; 

    static int last = -1;
    int val;
#   endif

    switch (cmd) {
#   ifdef TROLL_BUILD
        case IR_SPARA_STRONZATA:
            val = rand() % 4;
            if (val == last)
                val = (val + 1) % 4;

            if (troll_msg_r0 == nullptr) {
                troll_msg_r0 = (LCDMessageStaticText*)(new LCDMessageStaticText(0, troll_buff_r0, 6, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT))->SetFlags(TROLL_R0_ID);
                lcd->AddMessage(troll_msg_r0);
                troll_msg_r1 = (LCDMessageStaticText*)(new LCDMessageStaticText(1, troll_buff_r1, 6, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT))->SetFlags(TROLL_R0_ID);
                lcd->AddMessage(troll_msg_r1);
            }

            switch (last = val) {
                case 0:
                    sprintf(troll_buff_r0, "Women %c detected", coffee_byte);
                    memcpy(troll_buff_r1, "Opinion rejected", 17);
                    break;
                case 1:
                    memcpy(troll_buff_r0, "Chiesi?", 8);
                    memcpy(troll_buff_r1, "Non paresse", 12);
                    break;
                case 2:
                    memcpy(troll_buff_r0, "Null deferencing", 17);
                    memcpy(troll_buff_r1, "GODOOOOOOOOOOOOO", 17);
                    break;
                case 3:
                    memcpy(troll_buff_r0, "Maradona", 9);
                    memcpy(troll_buff_r1, "Sniffer", 8);
                    break;
                case 4:
                    // Disabled
                    lcd->AddMessage((new LCDMessageText(0, "zio mattone, zio yogurt, zio banana", true, LCDMessageFreeOpt::NO_CLEAN))->SetFlags(TROLL_R0_ID));
                    lcd->AddMessage((new LCDMessageText(1, "Sono tutti zii ma diverse categorie", true, LCDMessageFreeOpt::NO_CLEAN))->SetFlags(TROLL_R1_ID));
                    break;
            }

            troll_msg_r0->Reset();
            troll_msg_r1->Reset();
            break;
#   endif
#   ifdef MEM_DEBUG_BUILD
        case IR_HEAP_DEBUG:
            DisplayHeap();;
            break;
#   endif
        default:
            ret = false;
    }

    if (ret)
        return;

#endif


    if (!bl.is_connected())
        return;

    int current_vol = map(bl.get_volume(), 0, BL_MAX_AUDIO, 0, 100);

    switch (cmd) {
        case IR_VOL_PLUS:
            current_vol += 10;
            if (current_vol > 100) current_vol = 100;
            bl.set_volume(map(current_vol, 0, 100, 0, BL_MAX_AUDIO));

            ProcessProgressBar(current_vol);
            break;
        case IR_VOL_MINUS:
            current_vol -= 10;
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


        static LCDMessageStaticLikeText* track_msg = nullptr;
        static LCDMessageStaticLikeText* singer_msg = nullptr;
        static char* buff_1 = nullptr;
        static char* buff_2 = nullptr;

        LCDMessageStaticLikeText** tmp_msg;
        char** tmp_buff;
        int buff_len;
        int row;

        if (meta_type == BL_META_TRACK_NAME) {
            lcd->RemoveMessages();
            lcd->ClearAll();

            tmp_msg = &track_msg;
            tmp_buff = &buff_1;
            buff_len = BL_TRACK_NAME_MAX_LEN;
            row = BL_TRACK_ROW;
        } else {
            tmp_msg = &singer_msg;
            tmp_buff = &buff_2;
            buff_len = BL_TRACK_SINGER_MAX_LEN;
            row = BL_SINGER_ROW;
        }

        if (*tmp_msg == nullptr) {
            *tmp_buff = (char*)malloc(sizeof(char) * buff_len + sizeof(char));
            memset(*tmp_buff + buff_len - 3, '.', 3);
            *tmp_msg = new LCDMessageStaticLikeText(row, nullptr, false, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT);
            (*tmp_msg)->SetStr(*tmp_buff, false);
            lcd->AddMessage(*tmp_msg);
        }else
            (*tmp_msg)->Reset(false);

        int len = min(buff_len, (int)strlen((char*)meta));
        memcpy(*tmp_buff, (char*)meta, len);
        (*tmp_buff)[len] = 0;
        (*tmp_msg)->SetStrLen(len);
    }

#ifdef SERIAL_DEBUG
    Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", meta_type, meta);
#endif
}

void connection_state_changed_callback(esp_a2d_connection_state_t state, void* meta) {
    if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_CONNECTED) {
        ir->DeviceConnected();
        led->SetOn();
    } else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        lcd->RemoveMessages();
        SetDisconnectedMessage();
        led->SetDelay(BL_DISCONNECTED_BLINK_DELAY);
    } else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_CONNECTING)
        led->SetDelay(BL_CONNECTING_BLINK_DELAY);
    else if (state == esp_a2d_connection_state_t::ESP_A2D_CONNECTION_STATE_DISCONNECTING)
        led->SetDelay(BL_DISCONNECTING_BLINK_DELAY);
}
#pragma endregion

inline int DecimalLength(int n) {
    return floor(log10f(n) + 1);
}

void SetDisconnectedMessage() {
    static BaseLCDMessageText* r0_msg = nullptr;
    static BaseLCDMessageText* r1_msg = nullptr;

    if (r0_msg == nullptr) {
#if DISCONNECTED_MESSAGE_R0_STATIC
        r0_msg = new LCDMessageStaticText(0, DISCONNECTED_MESSAGE_R0, PLAY_FOR_TICKS_DISABLED, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT);
#else
        r0_msg = new LCDMessageText(0, DISCONNECTED_MESSAGE_R0, false, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT);
#endif
#if DISCONNECTED_MESSAGE_R1_STATIC
        r1_msg = new LCDMessageStaticText(1, DISCONNECTED_MESSAGE_R1, PLAY_FOR_TICKS_DISABLED, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT);
#else
        r1_msg = new LCDMessageText(1, DISCONNECTED_MESSAGE_R1, false, LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT);
#endif
        lcd->AddMessage(r0_msg);
        lcd->AddMessage(r1_msg);
    } else {
        r0_msg->Reset(false);
        r1_msg->Reset(false);
    }

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

    i2s_pin_config_t pin_config = {
        .bck_io_num = 25,
        .ws_io_num = 26,
        .data_out_num = 33,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    bl.set_pin_config(pin_config);
    bl.set_avrc_metadata_callback(avrc_metadata_callback);
    bl.set_on_connection_state_changed(connection_state_changed_callback);
    bl.start(BL_DEVICE_NAME);

    if (!bl.is_connected())  // In a rapid reboot the connection can still be active
        SetDisconnectedMessage();
}

void loop() {
#ifndef MEM_EXTREME_DEBUG_BUILD
    lcd->Tick();
    led->Tick();
    ir->Tick();
#else
    if (lcd->Tick())
        DisplayHeap();
#endif
}
