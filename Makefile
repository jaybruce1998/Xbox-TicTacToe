XBE_TITLE = nxdk\ sample\ -\ SDL2_GameController
GEN_XISO = TicTacToe.iso
SRCS = $(CURDIR)/main.c
NXDK_DIR ?= $(CURDIR)/../..
NXDK_SDL = y

include $(NXDK_DIR)/Makefile
