/*
*  产品名称：
*  产品描述：
*  说    明： 该代码为平台自动根据产品内容生成的代码模板框架，
*             您可以在此框架下开发。此框架包括数据点的定义和读写。
*  模板版本： v1.4
*/

#include "iot_export.h"
#include "project_config.h"

//这些宏在project_config.h中定义
#define DEVICE_ID_DEF                             "0dvo0bdoy00000000000068f"         //设备标识
#define DEVICE_SECRET_DEF                         "c08e66a8b08fd8436dac0dce9cc3bca9" //设备密钥

#define DPID_NUMBER_SOIL_HUMIDITY                 1  //数值型            土壤湿度
#define DPID_NUMBER_AIR_HUMIDITY                  2  //数值型            空气湿度
#define DPID_NUMBER_TEMPERATURE                   3  //数值型            温度
#define DPID_NUMBER_ILLUMINATION                  4  //数值型            光照强度
#define DPID_NUMBER_CO2                           5  //数值型            二氧化碳浓度
#define DPID_ENUM_BIRDS                           6  //枚举型            鸟类危害程度
#define DPID_BOOL_SPRINKLER_SWITCH                7  //布尔型            洒水器开关

double dpDoubleSoil_humidity;                     // 土壤湿度
double dpDoubleAir_humidity;                      // 空气湿度
double dpDoubleTemperature;                       // 温度
double dpDoubleIllumination;                      // 光照强度
int dpIntCO2;                                     // 二氧化碳浓度
int dpEnumBirds;                                  // 鸟类危害程度
bool dpBoolSprinkler_switch;                      // 洒水器开关


void eventProcess(event_type_t event, uint8_t *data, uint32_t len)
{
    switch(event) {
        case EVENT_DATAPOINT:    //处理平台数据
            //光照强度
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberDouble(DPID_NUMBER_ILLUMINATION, &dpDoubleIllumination)) {
                //用户代码
                log_info("dpDoubleIllumination = %f\r\n", dpDoubleIllumination);
            }

            //洒水器开关
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointBool(DPID_BOOL_SPRINKLER_SWITCH, &dpBoolSprinkler_switch)) {
                //用户代码
                log_info("dpBoolSprinkler_switch = %d\r\n", dpBoolSprinkler_switch);
            }
            break;

        case EVENT_CUSTOM_DATA:  //接受到透传数据
            break;

        case EVENT_CON_SERVER:   //模组已连服务器
            break;

        case EVENT_DISCON_SERVER://模组已断开服务器
            break;

        default:
            break;
    }
}

void userInit(void)
{
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    //初始设备信息
    System.init();
    System.setDeviceInfo(DEVICE_ID_DEF, DEVICE_SECRET_DEF, PRODUCT_ID_DEF, PRODUCT_SECRET_DEF, HARDWARE_VERSION_DEF, SOFTWARE_VERSION_DEF, COMM_TYPE_WIFI);
    System.setEventCallback(eventProcess);

    //添加数据点定义
    Cloud.defineDatapointNumber(DPID_NUMBER_SOIL_HUMIDITY, DP_PERMISSION_UP_ONLY, 0, 100, 1, 0); //土壤湿度
    Cloud.defineDatapointNumber(DPID_NUMBER_AIR_HUMIDITY, DP_PERMISSION_UP_ONLY, 0, 100, 1, 0); //空气湿度
    Cloud.defineDatapointNumber(DPID_NUMBER_TEMPERATURE, DP_PERMISSION_UP_ONLY, -50, 50, 1, 0); //温度
    Cloud.defineDatapointNumber(DPID_NUMBER_ILLUMINATION, DP_PERMISSION_UP_DOWN, 0, 100, 1, 0); //光照强度
    Cloud.defineDatapointNumber(DPID_NUMBER_CO2, DP_PERMISSION_UP_ONLY, 0, 100, 0, 0); //二氧化碳浓度
    Cloud.defineDatapointEnum(DPID_ENUM_BIRDS, DP_PERMISSION_UP_ONLY, 0); //鸟类危害程度
    Cloud.defineDatapointBool(DPID_BOOL_SPRINKLER_SWITCH, DP_PERMISSION_UP_DOWN, false); //洒水器开关

    Cloud.connect();
}

void userHandle(void)
{
    if(Cloud.connected()) {
        //处理需要上送到云平台的数据
        dpDoubleSoil_humidity+=0.1;
        dpDoubleAir_humidity+=0.1;
        dpDoubleTemperature+=0.1;
        dpDoubleIllumination+=0.1;
        dpIntCO2+=1;

        Cloud.writeDatapointNumberDouble(DPID_NUMBER_SOIL_HUMIDITY, dpDoubleSoil_humidity);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_AIR_HUMIDITY, dpDoubleAir_humidity);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_TEMPERATURE, dpDoubleTemperature);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_ILLUMINATION, dpDoubleIllumination);
        Cloud.writeDatapointNumberInt32(DPID_NUMBER_CO2, dpIntCO2);
        Cloud.writeDatapointEnum(DPID_ENUM_BIRDS, dpEnumBirds);
        Cloud.writeDatapointBool(DPID_BOOL_SPRINKLER_SWITCH, dpBoolSprinkler_switch);
        delay(20000);
    }
}

int main(void)
{
    userInit();
    while(1) {
        userHandle();
        System.loop();
    }
    return 0;
}

