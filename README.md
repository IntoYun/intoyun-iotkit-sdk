# IntoYun第三方设备SDK

## 项目介绍

![image](/docs/images/hardware-solution-third.png)

IntoYun平台是一个开放的物联网平台，允许第三方设备接入并使用云服务。

为了帮助开发者快速搭建与IntoYun平台安全的数据通道，方便设备与IntoYun云端的双向通道，我们推出了IntoYun第三方设备SDK项目。

## 架构框图

第三方设备SDK采取分层设计，分为硬件HAL层、中间层、应用SDK层。通过分层，SDK可以很方便地移植到不同硬件平台。

![image](/docs/images/software-architecture-third-sdk.png)

目前SDK向用户提供了如下功能：

* 提供与IntoYun平台通讯交互接口。
* 提供日志调试信息。
* 提供常用功能接口。例如WiFi设备，支持HTTP、MQTT、CoAP等接口。

## 目录和文件组成

```
+-- docs                : 项目文档
+-- examples            : 项目示例
+-- platform            : 硬件HAL层
    +-- inc             : 硬件HAL层头文件
    +-- src             : 硬件HAL层实现
        +-- esp32       : ESP32 HAL层实现
        +-- esp8266     : ESP8266 HAL层实现
        +-- linux       : Linux HAL层实现
        +-- osx         : MacOS HAL层实现
        +-- windows     : Windows HAL层实现
+-- sdk                 : SDK代码实现
    +-- cloud           : 云端应用层实现
        +-- datapoint   : 数据点功能
        +-- otaupdate   : OTA固件升级
    +-- comm-if         : 通讯协议中间层
    +-- comm            : 通讯层协议
        +-- coap        : CoAP协议实现
        +-- mqtt        : MQTT协议实现
    +-- config          : SDK配置文件
    +-- log             : LOG日志
    +-- sdk-impl        : SDK接口
    +-- security        : 通讯安全层
    +-- system          : 系统接口层
    +-- utils           : 工具层

```

## 设备支持

|  硬件平台  |  HAL层实现           |
| ---------- | -------------------- |
| linux      | platform/src/linux   |
| macos      | platform/src/osx     |
| windows    | platform/src/windows |
| esp32      | platform/src/esp32   |
| esp8266    | platform/src/esp8266 |

## 开发指南

- [SDK项目实例](https://github.com/IntoYun/intoyun-iotkit-demo)
- [固件库API接口](http://docs.intoyun.com/devicedev/software-develop-third/sdk-api/)

