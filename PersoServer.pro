QT += core gui sql charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Database/database_controller.cpp \
    Database/database_table_model.cpp \
    GUI/gui_initial.cpp \
    GUI/gui_master.cpp \
    Database/postgres_controller.cpp \
    Management/administration_system.cpp \
    Management/administration_system_builder.cpp \
    Management/firmware_generation_system.cpp \
    Network/perso_client_connection.cpp \
    Network/perso_host.cpp \
    Management/server_manager.cpp \
    Management/transponder_release_system.cpp \
    Management/transponder_seed_model.cpp \
    Miscellaneous/thread_object_builder.cpp \
    Security/des.cpp \
    main.cpp \
    GUI/main_window_kernel.cpp \
    GUI/gui.cpp \
    GUI/gui_delegates.cpp \
    GUI/user_interaction_system.cpp \
    GUI/log_system.cpp \
    Network/perso_client_connection_builder.cpp

HEADERS += \
    Database/database_controller.h \
    Database/database_table_model.h \
    GUI/gui_initial.h \
    GUI/gui_master.h \
    GUI/main_window_kernel.h \
    GUI/gui.h \
    GUI/gui_delegates.h \
    GUI/user_interaction_system.h \
    General/definitions.h \
    Database/postgres_controller.h \
    Management/administration_system.h \
    GUI/log_system.h \
    Management/administration_system_builder.h \
    Management/firmware_generation_system.h \
    Management/perso_client_connection.h \
    Management/perso_host.h \
    Network/server_manager.h \
    Network/transponder_release_system.h \
    Management/transponder_seed_model.h \
    Miscellaneous/thread_object_builder.h \
    Security/des.h \
    Network/perso_client_connection_builder.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
