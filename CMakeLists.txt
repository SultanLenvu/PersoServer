cmake_minimum_required(VERSION 3.16)
project(PersoServer VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network PrintSupport Sql Core5Compat)

qt_standard_project_setup()

qt_add_executable(PersoServer MACOSX_BUNDLE
    Database/abstract_sql_database.cpp Database/abstract_sql_database.h
    Database/postgre_sql_database.cpp Database/postgre_sql_database.h
    Database/postgre_sql_table.cpp Database/postgre_sql_table.h
    Database/sql_table_relation.cpp Database/sql_table_relation.h
    Database/abstract_sql_table.cpp Database/abstract_sql_table.h
    Database/sql_query_values.h Database/sql_query_values.cpp

    General/definitions.h

    Log/console_log_backend.cpp Log/console_log_backend.h
    Log/file_log_backend.cpp Log/file_log_backend.h
    Log/log_backend.cpp Log/log_backend.h
    Log/log_system.cpp Log/log_system.h
    Log/udp_log_backend.cpp Log/udp_log_backend.h

    Management/server_manager.cpp Management/server_manager.h
    Management/perso_server.cpp Management/perso_server.h
    Management/abstract_perso_server.h Management/abstract_perso_server.cpp
    Management/abstract_server_manager.h Management/abstract_server_manager.cpp
    Management/global_context.h Management/global_context.cpp

    ProductionDispatcher/abstract_production_dispatcher.h ProductionDispatcher/abstract_production_dispatcher.cpp
    ProductionDispatcher/general_production_dispatcher.h ProductionDispatcher/general_production_dispatcher.cpp
    ProductionDispatcher/transponder_release_system.cpp ProductionDispatcher/transponder_release_system.h
    ProductionDispatcher/transponder_seed.cpp ProductionDispatcher/transponder_seed.h
    ProductionDispatcher/isticker_printer.cpp ProductionDispatcher/isticker_printer.h
    ProductionDispatcher/te310_printer.cpp ProductionDispatcher/te310_printer.h
    ProductionDispatcher/abstract_authorization_system.h ProductionDispatcher/abstract_authorization_system.cpp
    ProductionDispatcher/abstract_info_system.h ProductionDispatcher/abstract_info_system.cpp
    ProductionDispatcher/abstract_transponder_release_system.h ProductionDispatcher/abstract_transponder_release_system.cpp
    ProductionDispatcher/authorization_system.h ProductionDispatcher/authorization_system.cpp
    ProductionDispatcher/info_system.h ProductionDispatcher/info_system.cpp

    ClientConnection/abstract_client_connection.h ClientConnection/abstract_client_connection.cpp
    ClientConnection/production_line_connection.cpp ClientConnection/production_line_connection.h
    ClientConnection/firmware_generation_system.cpp ClientConnection/firmware_generation_system.h
    ClientConnection/abstract_firmware_generation_system.h ClientConnection/abstract_firmware_generation_system.cpp
    ClientConnection/abstract_client_command.h ClientConnection/abstract_client_command.cpp
    ClientConnection/echo_comand.h ClientConnection/echo_comand.cpp
    ClientConnection/transponder_release_command.h ClientConnection/transponder_release_command.cpp
    ClientConnection/transponder_release_confirm_command.h ClientConnection/transponder_release_confirm_command.cpp
    ClientConnection/transponder_rerelease_command.h ClientConnection/transponder_rerelease_command.cpp
    ClientConnection/transponder_rerelease_confirm_command.h ClientConnection/transponder_rerelease_confirm_command.cpp
    ClientConnection/rollback_command.h ClientConnection/rollback_command.cpp
    ClientConnection/box_sticker_print_command.h ClientConnection/box_sticker_print_command.cpp
    ClientConnection/last_box_sticker_print_command.h ClientConnection/last_box_sticker_print_command.cpp
    ClientConnection/pallet_sticker_print_command.h ClientConnection/pallet_sticker_print_command.cpp
    ClientConnection/last_pallet_sticker_print_command.h ClientConnection/last_pallet_sticker_print_command.cpp

    Security/des.cpp Security/des.h

    main.cpp
    )
target_compile_definitions(PersoServer PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

target_link_libraries(PersoServer PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Sql
    Qt6::Core5Compat
)

install(TARGETS PersoServer
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET PersoServer
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
