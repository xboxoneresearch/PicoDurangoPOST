#include "common.h"

RuntimeState::RuntimeState(Display display) :
    _display(display),
    _postCodeQueue(sizeof(SegmentData), POST_MAX_QUEUE_SIZE, FIFO),
    _socPostLineQueue(sizeof(SocPostCode), POST_MAX_QUEUE_SIZE, FIFO)
{}

bool RuntimeState::begin() {
    _display.begin();
    initialized = true;

    return true;
}