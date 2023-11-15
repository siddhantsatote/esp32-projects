#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Picovoice_EN.h>
#include "params.h"

#define MEMORY_BUFFER_SIZE (70 * 1024)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);  // Initialize OLED display

static pv_picovoice_t *handle = NULL;
static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

static void wake_word_callback(void) {
    Serial.println("Wake word detected!");
    
    // Display wake word detection on OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Wake Word!");
    display.display();
    delay(2000);  // Display for 2 seconds
    display.clearDisplay();
}

static void inference_callback(pv_inference_t *inference) {
    Serial.println("Inference result:");
    // Extract and print inference results
    Serial.println("{...}");
    pv_inference_delete(inference);
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Initialize OLED display
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.display();
    delay(2000);
    display.clearDisplay();

    pv_status_t status = pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        Serial.print("Audio init failed with ");
        Serial.println(pv_status_to_string(status));
        while (1);
    }

    char **message_stack = NULL;
    int32_t message_stack_depth = 0;
    pv_status_t error_status;

    status = pv_picovoice_init(
        "${VFsYl0uXGtnw6a7req6ho0uffgTei8f3vfNSjytQT+A/WSf4W6gfdA==}", 
        MEMORY_BUFFER_SIZE,
        memory_buffer,
        sizeof(KEYWORD_ARRAY),
        KEYWORD_ARRAY,
        0.75f,  
        wake_word_callback,
        0,  
        NULL,  
        0.5f,  
        1.0f,  
        true,  
        inference_callback,
        &handle);

    if (status != PV_STATUS_SUCCESS) {
        Serial.print("Picovoice init failed with ");
        Serial.println(pv_status_to_string(status));

        error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
        if (error_status != PV_STATUS_SUCCESS) {
            Serial.println("Unable to get Picovoice error state");
            while (1);
        }

       
        for (int32_t i = 0; i < message_stack_depth; i++) {
            Serial.println(message_stack[i]);
        }

        pv_free_error_stack(message_stack);
        while (1);
    }

    Serial.println("Initialization successful!");
}

void loop() {
    const int16_t *buffer = pv_audio_rec_get_new_buffer();
    if (buffer) {
        const pv_status_t status = pv_picovoice_process(handle, buffer);
        if (status != PV_STATUS_SUCCESS) {
            Serial.print("Picovoice process failed with ");
            Serial.println(pv_status_to_string(status));
            char **message_stack = NULL;
            int32_t message_stack_depth = 0;
            pv_get_error_stack(&message_stack, &message_stack_depth);

            for (int32_t i = 0; i < message_stack_depth; i++) {
                Serial.println(message_stack[i]);
            }

            pv_free_error_stack(message_stack);
            while (1);
        }
    }
}