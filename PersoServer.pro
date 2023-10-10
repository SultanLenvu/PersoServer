QT += core sql network

CONFIG += c++11
CONFIG += console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Database/database_controller.cpp \
    Database/database_table_model.cpp \
    Database/postgres_controller.cpp \
    Management/perso_manager.cpp \
    Management/transponder_seed.cpp \
    Management/transponder_release_system.cpp \
    Management/firmware_generation_system.cpp \
    Management/log_system.cpp \
    Network/perso_client.cpp \
    Network/perso_server.cpp \
    Security/des.cpp \
    main.cpp

HEADERS += \
    General/definitions.h \
    Database/database_controller.h \
    Database/database_table_model.h \
    Database/postgres_controller.h \
    Management/log_system.h \
    Management/firmware_generation_system.h \
    Management/perso_manager.h \
    Management/transponder_release_system.h \
    Management/transponder_seed.h \
    Network/perso_client.h \
    Network/perso_server.h \
    Security/des.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
