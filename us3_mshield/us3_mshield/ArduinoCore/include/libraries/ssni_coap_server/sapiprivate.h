/*

Copyright (c) Silver Spring Networks, Inc.
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the ""Software""), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Silver Spring Networks, Inc.
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Silver Spring
Networks, Inc.

*/

#ifndef SAPIPRIVATE_H_
#define SAPIPRIVATE_H_

#include "sapi_error.h"
#include "log.h"
#include "cbor.h"
#include "coap_server.h"
#include "coappdu.h"
#include "coapmsg.h"
#include "coapsensoruri.h"
#include "coapobserve.h"
#include "coap_rbt_msg.h"
#include "arduino_time.h"


//////////////////////////////////////////////////////////////////////////
//
// SAPI Private functions, defines and Typedefs. Don't use these in your sensor code!
//
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Sensor API CoAP Server Dispatcher.
 *
 * This function is registered for call back in the coap_registry_init function (see coapsensoruri.cpp).
 * This allows us to control our own CoAP dispatching and isolates the CoAP Server.
 *
 * @param req     Pointer to the CoAP Request. Do not modify.
 * @param rsp     Pointer to the CoAP Response. Built by the SAPI framework.
 * @return        CoAP Error Code.
 */
error_t crsapi(struct coap_msg_ctx* req, struct coap_msg_ctx* rsp);

/*
 * @brief Generic CoAP Server resource handler.
 * 
 * Code to handle SAPI CoAP requests. Dispatched by crsapi (the SAPI dispatcher).
 *
 * Note that SAPI defines a application specific API for use by CoAP Clients.
 *
 * @param req        Pointer to the CoAP Request. Do not modify.
 * @param rsp        Pointer to the CoAP Response. Built by the SAPI framework.
 * @param it         Pointer to the CoAP Request Option COAP_OPTION_URI_PATH (the leaf). Should be the same as the sensor type.
 * @param sensor_id  The Sensor Id as generated by sapi_register_sensor.
 * @return  CoAP Error Code.
 */
error_t crresourcehandler(struct coap_msg_ctx *req, struct coap_msg_ctx *rsp, void *it, uint8_t sensor_id);

/**
 * @brief Add sensor type to a CBOR payload wrapper.
 *
 * This function takes a CBOR buffer and adds the device type to map element 0.
 *
 * @param cbuf         Pointer to an initialized CBOR buffer.
 * @param sensor_type  Pointer to the sensor type.
 * @return  CoAP Error Code.
 */
uint8_t cbor_enc_nic_type(struct cbor_buf *cbuf, char *sensor_type);

/**
 * @brief Build the CoAP response message for a sensor. Called on these CoAP requests:
 *   Get sensor value
 *   Observe sensor value
 *   Get sensor config
 *
 * Payload agnostic. Wrap the payload with a CBOR Map for MQTT routing.
 * The CBOR payload is a CBOR wrapper containing the device type followed by the payload.
 *
 * A typical CBOR payload: {0:"temp",1:<text payload>"}
 *
 * @param m           Pointer to an initialized CoAP message buffer.
 * @param len         Pointer to the CoAP message buffer length.
 * @param payload     Pointer to the sensor payload buffer.
 * @param payloadlen  Length of sensor payload buffer.
 * @param sensor_id   Sensor Id.
 * @return  CoAP Error Code.
 */
error_t build_rsp_msg(struct mbuf *m, uint8_t *len, char *payload, uint32_t payloadlen, uint8_t sensor_id);

/**
 * @brief Handle generation of an observation notification. Called in two ways by the CoAP Server:
 *   Periodic generation of notifications
 *   Indirectly via sapi_push_notification
 *
 * @param m           Pointer to an initialized CoAP message buffer.
 * @param len         Pointer to the CoAP message buffer length.
 * @return  CoAP Error Code.
 */
error_t sapi_observation_handler(struct mbuf *m, uint8_t *len, uint8_t sensor_id);

