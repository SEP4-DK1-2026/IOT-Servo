#include "windowopener.h"
#include "stdbool.h"

bool shouldOpen(float temp, bool hasRained, float wantedTemp) {
    return temp <= wantedTemp || hasRained ? false : true;
}