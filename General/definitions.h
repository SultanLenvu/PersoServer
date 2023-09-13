#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/* Общие определения */
//===============================================================
#define PROGRAM_NAME "PersoHost"
#define ORGANIZATION_DOMAIN "powersynt.ru"
#define ORGANIZATION_NAME "PowerSyntez"

#define MASTER_ACCESS_PASSWORD "1995"
#define TIMESTAMP_TEMPLATE "yyyy-MM-dd hh:mm:ss"
//===============================================================

/* Определения менеджера */
//===============================================================
#define SERVER_MANAGER_OPERATION_MAX_DURATION 60000
//===============================================================

/* Определения для генератора прошивок */
//===============================================================
#define UCID_CHAR_LENGTH 32
#define TRANSPONDER_SERIAL_NUMBER_CHAR_LENGTH 10
#define PAYMENT_MEANS_CHAR_LENGTH 19
#define EFC_CONTEXT_MARK_CHAR_LENGTH 12
#define TRANSPONDER_MODEL_CHAR_LENGTH 7
#define ACCR_REFERENCE_CHAR_LENGTH 4
#define COMMON_KEY_CHAR_LENGTH 16
#define MASTER_KEY_CHAR_LENGTH 32
//===============================================================

/* Определения для базы данных Postgres */
//===============================================================
#define TRANSPONDER_ID_START_SHIFT 500000

#define POSTGRES_SERVER_DEFAULT_IP "localhost"
#define POSTGRES_SERVER_DEFAULT_PORT 5432
#define POSTGRES_SERVER_DEFAULT_USER_NAME "postgres"
#define POSTGRES_SERVER_DEFAULT_PASSWORD "1995"
#define POSTGRES_DATABASE_DEFAULT_NAME "TransponderSeedbase"

#define TIMESTAMP_DEFAULT_VALUE "2000-01-01 T00:00:00.000"
//===============================================================

/* Определения для хоста */
//===============================================================
#define PERSO_SERVER_DEFAULT_IP "127.0.0.1"
#define PERSO_SERVER_DEFAULT_PORT 6000
#define IP_PORT_MAX_VALUE 65535
#define IP_PORT_MIN_VALUE 0

#define DATA_BLOCK_PART_WAIT_TIME 500
#define ONETIME_TRANSMIT_DATA_SIZE 10240
#define DATA_BLOCK_MAX_SIZE 250000

#define CLIENT_MAX_COUNT 5
#define CLIENT_CONNECTION_MAX_DURATION 10000
//===============================================================

/* Определения, касающиеся ключей безопасности транспондера */
//========================================================================
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
//========================================================================

#endif // DEFINITIONS_H
