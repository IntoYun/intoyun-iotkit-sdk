# IntoYun 第三方设备接入SDK

## 项目介绍

![image](/docs/images/hardware-solution-third.png)

intoyun-iotkit-sdk是第三方设备SDK代码工程。

该项目是IntoYun为物联网开发者推出的开源项目，其目的是帮助开发者快速搭建与IntoYun平台安全的数据通道，方便终端与IntoYun云端的双向通道。

## 架构框图

![image](/docs/images/software-architecture-third-sdk.png)

## 目录和文件组成

```
+-- docs                : 项目文档
+-- examples            : 硬件描述层接口实现
+-- platform            : 编译烧录各个固件的上层文件夹
    +-- inc             : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- src             : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
        +-- esp32       : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
        +-- esp8266     : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
        +-- linux       : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
        +-- osx         : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
        +-- windows     : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
+-- sdk                 : 主要用于操作系统内存管理保护
    +-- cloud           : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- comm-if         : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- comm            : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- config          : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- log             : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- sdk-impl        : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- security        : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- system          : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口
    +-- utils           : 针对于特定核心板扩展的函数接口， 主要通过调用hal层接口

```

## 项目功能

`第三方设备SDK`采取分层结构，分为硬件HAL层、中间层、应用SDK层，所以项目移植到不同硬件平台非常简单，目前向用户提供了如下功能：

* 提供与IntoYun平台通讯交互接口。支持自定义和数据点数据格式。
* 提供日志调试信息。
* 提供常用功能接口。例如WiFi设备，支持HTTP、MQTT、CoAP等接口。
* 已支持桌面计算机和常用嵌入式硬件平台。例如Windows、MacOS、Linux、esp8266、esp32。


## 设备支持


## 开发指南

- [应用开发教程](docs/build.md)
- [固件库API接口](http://docs.intoyun.com/devicedev/software-develop-master/master-sdk-api)