/**
 * @brief Original CoAP Server Dispatcher. This is provided in the Arduino sketch example with CoAP Server
 *        Prior to version 1.4.6. We include it here for backward compatibility. Do not remove!
 *
 * @param req     Pointer to the CoAP Request. Do not modify.
 * @param rsp     Pointer to the CoAP Response. Built by the original temp sensor code.
 * @return        CoAP Error Code.
 */
error_t crarduino( struct coap_msg_ctx *req, struct coap_msg_ctx *rsp );

/**
 * @brief Helper function to print a banner in the log.
 *
 */
void sapi_log_banner();


// SAPI Version String
#define SAPI_VERSION_NUMBER "1.0.0"
#define SAPI_VERSION_STRING "Itron SAPI: "

// Payload Max Length
#define SAPI_MAX_PAYLOAD_LEN		256
#define SAPI_MAX_DEVICE_TYPE_LEN    20

// Max devices that can be registered.
#define SAPI_MAX_DEVICES			4

// CoAP Observe Max-Age, see Section 5.10.5 of rfc7252. Default of 90s.
#define COAP_MSG_MAX_AGE_IN_SECS	90


/**
 * @brief Sensor Registration Info struct
 *
 */
typedef struct sensor_reg_info
{
	char devicetype[SAPI_MAX_DEVICE_TYPE_LEN];		// Sensor Device Type
	SensorInitFuncPtr		init;					// Sensor Initialization Function
	SensorReadFuncPtr		read;					// Sensor Read Function
	SensorReadCfgFuncPtr	readcfg;				// Sensor Read cfg Function
	SensorWriteCfgFuncPtr	writecfg;				// Sensor Save cfg Function
	uint32_t				frequency;				// Observation polling frequency (seconds)
	uint8_t					observer;				// 1 -> observer
	uint8_t					observer_id;			// Observer Id (valid only if observer == 1)
} sensor_reg_info_t;



#ifdef SAML21
#define SER_MON_PTR					&SerialUSB
#define UART_PTR        			&Serial2

#else

// For Boards that use a Atmel SAMD chip but are not the ADAFRUIT_METRO_M0_EXPRESS board
#ifndef ADAFRUIT_METRO_M0_EXPRESS
// For Boards that use other Atmel SAMD chip family
#if defined(ARDUINO_ARCH_SAMD)
#define SER_MON_PTR					&SerialUSB
#define UART_PTR        			&Serial1
#endif

// For Boards that use other Atmel SAM chip families
#if defined(ARDUINO_ARCH_SAM)
#define SER_MON_PTR				  	&SerialUSB
#define UART_PTR        			&Serial
#endif
#endif

// For the AdaFruit Metro M0 Express (uses the SAMD chip family).
#if defined(ADAFRUIT_METRO_M0_EXPRESS)
#define SER_MON_PTR				    &Serial
#define UART_PTR				    &Serial1
#endif

#endif

//////////////////////////////////////////////////////////////////////////
//
// Specify the baud rate for the Serial USB
//
//////////////////////////////////////////////////////////////////////////
#define SER_MON_BAUD_RATE     			115200


//////////////////////////////////////////////////////////////////////////
//
// Set the logging level (see log.h)
// Typically you will set this to LOG_INFO or LOG_DEBUG
//
//////////////////////////////////////////////////////////////////////////
#define LOG_LEVEL						LOG_DEBUG


//////////////////////////////////////////////////////////////////////////
//
// Set the UART time-out (the serial link between Arduino and MilliShield)
// Values is in milliseconds
//
//////////////////////////////////////////////////////////////////////////
#define HDLC_UART_TIMEOUT_IN_MS				2000


//////////////////////////////////////////////////////////////////////////
//
// The largest HDLC payload size. Maximum payload length in the MilliShield is 255
//
//////////////////////////////////////////////////////////////////////////
#define HDLC_MAX_PAYLOAD_LEN				(255)


//////////////////////////////////////////////////////////////////////////
//
// Local time zone relative to UTC
// Examples: Pacific: -8, Eastern: -5, London: 0, Paris: +1, Sydney: +10
//
//////////////////////////////////////////////////////////////////////////
#define LOCAL_TIME_ZONE					(-8)


#endif /* SAPIPRIVATE_H_ */