#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/* Общие определения */
//===============================================================
#define PROGRAM_NAME "PersoServer"
#define ORGANIZATION_DOMAIN "powersynt.ru"
#define ORGANIZATION_NAME "PowerSyntez"

#define MASTER_ACCESS_PASSWORD "1995"
#define BOX_STICKER_DATE_TEMPLATE "dd.MM.yyyy"

#define IP_PORT_MAX_VALUE 65535
#define IP_PORT_MIN_VALUE 0
//===============================================================

/* Определения системы логгирования */
//===============================================================
#define LOG_FILE_DEFAULT_MAX_NUMBER 10

#define UDP_LOG_DESTINATION_DEFAULT_IP "127.0.0.1"
#define UDP_LOG_DESTINATION_DEFAULT_PORT 6665
//===============================================================

/* Определения для базы данных Postgres */
//===============================================================

#define TRANSPONDER_ID_START_SHIFT 500000

#define POSTGRES_DEFAULT_SERVER_IP "127.0.0.1"
#define POSTGRES_DEFAULT_SERVER_PORT 5432
#define POSTGRES_DEFAULT_DATABASE_NAME "TransponderDatabase"
#define POSTGRES_DEFAULT_USER_NAME "postgres"
#define POSTGRES_DEFAULT_USER_PASSWORD "1995"

#define TIMESTAMP_DEFAULT_VALUE "2000-01-01 00:00:00"
#define POSTGRES_TIMESTAMP_TEMPLATE "yyyy-MM-dd hh:mm:ss"
//===============================================================

/* Определения для сервера */
//===============================================================
#define PERSO_SERVER_DEFAULT_LISTEN_IP "127.0.0.1"
#define PERSO_SERVER_DEFAULT_LISTEN_PORT 6666

#define RELEASER_WAIT_TIME 1000
#define DATA_BLOCK_PART_WAIT_TIME 500
#define ONETIME_TRANSMIT_DATA_SIZE 10240
#define DATA_BLOCK_MAX_SIZE 250000

#define CLIENT_MAX_COUNT 5
#define CLIENT_CONNECTION_MAX_DURATION 10000

#define RESTART_DEFAULT_PERIOD 30
//===============================================================

/* Определения для системы выпуска транспондеров */
//===============================================================
#define TRS_DEFAULT_CHECK_PERIOD 1
//===============================================================

/* Определения для генератора прошивок */
//===============================================================
#define DEFAULT_FIRMWARE_BASE_PATH "Firmware/base.bin"
#define DEFAULT_FIRMWARE_DATA_PATH "Firmware/data.bin"

#define UCID_CHAR_LENGTH 32
#define TRANSPONDER_SERIAL_NUMBER_CHAR_LENGTH 10
#define PAN_CHAR_LENGTH 19
#define FULL_PAN_CHAR_LENGTH 20
#define EFC_CONTEXT_MARK_CHAR_LENGTH 12
#define TRANSPONDER_MODEL_CHAR_LENGTH 7
#define ACCR_REFERENCE_CHAR_LENGTH 4
#define EQUIPMENT_CLASS_CHAR_LENGTH 4
#define MANUFACTURER_ID_CHAR_LENGTH 4
#define COMMON_KEY_CHAR_LENGTH 16
#define MASTER_KEY_CHAR_LENGTH 32

#define FIRMWARE_SIZE 131072
#define FIRMWARE_BASE_SIZE 126976
#define FIRMWARE_DATA_SIZE 4096
#define FIRMWARE_BASE_START_ADDRESS 0x0801F000
#define FIRMWARE_DATA_START_ADDRESS 0x08000000

#define SECURITY_KEY_COUNT 10
#define COMMON_KEY_BYTE_LENGTH 8
#define MASTER_KEY_BYTE_LENGTH 16

#define TRANSPORT_MACCRKEY_DEFAULT_VALUE "C2705BDAC258A4B5B067454F31DAFD0E"
#define TRANSPORT_MPERKEY_DEFAULT_VALUE "C1762A26109425A2D6AD54F8C2B0C2B6"

#define TRANSPORT_MAUKEY1_DEFAULT_VALUE "83B91675C49715BF10AD4F19C2C4A707"
#define TRANSPORT_MAUKEY2_DEFAULT_VALUE "972A3B8CFD6D6D6D5BBF70F15BE91534"
#define TRANSPORT_MAUKEY3_DEFAULT_VALUE "2A08BA1F25C108029DF226C445F4A7E9"
#define TRANSPORT_MAUKEY4_DEFAULT_VALUE "0723A158291A3BDF38492A9EB097A1D3"
#define TRANSPORT_MAUKEY5_DEFAULT_VALUE "76A70E430D675D499883046B6868AB2A"
#define TRANSPORT_MAUKEY6_DEFAULT_VALUE "9E94BA168625257F649E43C4A41F5149"
#define TRANSPORT_MAUKEY7_DEFAULT_VALUE "6420EA83A27F987A08A49E9D34C2BA58"
#define TRANSPORT_MAUKEY8_DEFAULT_VALUE "BC51CB9E75E020B37373026875FEBAE9"

#define COMMERCIAL_MASTER_KEY_DEFAULT_VALUE "00000000000000000000000000000000"

#define SYSTEM_PASSWORD "4C495A41030907E6"

// Расположение атрибутов транспондера в памяти
#define EFC_CONTEXT_MARK_FPI 12
#define EQUIPMENT_OBU_ID_FPI 332
#define PAYMENT_MEANS_FPI 372

#define MANUFACTURER_ID_FPI 532
#define MANUFACTURING_SERIAL_NO_FPI 572
#define EQUIPMENT_CLASS_FPI 612
#define TRANSPONDER_PART_NO_FPI 652
#define BATTERY_INSERTATION_DATE_FPI 692
#define ACCR_REFERENCE_FPI 732
#define SOUND_FPI 772
#define BATTERY_LOW_COUNTER_LIMIT_FPI 812

#define AUKEY1_FPI 841
#define AUKEY2_FPI 850
#define AUKEY3_FPI 859
#define AUKEY4_FPI 868
#define AUKEY5_FPI 877
#define AUKEY6_FPI 886
#define AUKEY7_FPI 895
#define AUKEY8_FPI 904
#define ACCRKEY_FPI 922
#define PERKEY_FPI 931

#define TRANSPONDER_LOG_FPI 2360

// Длины атрибутов в октетах
#define EFC_CONTEXT_MARK_SIZE 6
#define EQUIPMENT_OBU_ID_SIZE 4
#define PAYMENT_MEANS_SIZE 14

#define MANUFACTURER_ID_SIZE 2
#define MANUFACTURING_SERIAL_NO_SIZE 4
#define EQUIPMENT_CLASS_SIZE 2
#define TRANSPONDER_PART_NO_SIZE 7
#define BATTERY_INSERTATION_DATE_SIZE 2
#define ACCR_REFERENCE_SIZE 2
#define SOUND_SIZE 27
#define BATTERY_LOW_COUNTER_LIMIT_SIZE 2

#define TRANSPONDER_LOG_SIZE 50

#define COMMON_KEY_SIZE 8
//========================================================================

/* Определения для принтера */
//========================================================================
#define PRINTER_FOR_BOX_DEFAULT_NAME "TSC TE310 Box"
#define PRINTER_FOR_PALLET_DEFAULT_NAME "TSC TE310 Pallet"

#define TSC_TE310_LIBRARY_DEFAULT_PATH \
  "C:/Workspace/Development/PersoServer/PrinterLib/x64/TSCLIB.dll"
//========================================================================

#endif // DEFINITIONS_H
