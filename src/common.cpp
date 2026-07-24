#include "common.h"

RuntimeState::RuntimeState(Display display) :
    _postCodeQueue(sizeof(SegmentData), POST_MAX_QUEUE_SIZE, FIFO),
    _display(display)
{}

bool RuntimeState::begin() {
    _display.begin();
    initialized = true;

    return true;
}