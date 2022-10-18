#include <Arduino.h>

class RPMs{
    private:
        uint64_t lastTime;
        float tmp8=0;
        float tempHz;
        

        //calculate mean from last n points
        float calculateAvg(float value,uint8_t n){
            //if((value>((tmp8/n)+10))||(value<((tmp8/n)-10))){
            tmp8 += value;
            value = tmp8 / (n+1);
            tmp8 -= value;
            return value;
        }

    public:
        float RPM = 0;
        float period;
        float Hz = 0;
        float meanHz= 0.0;
        uint8_t poles = 4; 

        //Callback for rpm pin
        void processRPM(void){
            uint64_t timeNow = time_us_64();
            period = timeNow - lastTime;
            if(period>30000){
                lastTime = timeNow;
                tempHz = 1/(period/1000000.0);
                if((tempHz>(meanHz+5))||(tempHz<(meanHz-5))){
                    return;
                }
                meanHz = calculateAvg(tempHz,6);
                Hz = tempHz;
                RPM = Hz*60.0/poles; 
            }
        }

        //checks for 0 rpm 
        void update(void){
            uint64_t timeout = (5000000) + lastTime;
            if (time_us_64() > timeout){
                RPM = 0;
                period = 0;
                Hz = 0;
                meanHz = 0;
            }
        }

};