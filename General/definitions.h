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
#define UCID_LENGTH 32
#define TRANSPONDER_SERIAL_NUMBER_LENGTH 10
#define PAYMENT_MEANS_LENGTH 19
#define EFC_CONTEXT_MARK_LENGTH 12
#define TRANSPONDER_MODEL_LENGTH 7
#define ACCR_REFERENCE_LENGTH 4
#define COMMON_KEY_LENGTH 16
#define MASTER_KEY_LENGTH 32
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

#endif // DEFINITIONS_H
