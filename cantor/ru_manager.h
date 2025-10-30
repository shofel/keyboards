#pragma once

#include <stdint.h>

/* Depth-safe suspend/resume of a target layer (e.g., L_RUSSIAN) */
void ru_suspend(uint8_t layer);
void ru_resume(uint8_t layer);
int  ru_suspended(uint8_t layer);


