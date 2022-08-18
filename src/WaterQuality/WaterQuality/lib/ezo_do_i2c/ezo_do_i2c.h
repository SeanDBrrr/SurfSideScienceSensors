#ifndef EZO_DO_I2C_H
    #define EZO_DO_I2C_H
    #include <Ezo_i2c.h>
    #include <sensorbase.h>
    #include <ezo_ec_i2c.h>
    #include <ezo_rtd_i2c.h>

    class ezo_do_i2c: public sensorBase, public Ezo_board{
        public:
        bool salinity_compensation = false;
        ezo_rtd_i2c RTD_TEMP_COMPENSATION;
        bool temperature_compensation = true;
        ezo_ec_i2c EC_COMPENSATION;
        bool isECcompensated = false;
        bool isTcompensated = false;
        /**
         * @brief begin function, sensor arguments
         * 
         * @param enablePin 
         * @param oversamples 
         * @param sensorName 
         * @param unit 
         */
        ezo_do_i2c(int enablePin=13, uint8_t address=0x61, float oversamples=5, String sensorname="DISSOLVED_OXYGEN", String unit="mg/L") : Ezo_board(address, sensorname.c_str()){
            ENABLEPIN = enablePin;
            averagingSamples = oversamples;
            checkValueInRange = true;
            sensorPwrDelay = 2000;
            sampleReadDelay = 1000;
            SENSOR_ENABLE_STATE = HIGH;
            sensorName[0] = sensorname;
            units[0] = unit;
            numberOfreadings = 1;
            sensorStabilizeDelay[0] = 30000;
            sensorReadingDecimals[0] = 3;
            EXPECTED_VALUE_MIN[0] = 0.01;
            EXPECTED_VALUE_MAX[0] = 100;
            sensorName[0] = sensorname;
            i2c_address = address;

            if(enablePin != 0){
                pinMode(enablePin, OUTPUT);
                digitalWrite(enablePin, !SENSOR_ENABLE_STATE);
            }
                        
        }

        int compensateTemperature(){
            if(temperature_compensation && !isTcompensated){
                isTcompensated = true;
                RTD_TEMP_COMPENSATION.getSamples();
                float temperature = RTD_TEMP_COMPENSATION.samplesTemp[0];
                String T_compensate = "T,"+String(temperature, 2);
                send_cmd(T_compensate.c_str());
                delay(1000);
                send_cmd("T,?");
                char buf[32];
                delay(1000);
                receive_cmd(buf, 32);
                bool compensation_confirmed = String(buf).indexOf(String(temperature, 2)) > 0;
                if (!compensation_confirmed){
                    processErrorBuffer(0, "temperature compensation Fail T: "+String(temperature, 2)+" calibration response: "+String(buf));
                    return SENSOR_BASE_FAIL;
                }
            }
            return SENSOR_BASE_SUCCESS;
        }

        int compensateSalinity(){
            if(salinity_compensation && !isECcompensated){
                isECcompensated = true;
                EC_COMPENSATION.getSamples();
                float temperature = EC_COMPENSATION.samplesTemp[0];
                String EC_compensate = "T,"+String(temperature, 2);
                send_cmd(EC_compensate.c_str());
                delay(1000);
                send_cmd("S,?");
                char buf[32];
                delay(1000);
                receive_cmd(buf, 32);
                bool compensation_confirmed = String(buf).indexOf(String(temperature, 2)) > 0;
                if (!compensation_confirmed){
                    processErrorBuffer(0, "Salinity compensation Fail S: "+String(temperature, 2)+" calibration response: "+String(buf));
                    return SENSOR_BASE_FAIL;
                }
            }
            return SENSOR_BASE_SUCCESS;
        }

        /**
         * @brief read sensor implementation
         * sum the current value to the buffer
         * report sensor status
         * 
         * @param delay_ 
         * @return int
         */
        
        int readSensorImpl(float *buffer, int *sensorstatus, long delay_){
            Serial.println("here1");
            if(compensateTemperature() == SENSOR_BASE_FAIL || compensateSalinity() == SENSOR_BASE_FAIL){
                sensorstatus[0] = SENSOR_BASE_FAIL;
                buffer[0] = SENSOR_BASE_FAIL;
                return SENSOR_BASE_FAIL;
            }
            int status_ = 0;
            send_read_cmd();
            delay(delay_);
            receive_read_cmd(); 
            float val = get_last_received_reading();
            status_ = get_error() == SUCCESS? SENSOR_BASE_SUCCESS: SENSOR_BASE_FAIL;
            sensorstatus[0] = status_;
            buffer[0] = val;
            return status_;
        }

        /**
         * @brief enable sensors implmentation
         * return 1:all sensors anabled, -1: some or all sensors enable failed
         * 
         * @return int 
         */
        int enableSensorsImpl(int *sensorstatus){
            digitalWrite(ENABLEPIN, SENSOR_ENABLE_STATE);
            delay(sensorPwrDelay);
            readSensorImpl(samplesTemp, sensorstatus, sampleReadDelay);
            int status_ = sensorstatus[0];
            return status_;
        }

        int disableSensorsImpl(int *sensorstatus){
            digitalWrite(ENABLEPIN, !SENSOR_ENABLE_STATE);
            delay(sensorPwrDelay);
            readSensorImpl(samplesTemp, sensorstatus, sampleReadDelay);
            int status_ = sensorstatus[0] == SENSOR_BASE_FAIL? SENSOR_BASE_SUCCESS: SENSOR_BASE_FAIL;
            sensorstatus[0] = status_;
            return sensorstatus[0];
        }

        int calibrateSensorsImpl(int statusLed,int *sensorstatus, int buttonPin){
            String atm_calibrate = "Cal";
            String zero_calibrate = "Cal,0";
            pinMode(buttonPin, INPUT);
            pinMode(statusLed, OUTPUT);
            digitalWrite(statusLed, HIGH);
            send_cmd("Cal,clear");
            Serial.println("Calibration atmospheric calibration do: atmospheric");
            int pinState = digitalRead(buttonPin);
            while(digitalRead(buttonPin) != pinState){
                getSamples();
                Serial.println("toggle pin to begin current do value: "+String(samplesTemp[0]));
                delay(1000);
            }
            digitalWrite(statusLed, LOW);
            send_cmd(atm_calibrate.c_str());
            // digitalWrite(statusLed, HIGH);
            // Serial.println("Calibration zero calibration do: 0 ");
            // pinState = digitalRead(buttonPin);
            // while(digitalRead(buttonPin) != pinState){
            //     getSamples();
            //     Serial.println("toggle pin to begin current do value: "+String(samplesTemp[0]));
            //     delay(1000);
            // }

            delay(1000);
            send_cmd("Cal,?");
            delay(1000);
            char buf[32];
            receive_cmd(buf, 32);
            Serial.println(String(buf));
            bool isCalibrated = String(buf).indexOf("CAL,1") > 0;
            if(!isCalibrated){
                processErrorBuffer(0, "calibration Fail calibration response: "+String(buf));
                sensorstatus[0] = SENSOR_BASE_FAIL;
                return SENSOR_BASE_FAIL;
            }
            Serial.println(sensorName[0]+" do calibrated "+String(buf));
            sensorstatus[0] = SENSOR_BASE_SUCCESS;
            return SENSOR_BASE_SUCCESS;

        }
    };
#endif