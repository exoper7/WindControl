#include <Arduino.h>

class voltage{
    private:
        float tmp8=0;
        
        float getVoltage(void){
            s16_t ADC_value = analogRead(adcPin)-offset;
            if(ADC_value<0){
                return 0.0;
            }
            float Vo = (Vdd * (ADC_value))/adcMax;
            float Vs = Vo/(Ro/(Rt+Ro));
            return Vs;
        }

        float calculateAvgVolt(float voltActual,uint8_t n){
        if((voltActual>((tmp8/n)+10))||(voltActual<((tmp8/n)-10))){
            tmp8=voltActual*n;
        }else{
            tmp8 += voltActual;
            voltActual = tmp8 / (n+1);
            tmp8 -= voltActual;
        }
        return voltActual;
    }

    public:
        u16_t offset = 0;
        u8_t adcPin;
        float Vdd = 3.300;
        float adcMax = 4096.00;
        float Ro = 10000.0;
        float Rt = 300000.0;
        float R1 = 10.0;
        float R2 = 10.0;

        float U=0;
        float I=0;
        float P=0;

        void process(bool _R1, bool _R2){
            U = calculateAvgVolt(getVoltage(),32);
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
            I = U / R;
            P = U * I;
        }

        void setOffset(){
            u32_t offsetSum=0;
            for(u8_t loop = 0;loop < 10; loop++){
                offsetSum = offsetSum + analogRead(adcPin);
                delay(5);
            }
            offset = offsetSum/10;
        }
};