#ifndef SHT31_S_H
#define SHT31_S_H
#include <sensorbase.h>
#include <Arduino.h>
#include "SHT31.h"

/**
 * @brief SHT31 class
 * Inherits from sensorBase class and SHT31 class.
 */
class SHT31_S : public sensorBase, public SHT31
{
public:
    enum Sensors
    {
        TEMPERATURE,
        HUMIDITY
    };

    /**
     * @brief Construct a new sht31 s object.
     * 
     * @param enablepin Input enable pin.
     * @param sensorname Input sensor name.
     * @param unit Input sensor units.
     * @param numberOfSamples Input number of samples per reading.
     * @param sampleRead_delay Inout read elay per sample
     * @param decimals Input measurement decimals ammount.
     */
    SHT31_S(int enablepin, String sensorname[], String unit[], int numberOfSamples = 10, long sampleRead_delay = 50, int decimals = 2):SHT31()
    {
        ENABLEPIN = enablepin;
        averagingSamples = numberOfSamples;
        checkValueInRange = true;
        sampleReadDelay = sampleReadDelay;
        SENSOR_ENABLE_STATE = HIGH;
        sensorPwrDelay = 500;
        numberOfreadings = 2;
        sensorName[TEMPERATURE] = sensorname[TEMPERATURE];
        sensorName[HUMIDITY] = sensorname[HUMIDITY];
        units[TEMPERATURE] = unit[TEMPERATURE];
        units[HUMIDITY] = unit[HUMIDITY];
        sensorStabilizeDelay[TEMPERATURE] = sampleRead_delay;
        sensorStabilizeDelay[HUMIDITY] = sampleRead_delay;
        sensorReadingDecimals[TEMPERATURE] = decimals;
        sensorReadingDecimals[HUMIDITY] = decimals;
        EXPECTED_VALUE_MIN[TEMPERATURE] = -20;
        EXPECTED_VALUE_MIN[HUMIDITY] = 0;
        EXPECTED_VALUE_MAX[TEMPERATURE] = 60;
        EXPECTED_VALUE_MAX[HUMIDITY] = 110;
        if (ENABLEPIN != 0)
        {
            pinMode(ENABLEPIN, OUTPUT);
        }
    }

    /**
     * @brief Setting up iplementation for sensor readings.
     *
     * @param buffer Array of sensor measurements.
     * @param sensorstatus Sensor status code.
     * @param delay_ Delay.
     * @return Int Status code.
     */
    int readSensorImpl(float *buffer, int *sensorstatus, long delay_)
    {
        if (isConnected())
        {
            read();
            buffer[0] = getTemperature();
            buffer[1] = getHumidity();
            sensorStatus[0] = SENSOR_BASE_SUCCESS;
            sensorStatus[1] = SENSOR_BASE_SUCCESS;
            return SENSOR_BASE_SUCCESS;
        }
        else
        {
            sensorStatus[0] = SENSOR_BASE_FAIL;
            sensorStatus[1] = SENSOR_BASE_FAIL;
            return SENSOR_BASE_FAIL;
        }
    }

    /**
     * @brief Setting up iplementation for enabling the sensor.
     *
     * @param sensorstatus Sensor status.
     * @return int Status.
     */
    int enableSensorsImpl(int *sensorstatus)
    {
        digitalWrite(ENABLEPIN, SENSOR_ENABLE_STATE);
        delay(sensorPwrDelay);
        int status_ = readSensorImpl(samplesBufferTemp, sensorstatus, 0);
        for (int i = 0; i < numberOfreadings; i++)
        {
            sensorstatus[i] = status_;
        }
        return status_;
    }
    
    /**
     * @brief Setting up iplementation for disabling the sensor.
     *
     * @param sensorstatus Sensor status.
     * @return int Status.
     */
    int disableSensorsImpl(int *sensorstatus)
    {
        digitalWrite(ENABLEPIN, !SENSOR_ENABLE_STATE);
        delay(sensorPwrDelay);
        int status_ = readSensorImpl(samplesBufferTemp, sensorstatus, 0) == SENSOR_BASE_FAIL? SENSOR_BASE_SUCCESS : SENSOR_BASE_FAIL;
        for (int i = 0; i < numberOfreadings; i++)
        {
            sensorstatus[i] = status_;
        }
        return status_;
    }

    /**
     * @brief Setting up iplementation for calibrating the sensor.
     *
     * @param statusLed Imput status LED pin.
     * @param sensorstatus Sensor status.
     * @return int status.
     */
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