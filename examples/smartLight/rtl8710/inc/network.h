/*
 * Copyright (c) 2013-2018 Molmc Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stddef.h>
#include "esp_wifi.h"

typedef void (* wifi_state_cb_t)();
typedef void (* wifi_disco_cb_t)(uint8_t reason);

void set_on_station_first_connect(wifi_state_cb_t cb);
void set_on_station_connect(wifi_state_cb_t cb);
void set_on_station_disconnect(wifi_disco_cb_t cb);
void set_on_client_connect(wifi_state_cb_t cb);
void set_on_client_disconnect(wifi_state_cb_t cb);

WIFI_MODE init_esp_wifi();
bool start_wifi_station(const char * ssid, const char * pass);
bool stop_wifi_station();
bool start_wifi_ap(const char * ssid, const char * pass);
bool stop_wifi_ap();

bool wifi_station_connected();
bool wifi_ap_enabled();

#endif /* _WIFI_STATE_MACHINE_H_ */

