file(GLOB SRC_FILES *.h *.cpp)

generate_energia_firmware(ATOS_Reset
    SKETCH ATOS_Reset.ino
    SRCS ${SRC_FILES}
    BOARD witii_cc3200
    PORT /dev/tty.usbserial-cc3101B)