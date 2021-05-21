QT += widgets serialport
requires(qtConfig(combobox))

TARGET = receiver
TEMPLATE = app

HEADERS += \
    commanddecoder.h \
    dialog.h

SOURCES += \
    commanddecoder.cpp \
    main.cpp \
    dialog.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/receiver
INSTALLS += target
