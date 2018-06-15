#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS += platform/inc sdk/cloud/datapoint sdk/cloud/otaupdate sdk/comm-if sdk/comm/coap sdk/comm/mqtt sdk/config sdk/log sdk/sdk-impl sdk/sdk-impl/exports sdk/sdk-impl/imports sdk/security sdk/system sdk/utils/digest sdk/utils/misc sdk/utils/cJSON

COMPONENT_SRCDIRS += platform/src/esp8266 sdk/cloud/datapoint sdk/cloud/otaupdate sdk/comm-if sdk/comm/coap sdk/comm/coap/CoAPPacket sdk/comm/mqtt sdk/comm/mqtt/MQTTPacket sdk/log sdk/sdk-impl sdk/security sdk/system sdk/utils/digest sdk/utils/misc sdk/utils/cJSON

CFLAGS +=


