#ifndef PMS_SS_H
#define PMS_SS_H
#include <sensorbase.h>
#include <Arduino.h>
#include "PMS.h"
#include "SoftwareSerial.h"
SoftwareSerial serial;
SoftwareSerial serial2;

class PMS_SS : public sensorBase
{
public:
    PMS *pms;
    PMS::DATA pms_data;
    void begin(int rxPin, int txPin, int enablepin, String sensorname[], String unit[], int numberOfSamples = 10, long sampleRead_delay = 50, int decimals = 0, int index = 0)
    {
        ENABLEPIN = enablepin;
        averagingSamples = numberOfSamples;
        checkValueInRange = true;
        sampleReadDelay = sampleReadDelay;
        SENSOR_ENABLE_STATE = HIGH;
        sensorPwrDelay = 500;
        numberOfreadings = 3;
        if (index == 0)
        {
            serial.begin(9600, SWSERIAL_8N1, rxPin, txPin, false, 192);

            PMS pms_(serial);
            pms = &pms_;
        }
        else
        {
            serial2.begin(9600, SWSERIAL_8N1, rxPin, txPin, false, 192);

            PMS pms_(serial2);
            pms = &pms_;
        }
        for (int i = 0; i < numberOfreadings; i++)
        {
            sensorName[i] = sensorname[i];
            units[i] = unit[i];
            sensorStabilizeDelay[i] = 30000;
            sensorReadingDecimals[i] = decimals;
            EXPECTED_VALUE_MIN[i] = 0;
            EXPECTED_VALUE_MAX[i] = 1000000;
            pinMode(ENABLEPIN, OUTPUT);
        }
        if (ENABLEPIN != 0)
        {
            pinMode(ENABLEPIN, OUTPUT);
            digitalWrite(ENABLEPIN, HIGH);
        }
    }

    int readSensorImpl(float *buffer, int *sensorstatus, long delay_)
    {
        pms->requestRead();
        if (pms->readUntil(pms_data, 2000))
        {
            buffer[0] = pms_data.PM_SP_UG_1_0;
            buffer[1] = pms_data.PM_SP_UG_2_5;
            buffer[2] = pms_data.PM_SP_UG_10_0;
            sensorStatus[0] = SENSOR_BASE_SUCCESS;
            sensorStatus[1] = SENSOR_BASE_SUCCESS;
            sensorStatus[2] = SENSOR_BASE_SUCCESS;
            return SENSOR_BASE_SUCCESS;
        }
        else
        {
            sensorStatus[0] = SENSOR_BASE_FAIL;
            sensorStatus[1] = SENSOR_BASE_FAIL;
            sensorStatus[2] = SENSOR_BASE_FAIL;
            return SENSOR_BASE_FAIL;
        }
    }

    int enableSensorsImpl(int *sensorstatus)
    {
        digitalWrite(ENABLEPIN, SENSOR_ENABLE_STATE);
        delay(sensorPwrDelay);
        pms->activeMode();
        pms->wakeUp();
        int status_ = readSensorImpl(samplesBufferTemp, sensorstatus, 0);
        for (int i = 0; i < numberOfreadings; i++)
        {
            sensorstatus[i] = status_;
        }
        return status_;
    }
    int disableSensorsImpl(int *sensorstatus)
    {
        digitalWrite(ENABLEPIN, !SENSOR_ENABLE_STATE);
        delay(sensorPwrDelay);
        int status_ = readSensorImpl(samplesBufferTemp, sensorstatus, 0) == SENSOR_BASE_FAIL ? SENSOR_BASE_SUCCESS : SENSOR_BASE_FAIL;
        for (int i = 0; i < numberOfreadings; i++)
        {
            sensorstatus[i] = status_;
        }
        return status_;
    }

    int calibrateSensorsImpl(int statusLed, int *sensorstatus)
    {
        for (int i = 0; i < numberOfreadings; i++)
        {
            sensorstatus[i] = SENSOR_BASE_SUCCESS;
        }
        return SENSOR_BASE_SUCCESS;
    }
};

#endif