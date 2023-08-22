#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/* Общие определения */
//===============================================================
#define PROGRAM_NAME "PersoHost"
#define ORGANIZATION_DOMAIN "powersynt.ru"
#define ORGANIZATION_NAME "PowerSyntez"

#define MASTER_ACCESS_PASSWORD "1995"
//===============================================================

/* Определения менеджера */
//===============================================================
#define SERVER_MANAGER_OPERATION_MAX_DURATION 10000
//===============================================================

/* Определения для базы данных Postgres */
//===============================================================
#define POSTGRES_SERVER_DEFAULT_IP "localhost"
#define POSTGRES_SERVER_DEFAULT_PORT 5432
#define POSTGRES_SERVER_DEFAULT_USER_NAME "postgres"
#define POSTGRES_SERVER_DEFAULT_PASSWORD "1995"
#define POSTGRES_DATABASE_DEFAULT_NAME "TransponderDatabase"
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
