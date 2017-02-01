#ifndef CONST
#define CONST
#include <QtGlobal>

//ENUMS

typedef enum {
               ecSendNothing,
               ecSendDBEvents,
               ecSendNewVehicle,
               ecReadVehicle,
               ecWriteLogVehicle,
               ecPeriodicalReport
             }TeEmailCategory;



//konstanty
#define PROJECT_NAME_BVG_2010         "BCN"
#define PROJECT_NAME_GTA              "GTA"
#define PROJECT_NAME_GTB              "GTB"

#define CW_HEARTBEAT_TRAM             0x2000
#define KN_VEHICLE_NUMBER_OFFSET_BCN  10000
#define CW_REQ_ALL_LOG                0x7000
#define CW_READ_TROL_LOG              0x7000
#define CW_READ_TRAM_LOG              0x7010
#define REQ_READ_READ_ALL             0xFFFF
#define VEHICLE_UDP_PORT_TROL_LOG     50010
#define VEHICLE_UDP_PORT_TRAM_LOG     13001
#define HEARTBEAT_RECEIVE_PORT        13000
//#define VEHICLE_UDP_PORT              50005
//#define DEBUG_JAMIC_ID                0x00010103
//#define DEBUG_JAMIC_ID                0x0102030F
#define UNKNOWN_JAMIC_ID              0xFFFFFFFF
//debug
//OPDATA
#define VEHICLE_UDP_PORT_TROL_OPDATA  50011
#define VEHICLE_UDP_PORT_TRAM_OPDATA  13001

#define CW_READ_TROL_OPDATA           0x1000
#define CW_READ_TRAM_OPDATA           0x1000
#define CW_READ_TROL_OPDATA_NOK       0x1001
#define CW_READ_TRAM_OPDATA_NOK       0x1001


#define CW_READ_TROL_VERSIONS         0x1002
#define CW_READ_TRAM_VERSIONS         0x1002
#define CW_READ_TROL_VERSIONS_NOK     0x1003
#define CW_READ_TRAM_VERSIONS_NOK     0x1003


#define VERSION_TABLE_FIRST_VERSION_FIELD 3
#define KN_MAX_VEHICLE_IDLE_TIME_HOURS 72





#define FAIL_SOURCE_TC        0
#define FAIL_SOURCE_FC        1

#define FAIL_SOURCE_MASTER1   0
#define FAIL_SOURCE_MASTER2   1
#define FAIL_SOURCE_SLAVE     2
#define FAIL_SOURCE_JAMIC     3

#define ALARM_TYPE_UP         1
#define ALARM_TYPE_DOWN       2
#define ALARM_TYPE_CONFIRM    3

#define MYSQL_ERR_DUP_ENTRY 1062
#define MYSQL_DUPLICATE_COLUMN_NAME_ERR                1060

//common macros
#if defined(Q_OS_WIN)
  #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#elif defined(Q_OS_LINUX)
  #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
  #define __FILENAME__ (__FILE__)
#endif
#define DBGINFO  "(" << __LINE__ << "@" <<__FILENAME__ << ")"
#define TIMEINFO (QDateTime::currentDateTimeUtc().toString( Qt::ISODate))

#endif // CONST

