// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


// This sample is an alternative version to Serializer's devicetwin_simplesample.
// Here we show how to consume and handle the same data in that sample using common json parsing tools, without requiring the Serializer client.

// WARNING: Check the return of all API calls when developing your solution. Return checks ommited for sample simplification.

#include <stdio.h>
#include <stdlib.h>

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "iothub_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "parson.h"

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
    #include "iothubtransportmqtt.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    #include "iothubtransportmqtt_websockets.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    #include "iothubtransportamqp.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    #include "iothubtransportamqp_websockets.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    #include "iothubtransporthttp.h"
#endif // SAMPLE_HTTP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

/* Paste in the your iothub connection string  */
static const char* connectionString = "[device connection string]";

static bool g_continueRunning;
#define DOWORK_LOOP_NUM     3

typedef struct MAKER_TAG
{
	char* makerName;
	char* style;
	int year;
} Maker;

typedef struct GEO_TAG
{
	double longitude;
	double latitude;
} Geo;

typedef struct CAR_STATE_TAG
{
	int32_t softwareVersion;
	uint8_t reported_maxSpeed;
	char* vanityPlate;
} CarState;

typedef struct CAR_SETTINGS_TAG
{
	uint8_t desired_maxSpeed;
	Geo location;
} CarSettings;

typedef struct CAR_TAG
{
	char* lastOilChangeDate;
	char* changeOilReminder;
	Maker maker;
	CarState state;
	CarSettings settings;
} Car;

static char* serializeToJson(Car* car)
{
	char* result;



	return result;
}

static Car* parseFromJson(const char* json)
{
	Car* car = malloc(sizeof(Car));



	return car;
}

static const char* deviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
	char* result;

	if (strcmp("getCarVIN", method_name) == 0)
	{
		result = malloc(sizeof(22));
		(void)sprintf(result, "{ 1HGCM82633A004352 }");
	}
	else
	{
		// All other entries are ignored.
		result = malloc(sizeof(4));
		(void)sprintf(result, "{ }");
	}

	return result;
}

static void deviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback)
{
    (void)userContextCallback;

	// desired_maxSpeed -> print when changed.

    printf("Device Twin update received (state=%s, size=%zu): %s\r\n", 
        ENUM_TO_STRING(DEVICE_TWIN_UPDATE_STATE, update_state), size, payLoad);
}

static void reportedStateCallback(int status_code, void* userContextCallback)
{
    (void)userContextCallback;
    printf("Device Twin reported properties update completed with result: %d\r\n", status_code);
}


void serializer_to_iothub_client_migration_run(void)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
    IOTHUB_CLIENT_HANDLE iotHubClientHandle;
    g_continueRunning = true;

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    protocol = MQTT_WebSocket_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    protocol = AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    protocol = AMQP_Protocol_over_WebSocketsTls;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    protocol = HTTP_Protocol;
#endif // SAMPLE_HTTP

    if (platform_init() != 0)
    {
        (void)printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, protocol)) == NULL)
        {
            (void)printf("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            bool traceOn = true;
            (void)IoTHubClient_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn);

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
            // For mbed add the certificate information
            if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
            {
                (void)printf("failure to set option \"TrustedCerts\"\r\n");
            }
#endif // SET_TRUSTED_CERT_IN_SAMPLES

			Car car;
			car.lastOilChangeDate = "2016";
			car.maker.makerName = "Fabrikam";
			car.maker.style = "sedan";
			car.maker.year = 2014;
			car.state.reported_maxSpeed = 100;
			car.state.softwareVersion = 1;
			car.state.vanityPlate = "1I1";

			char* serializedCar = serializeToJson(&car);

			(void)IoTHubClient_SendReportedState(iotHubClientHandle, serializedCar, strlen(serializedCar), reportedStateCallback, NULL);
			(void)IoTHubClient_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
            (void)IoTHubClient_SetDeviceTwinCallback(iotHubClientHandle, deviceTwinCallback, &car);

            do
            {
                ThreadAPI_Sleep(100);
            } while (g_continueRunning);

            IoTHubClient_LL_Destroy(iotHubClientHandle);
			free(serializedCar);
		}

        platform_deinit();
    }
}

int main(void)
{
    serializer_to_iothub_client_migration_run();
    return 0;
}
