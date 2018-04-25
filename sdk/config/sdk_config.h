#ifndef __SDK_CONFIG_H__
#define __SDK_CONFIG_H__

/*
 * sdk 配置文件
 */

#define CONFIG_CLOUD_CHANNEL               1   //选择云端通讯通道  1: MQTT  2: COAP
#define CONFIG_CLOUD_DATAPOINT_ENABLED     0   //是否使能数据点通讯接口

// 数据点相关配置
#define CONFIG_PROPERTIES_MAX              50  //支持的数据点的最大个数
#define CONFIG_AUTOMATIC_INTERVAL          600 //数据点自动发送默认时间间隔

// 云端通讯安全模式
// 通道 + 认证 + 数据组织加密方式
// 1. TCP + Guider + formCrypto
// 2. TCP + Guider + formPlain
// 3. TLS + Guider + Plain
#define CONFIG_CLOUD_SECURE_MODE           1   //云端通讯安全模式

#define CONFIG_MQTT_ENABLED                1   //是否使能MQTT功能
#define CONFIG_COAP_ENABLED                1   //是否使能COAP功能
#define CONFIG_SYSTEM_KEY_ENABLE           0   //是否按键接口功能
#define CONFIG_SYSTEM_TIMER_ENABLE         0   //是否定时器接口功能
#define CONFIG_LOG_ENABLE                  1   //是否使能日志功能

#endif
