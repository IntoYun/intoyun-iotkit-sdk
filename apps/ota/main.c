/*
*  产品名称：
*  产品描述：
*  说    明： 该代码为平台自动根据产品内容生成的代码模板框架，
*             您可以在此框架下开发。此框架包括数据点的定义和读写。
*  模板版本： v1.4
*/

#include "arduino.h"
#include "project_config.h"

//这些宏在project_config.h中定义
PRODUCT_ID(PRODUCT_ID_DEF)                                 //产品标识
PRODUCT_SECRET(PRODUCT_SECRET_DEF)                         //产品密钥
PRODUCT_SOFTWARE_VERSION(SOFTWARE_VERSION_DEF)     //产品软件版本号
PRODUCT_HARDWARE_VERSION(HARDWARE_VERSION_DEF)     //产品硬件版本号

//定义数据点ID
//格式：DPID_数据类型_数据点英文名. 如果英文名相同的，则在后面添加_1 _2 _3形式。
//说明：布尔型: BOOL. 数值型: INT32或者DOUBLE. 枚举型: ENUM. 字符串型: STRING. 透传型: BINARY
#define DPID_BOOL_SWITCH                      1  //布尔型     开关(可上送可下发)
#define DPID_DOUBLE_TEMPATURE                 2  //浮点型     温度(可上送可下发)
#define DPID_INT32_HUMIDITY                   3  //整型       湿度(可上送可下发)
#define DPID_ENUM_COLOR                       4  //枚举型     颜色模式(可上送可下发)
#define DPID_STRING_DISPLAY                   5  //字符串型   显示字符串(可上送可下发)
#define DPID_BINARY_LOCATION                  6  //透传型     位置(可上送可下发)

//定义数据点变量
//格式：数据类型+数据点英文名. 如果英文名相同的，则在后面添加_1 _2 _3形式。
//布尔型为bool
//枚举型为int
//整型为int32_t
//浮点型为double
//字符型为String
//透传型为uint8_t*型
//透传型长度为uint16_t型
bool dpBoolSwitch;              //开关
double dpDoubleTemperature;     //温度
int32_t dpInt32Humidity;        //湿度
int dpEnumColor;                //颜色模式
String dpStringDisplay;         //显示字符串
uint8_t *dpBinaryLocation;      //位置
uint16_t dpBinaryLocationLen;   //位置长度

uint32_t timerID;

void system_event_callback(system_event_t event, int param, uint8_t *data, uint16_t datalen)
{
    if((event == event_cloud_data) && (param == ep_cloud_data_datapoint))
    {
        /*************此处修改和添加用户控制代码*************/
        //开关
        if(RESULT_DATAPOINT_NEW == Cloud.readDatapoint(DPID_BOOL_SWITCH, dpBoolSwitch))
        {
            //用户代码
        }

        //温度
        if(RESULT_DATAPOINT_NEW == Cloud.readDatapoint(DPID_DOUBLE_TEMPATURE, dpDoubleTemperature))
        {
            //用户代码
        }

        //湿度
        if(RESULT_DATAPOINT_NEW == Cloud.readDatapoint(DPID_INT32_HUMIDITY, dpInt32Humidity))
        {
            //用户代码
        }

        // 颜色模式
        if (RESULT_DATAPOINT_NEW == Cloud.readDatapoint(DPID_ENUM_COLOR, dpEnumColor))
        {
            //用户代码
        }

        // 显示字符串
        if (RESULT_DATAPOINT_NEW == Cloud.readDatapoint(DPID_STRING_DISPLAY, dpStringDisplay))
        {
            //用户代码
        }

        // 位置信息
        if (RESULT_DATAPOINT_NEW == Cloud.readDatapoint(DPID_BINARY_LOCATION, dpBinaryLocation, dpBinaryLocationLen))
        {
            //用户代码
        }
        //"数据发送" 设置为“用户处理”时添加 此注释无需添加
        //数据上报手动处理
        Cloud.sendDatapointAll();
        /*******************************************************/
    }
}

void userInit(void)
{
    //定义数据点事件
    System.on(event_cloud_data, system_event_callback);
    //"数据发送" 设置为“用户处理”时添加 此注释无需添加
    //设置数据上报手动处理
    Cloud.datapointControl(DP_TRANSMIT_MODE_MANUAL);
    //定义产品数据点
    Cloud.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false);                            //开关
    Cloud.defineDatapointNumber(DPID_DOUBLE_TEMPATURE, DP_PERMISSION_UP_ONLY, 0, 100, 1, 0);              //温度
    Cloud.defineDatapointNumber(DPID_INT32_HUMIDITY, DP_PERMISSION_UP_ONLY, 0, 100, 0, 0);               //湿度
    Cloud.defineDatapointEnum(DPID_ENUM_COLOR, DP_PERMISSION_UP_DOWN, 1);                                 //颜色模式
    Cloud.defineDatapointString(DPID_STRING_DISPLAY, DP_PERMISSION_UP_DOWN, 255, "hello! intoyun!");      //显示字符串
    Cloud.defineDatapointBinary(DPID_BINARY_LOCATION, DP_PERMISSION_UP_DOWN, 255, "\x12\x34\x56\x78", 4); //位置信息

    /*************此处修改和添加用户初始化代码**************/
    timerID = timerGetId();

    /*******************************************************/
}

void userHandle(void)
{
    /*************此处修改和添加用户处理代码****************/
    if(timerIsEnd(timerID, 3000))  //处理间隔  用户可自行更改
    {
        timerID = timerGetId();
        //更新数据点数据（数据点具备：上送属性）
        Cloud.writeDatapoint(DPID_BOOL_SWITCH, dpBoolSwitch);
        Cloud.writeDatapoint(DPID_DOUBLE_TEMPATURE, dpDoubleTemperature);
        Cloud.writeDatapoint(DPID_INT32_HUMIDITY, dpInt32Humidity);
        Cloud.writeDatapoint(DPID_ENUM_COLOR, dpEnumColor);
        Cloud.writeDatapoint(DPID_STRING_DISPLAY, "hello! intoyun!");
        Cloud.writeDatapoint(DPID_BINARY_LOCATION, "\x12\x34\x56\x78", 4);
        //"数据发送" 设置为“用户处理”时添加 此注释无需添加
        //数据上报手动处理
        Cloud.sendDatapointAll();
    }
    /*******************************************************/
}

void setup()
{
    userInit();
}

void loop()
{
    //loop不能阻塞
    userHandle();
}

