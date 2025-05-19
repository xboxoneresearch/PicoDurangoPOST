#include "common.h"

RuntimeState::RuntimeState() :
    _postCodeQueue(sizeof(SegmentData), POST_MAX_QUEUE_SIZE, FIFO),
    _display(DISP_SCREEN_WIDTH, DISP_SCREEN_HEIGHT, SSD1306_DISP_ADDRESS, &Wire1)
{}

bool RuntimeState::begin() {
    _display.begin(PIN_SDA_DISP, PIN_SCL_DISP);
    initialized = true;

    return true;
}