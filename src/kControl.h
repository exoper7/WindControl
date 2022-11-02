#include <Arduino.h>

#define overVolt 1;
#define overTemp 2;


class kControl{
    private:
        unsigned long millisLastChange;
        bool maxLoad;
        bool minLoad;

        //reduce load
        void reduce(void){
            millisLastChange =  millis();
            if(K1 && K2 && K3){
                minLoad = true;
                return;
            }
            if(K1 && K2){
                K3 = true;
                currState = 3;
                return;
            }
            if(K1){
                K2 = true;
                currState = 2;
            }else{
                K1 = true;
                maxLoad = false;
                currState = 1;
            }
        }

        //advance load
        void advance(void){
            millisLastChange =  millis();
            if(!K1){
                maxLoad = true;
                return;
            }
            if(K1 && K2 && K3){
                K3 = false;
                minLoad = false;
                currState = 2;
                return;
            }
            if(K1 && K2){
                K2 = false;
                currState = 1;
            }else{
                K1 = false;
                currState = 0;
            }
        }

        bool readyForChange(u16_t timeToChange){
            if(millis()<30000){
                return false;
            }
            millisFromLastChange =  millis() - millisLastChange;
            return (millisFromLastChange > timeToChange);
        }

    public:
        bool K1;
        bool K2;
        bool K3;
        bool Alarm;
        u8_t AlarmCode;
        unsigned long millisFromLastChange;
        bool OverTemperature;
        u8_t currState = 0;

        void process(float Voltage, float T1, float T2){
            if((T1>90.0)||(T2>90.0)){
                OverTemperature = true;
                Alarm = true;
                AlarmCode = overTemp;
                K1 = false;
                K2 = false;
                K3 = false;
                return;
            }
            
            if(OverTemperature){
                if((T1<70.0)&&(T2<70.0)){
                    OverTemperature = false;
                    Alarm = false;
                    AlarmCode = 0;
                }
            }

            if(Voltage > 55.0){
                if(maxLoad){
                    Alarm = true;
                    AlarmCode = overVolt;
                }else{
                    Alarm = false;
                    AlarmCode = 0;
                    if(readyForChange(1000)){
                        advance();
                    }
                }
            }else if(Voltage < 35.0){
                if(!minLoad){
                    if(readyForChange(30000)){
                        reduce();
                    }
                }
            }
            readyForChange(1000);
        }

};