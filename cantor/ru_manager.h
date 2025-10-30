#pragma once

#include <stdint.h>

/* Depth-safe suspend/resume of a target layer (e.g., L_RUSSIAN) */
void ru_suspend(int layer);
void ru_resume(int layer);
int  ru_suspended(int layer);


