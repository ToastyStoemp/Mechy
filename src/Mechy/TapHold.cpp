#include "Mechy.h"
#include "TapHold.h"
#include <Keyboard.h>

#define TAPHOLD_DELAY 250

TapHold::TapHold() {
    eventArray = NULL;
}

uint8_t TapHold::defaultName() {
    return FN_TAP_HOLD;
}

void TapHold::begin() {
    TapHoldEvent* array = (TapHoldEvent*)malloc(sizeof(TapHoldEvent) * TapHold::keys);
    TapHoldKey* ptr = TapHold::keyPtrStack;
    while (ptr) {
        array[ptr->keyIndex].tapKey = ptr->tapKey;
        array[ptr->keyIndex].holdKey = ptr->holdKey;
        array[ptr->keyIndex].isActive = false;
        array[ptr->keyIndex].modifierSnapshot = 0;
        free(ptr);
        ptr = ptr->next;
    }
    eventArray = array;
}

void TapHold::run(Event* event) {
    TapHoldEvent* keyPtr = eventArray + event->key;

    if (event->isPressed()) {
        keyPtr->modifierSnapshot = mechy->currentModifiers();
        keyPtr->isActive = true;
    }
    else if (keyPtr->isActive) {
        if (event->isHeld() && event->duration > TAPHOLD_DELAY) {
            uint16_t mods = mechy->currentModifiers();
            mechy->updateModifiers(keyPtr->modifierSnapshot);
            mechy->runPlugin(KEY_STATE_PRESSED, &keyPtr->holdKey, 0);
            mechy->runPlugin(KEY_STATE_RELEASED, &keyPtr->holdKey, 1);
            mechy->updateModifiers(mods);
            keyPtr->isActive = false;
        }
        else if (event->isReleased()) {
            uint16_t mods = mechy->currentModifiers();
            mechy->updateModifiers(keyPtr->modifierSnapshot);
            mechy->runPlugin(KEY_STATE_PRESSED, &keyPtr->tapKey, 0);
            mechy->runPlugin(KEY_STATE_RELEASED, &keyPtr->tapKey, 1);
            mechy->updateModifiers(mods);
            keyPtr->isActive = false;
        }
    }
}

uint16_t TapHold::add(KBD tapKey, KBD holdKey) {
    uint8_t keyIndex = TapHold::keys;
    TapHoldKey* ptr = (TapHoldKey*)malloc(sizeof(TapHoldKey));
    ptr->keyIndex = keyIndex;
    ptr->tapKey = tapKey;
    ptr->holdKey = holdKey;
    ptr->next = TapHold::keyPtrStack;
    TapHold::keyPtrStack = ptr;
    ++TapHold::keys;
    return keyIndex;
}

uint8_t TapHold::keys = 0;
TapHoldKey* TapHold::keyPtrStack = NULL;