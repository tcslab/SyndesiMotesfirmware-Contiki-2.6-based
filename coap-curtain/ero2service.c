/*
 * Copyright (c) 2012, Sensor Nets Laboratory, University of Geneva.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *      Common service implementation
 * \authors
 *      Kasun Samarasinghe <Kasun.Wijesiriwardana@unige.ch>
 *			Orestis Evangelatos <Orestis.Evangelatos@unige.ch>
 *
 *
 *		Services supported by the Curtain sensor: eg: curtain?status=up
 *		status=up				> roll up curtain
 *		status=down			> roll down curtain
 *		status=stepup		> roll up curtain for 1 Second
 *		status=stepdown	> roll down curtain for 1 Second
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "erbium.h"

#include "dev/leds.h"

#if WITH_COAP == 12
#include "er-coap-12.h"
#elif WITH_COAP == 7
#include "er-coap-07.h"
#else
#endif 

#include "ero2manager.h"
#include "ero2service.h"
#include "ero2registry.h"

#include "telosb-actuator.h"
//#include "dev/light-sensor.h"


int flag=0;
struct ero2_service curtain_service;
static struct etimer et;
char time;
char timeset;

PROCESS(timer_process_up,"Timer Process UP");
PROCESS_THREAD(timer_process_up, ev, data)
{
	PROCESS_BEGIN();

	leds_on(LEDS_GREEN);
	time = data;
	ActuatorON(1);
	//wait for 23 seconds, till curtain up, then switch of the actuators
	etimer_set(&et, CLOCK_SECOND * time);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	ActuatorOFF(1);
	leds_off(LEDS_GREEN);
  PROCESS_END();
}


PROCESS(timer_process_down,"Timer Process Down");
PROCESS_THREAD(timer_process_down, ev, data)
{
	PROCESS_BEGIN();
	leds_on(LEDS_RED);
	ActuatorON(2);
	time = data;
	//wait for 23 seconds, till curtain down, then switch of the actuators
	etimer_set(&et, CLOCK_SECOND * time);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	ActuatorOFF(2);
	leds_off(LEDS_RED);
  PROCESS_END();
}


RESOURCE(curtain, METHOD_GET, "curtain", "title=\"curtain?status=up/down..\";rt=\"Text\"");
void curtain_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const char *len = NULL;
  char message[8];
	const char *status = NULL;


	//printf("I am printing and toggling\n");
	leds_toggle(LEDS_BLUE);
	SetActuator(1);
	SetActuator(2);

	//Light value
	//SENSORS_ACTIVATE(light_sensor);
	//int LightValue = 10* light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) /7;
	
	if ((len=REST.get_query_variable(request, "status", &status))) 
		{
		  //PRINTF("status %.*s\n", len, status);


		  if (strncmp(status, "up", len)==0) {
				//flag=1;
				//sprintf(message,"Door lock= OPEN");
				//printf("message is %s\n",message);
				memcpy(buffer, "Curtain=UP", 11);
				timeset = 24;
				process_start(&timer_process_up,timeset);
			}
		  else if (strncmp(status, "down", len)==0) {
				//printf("message is %s\n",message);
				memcpy(buffer, "Curtain=DOWN", 13);
				timeset = 22;
				process_start(&timer_process_down,timeset);
			}

			else if (strncmp(status, "stepup", len)==0) {
				//printf("message is %s\n",message);
				//memcpy(buffer, "Curtain= UP", 12);
				timeset = 1;
				process_start(&timer_process_up,timeset);
			}
			else if (strncmp(status, "stepdown", len)==0) {
				//printf("message is %s\n",message);
				//memcpy(buffer, "Curtain= DOWN", 13);
				timeset = 1;
				process_start(&timer_process_down,timeset);
			}

		else {
		memcpy(buffer, "Wrong request!", 15);
		}

	}

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); 
  REST.set_header_etag(response,10, 1);
  REST.set_response_payload(response, buffer, 16);
}  //end of 






void start_app_services()
{
	/* curtain service */
	rest_init_engine();
	rest_activate_resource(&resource_curtain);
	curtain_service.name="curtain";	
	curtain_service.uri="curtain";
	curtain_service.request_method="COAP_GET";	
	add_to_local_registry(&curtain_service);
	printf("curtain service printing");

}



