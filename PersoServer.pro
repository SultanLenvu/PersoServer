QT += core gui sql charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Database/database_buffer.cpp \
    Database/database_controller.cpp \
    GUI/gui_initial.cpp \
    GUI/gui_master.cpp \
    Database/postgres_controller.cpp \
    Management/obu_issuer_order.cpp \
    Management/perso_client_connection.cpp \
    Management/perso_host.cpp \
    Management/server_manager.cpp \
    Management/transponder_initializer.cpp \
    main.cpp \
    GUI/main_window_kernel.cpp \
    GUI/gui.cpp \
    GUI/gui_delegates.cpp \
    GUI/user_interaction_system.cpp \
    Management/log_system.cpp \
    Management/user_settings.cpp

HEADERS += \
    Database/database_buffer.h \
    Database/database_controller.h \
    GUI/gui_initial.h \
    GUI/gui_master.h \
    GUI/main_window_kernel.h \
    GUI/gui.h \
    GUI/gui_delegates.h \
    GUI/user_interaction_system.h \
    General/definitions.h \
    Database/postgres_controller.h \
    Management/log_system.h \
    Management/obu_issuer_order.h \
    Management/perso_client_connection.h \
    Management/perso_host.h \
    Management/server_manager.h \
    Management/transponder_initializer.h \
    Management/user_settings.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
