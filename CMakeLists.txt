cmake_minimum_required(VERSION 3.5)
project(PersoServer VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#=========================== Qt framework connection ==============================#
find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network PrintSupport Sql Core5Compat)

qt_standard_project_setup()
#==================================================================================#

#=========================== Project configuration ================================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PRINTERS_ENABLE OFF)
set(UNAUTHORIZE_EXPIRATION OFF)
set(SERVER_PANIC_STATE_ENABLE OFF)
configure_file(
    config.h.in config.h @ONLY)
#==================================================================================#

#=========================== Project source directories ===========================#
include_directories(Database/)
include_directories(General/)
include_directories(Log/)
include_directories(Management/)
include_directories(ProductionDispatcher/)
include_directories(Printing/)
include_directories(ClientConnection/)
include_directories(ClientConnection/Commands)
include_directories(Security/)
#==================================================================================#

add_executable(PersoServer
    Database/abstract_sql_database.cpp Database/abstract_sql_database.h
    Database/postgre_sql_database.cpp Database/postgre_sql_database.h
    Database/postgre_sql_table.cpp Database/postgre_sql_table.h
    Database/sql_table_relation.cpp Database/sql_table_relation.h
    Database/abstract_sql_table.cpp Database/abstract_sql_table.h
    Database/sql_query_values.h Database/sql_query_values.cpp

    General/definitions.h
    General/types.h

    Log/console_log_backend.cpp Log/console_log_backend.h
    Log/file_log_backend.cpp Log/file_log_backend.h
    Log/log_backend.cpp Log/log_backend.h
    Log/log_system.cpp Log/log_system.h
    Log/udp_log_backend.cpp Log/udp_log_backend.h

    Management/server_manager.cpp Management/server_manager.h
    Management/perso_server.cpp Management/perso_server.h
    Management/abstract_perso_server.h Management/abstract_perso_server.cpp
    Management/abstract_server_manager.h Management/abstract_server_manager.cpp
    Management/global_environment.h Management/global_environment.cpp

    ProductionDispatcher/abstract_production_dispatcher.h ProductionDispatcher/abstract_production_dispatcher.cpp
    ProductionDispatcher/production_dispatcher.h ProductionDispatcher/production_dispatcher.cpp
    ProductionDispatcher/transponder_release_system.cpp ProductionDispatcher/transponder_release_system.h
    ProductionDispatcher/abstract_launch_system.h ProductionDispatcher/abstract_launch_system.cpp
    ProductionDispatcher/abstract_info_system.h ProductionDispatcher/abstract_info_system.cpp
    ProductionDispatcher/abstract_release_system.h ProductionDispatcher/abstract_release_system.cpp
    ProductionDispatcher/info_system.h ProductionDispatcher/info_system.cpp
    ProductionDispatcher/production_line_launch_system.h ProductionDispatcher/production_line_launch_system.cpp
    ProductionDispatcher/abstract_firmware_generation_system.h ProductionDispatcher/abstract_firmware_generation_system.cpp
    ProductionDispatcher/firmware_generation_system.cpp ProductionDispatcher/firmware_generation_system.h

    Printing/abstract_sticker_printer.cpp Printing/abstract_sticker_printer.h
    Printing/te310_printer.cpp Printing/te310_printer.h

    ClientConnection/abstract_client_connection.h ClientConnection/abstract_client_connection.cpp
    ClientConnection/client_connection.cpp ClientConnection/client_connection.h
    ClientConnection/production_line_context.h ClientConnection/production_line_context.cpp
    ClientConnection/production_context_owner.h ClientConnection/production_context_owner.cpp

    ClientConnection/Commands/abstract_client_command.h ClientConnection/Commands/abstract_client_command.cpp
    ClientConnection/Commands/echo_comand.h ClientConnection/Commands/echo_comand.cpp
    ClientConnection/Commands/release_transponder_command.h ClientConnection/Commands/release_transponder_command.cpp
    ClientConnection/Commands/confirm_transponder_release_command.h ClientConnection/Commands/confirm_transponder_release_command.cpp
    ClientConnection/Commands/rerelease_transponder_command.h ClientConnection/Commands/rerelease_transponder_command.cpp
    ClientConnection/Commands/confirm_transponder_rerelease_command.h ClientConnection/Commands/confirm_transponder_rerelease_command.cpp
    ClientConnection/Commands/rollback_transponder_command.h ClientConnection/Commands/rollback_transponder_command.cpp
    ClientConnection/Commands/print_box_sticker_command.h ClientConnection/Commands/print_box_sticker_command.cpp
    ClientConnection/Commands/print_last_box_sticker_command.h ClientConnection/Commands/print_last_box_sticker_command.cpp
    ClientConnection/Commands/print_pallet_sticker_command.h ClientConnection/Commands/print_pallet_sticker_command.cpp
    ClientConnection/Commands/print_last_pallet_sticker_command.h ClientConnection/Commands/print_last_pallet_sticker_command.cpp
    ClientConnection/Commands/log_in_command.h ClientConnection/Commands/log_in_command.cpp
    ClientConnection/Commands/log_out_command.h ClientConnection/Commands/log_out_command.cpp
    ClientConnection/Commands/get_current_transponder_data_command.h ClientConnection/Commands/get_current_transponder_data_command.cpp
    ClientConnection/Commands/get_transponder_data_command.h ClientConnection/Commands/get_transponder_data_command.cpp
    ClientConnection/Commands/request_box_command.h ClientConnection/Commands/request_box_command.cpp
    ClientConnection/Commands/get_current_box_data_command.h ClientConnection/Commands/get_current_box_data_command.cpp
    ClientConnection/Commands/refund_current_box_command.h ClientConnection/Commands/refund_current_box_command.cpp
    ClientConnection/Commands/complete_current_box_command.h ClientConnection/Commands/complete_current_box_command.cpp

    Security/des.cpp Security/des.h

    main.cpp
    config.h.in
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
