/*
*  产品名称：
*  产品描述：
*  说    明： 该代码为平台自动根据产品内容生成的代码模板框架，
*             您可以在此框架下开发。此框架包括数据点的定义和读写。
*  模板版本： v1.4
*/

#include "iot_export.h"
#include "project_config.h"

#define DEVICE_ID_DEF                             "0dvo0bdoy00000000000068f"         //设备标识
#define DEVICE_SECRET_DEF                         "c08e66a8b08fd8436dac0dce9cc3bca9" //设备密钥


void eventProcess(system_event_t event, system_events_param_t param, uint8_t *data, uint32_t len)
{
    if(event == event_cloud_comm) {
        switch(param){
            case ep_cloud_comm_data:
                break;
            case ep_cloud_comm_ota:
                break;
            default:
                break;
        }
    } else if(event == event_network_status) {
        switch(param){
            case ep_network_status_disconnected:  //模组已断开路由器
                log_info("event network disconnect router\r\n");
                break;
            case ep_network_status_connected:     //模组已连接路由器
                log_info("event network connect router\r\n");
                break;
            case ep_cloud_status_disconnected:  //模组已断开平台
                log_info("event network disconnect server\r\n");
                break;
            case ep_cloud_status_connected:     //模组已连接平台
                log_info("event network connect server\r\n");
                break;
            default:
                break;
        }
    }
}

void userInit(void)
{
    IOT_OpenLog("ota");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    //初始设备信息
    System.init();
    System.setDeviceInfo(DEVICE_ID_DEF, DEVICE_SECRET_DEF, PRODUCT_ID_DEF, PRODUCT_SECRET_DEF, HARDWARE_VERSION_DEF, SOFTWARE_VERSION_DEF);
    System.setEventCallback(eventProcess);

    Cloud.connect();
}

void userHandle(void)
{

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

