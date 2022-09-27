#include <Arduino.h>

class voltage{
    private:

        float getVoltage(void){
            float Vo = (Vdd * analogRead(adcPin))/adcMax;
            float Vs = Vo/(Ro/(Rt+Ro));
            return Vs;
        }

    public:
        u8_t adcPin;
        float Vdd = 3.300;
        float adcMax = 4096.00;
        float Ro = 10000.0;
        float Rt = 400000.0;
        float R1 = 10.0;
        float R2 = 10.0;

        float U=0;
        float I=0;
        float P=0;

        void process(bool _R1, bool _R2){
            U = getVoltage();
            float R;
            if(_R1){
                if(_R2){
                    R = R1 + R2;
                }else{
                    R=R2;
                }
            }else{
                if(_R2){
                    R=R1;
                }else{
                    R=1.0/((1.0/R1)+(1.0/R2));
                }
            }
            I = U /R;
            P = U * I;
        }
};