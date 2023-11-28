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
    Management/firmware_generation_system.cpp Management/firmware_generation_system.h
    Management/server_manager.cpp Management/server_manager.h
    Management/transponder_release_system.cpp Management/transponder_release_system.h
    Management/transponder_seed.cpp Management/transponder_seed.h
    Network/perso_client_connection.cpp Network/perso_client_connection.h
    Network/perso_server.cpp Network/perso_server.h
    Security/des.cpp Security/des.h
    StickerPrinter/isticker_printer.cpp StickerPrinter/isticker_printer.h
    StickerPrinter/te310_printer.cpp StickerPrinter/te310_printer.h
    main.cpp
    Database/sql_table_relation.h Database/sql_table_relation.cpp
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
