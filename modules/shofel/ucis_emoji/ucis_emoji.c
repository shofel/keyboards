/**
 * UCIS Emoji Community Module
 *
 * This module provides UCIS emoji input support with predefined emoji sequences.
 * The actual ucis_symbol_table array is defined in introspection.c.
 */

#include QMK_KEYBOARD_H

ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0);

// This module doesn't need hooks - it only provides data structures via introspection

