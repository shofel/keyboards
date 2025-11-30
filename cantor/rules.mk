LTO_ENABLE = yes # better size of firmware (link time optimization)
CONSOLE_ENABLE = yes

# save a bit of space
SPACE_CADET_ENABLE = no
GRAVE_ESC_ENABLE = no
MAGIC_ENABLE = no

MOUSEKEY_ENABLE = yes      # Mouse keys
EXTRAKEY_ENABLE = yes      # Audio control and System control
DIGITIZER_ENABLE = yes     # Touchscreen. For bisection

COMBO_ENABLE = yes
LEADER_ENABLE = yes
TAP_DANCE_ENABLE = no
CAPS_WORD_ENABLE = no
DEFERRED_EXEC_ENABLE = yes
# https://docs.qmk.fm/features/key_overrides
KEY_OVERRIDE_ENABLE = yes

UNICODE_COMMON = yes
UNICODEMAP_ENABLE = yes

SRC += oneshot.c
