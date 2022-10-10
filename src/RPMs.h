#include <Arduino.h>

class RPMs{
    private:
        uint64_t lastTime;
    public:
        float RPM = 0;
        float period;
        float Hz = 0;
        uint8_t poles = 1; 

        //Callback for rpm pin
        void processRPM(void){
            uint64_t timeNow = time_us_64();
            period = timeNow - lastTime;
            lastTime = timeNow;
            Hz = 1/(period/1000000.0);
            RPM = Hz*60.0/poles;
        }

        //checks for 0 rpm 
        void update(void){
            uint64_t timeout = (60000000/poles) + lastTime;
            if (time_us_64() > timeout){
                RPM = 0;
                period = 0;
                Hz = 0;
            }
        }

};