VIA_ENABLE = yes

TAP_DANCE_ENABLE = yes
MOUSEKEY_ENABLE = yes

#CONSOLE_ENABLE = yes

# MACCEL
SRC += ./maccel/maccel.c
ifeq ($(strip $(VIA_ENABLE)), yes)
	ifeq ($(strip $(MACCEL_VIA_ENABLE)), yes)
		SRC += ./maccel/maccel_via.c
	endif
endif
OPT_DEFS += -DMACCEL_ENABLE