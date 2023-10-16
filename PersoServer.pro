QT += core sql network

CONFIG += c++11
CONFIG += console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Log/console_log_backend.cpp \
    Log/file_log_backend.cpp \
    Log/log_system.cpp \
    Log/log_backend.cpp \
    Database/database_controller.cpp \
    Database/database_table_model.cpp \
    Database/postgres_controller.cpp \
    Log/udp_log_backend.cpp \
    Management/server_manager.cpp \
    Management/transponder_seed.cpp \
    Management/transponder_release_system.cpp \
    Management/firmware_generation_system.cpp \
    Network/perso_client.cpp \
    Network/perso_server.cpp \
    Security/des.cpp \
    StickerPrinter/isticker_printer.cpp \
    StickerPrinter/te310_printer.cpp \
    main.cpp

HEADERS += \
    Log/console_log_backend.h \
    Log/file_log_backend.h \
    Log/log_system.h \
    Log/log_backend.h \
    Database/database_controller.h \
    Database/database_table_model.h \
    Database/postgres_controller.h \
    Log/udp_log_backend.h \
    Management/firmware_generation_system.h \
    Management/server_manager.h \
    Management/transponder_release_system.h \
    Management/transponder_seed.h \
    Network/perso_client.h \
    Network/perso_server.h \
    Security/des.h \
    StickerPrinter/isticker_printer.h \
    StickerPrinter/te310_printer.h \
    General/definitions.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
