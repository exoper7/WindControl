#include <Arduino.h>

class voltage{
    private:
        float tmp8=0;
        
        //read adc and calculate voltage
        float getVoltage(void){
            s16_t ADC_value = analogRead(adcPin)-offset;
            if(ADC_value<0){
                return 0.0;
            }
            float Vo = (Vdd * (ADC_value))/adcMax;
            float Vs = Vo/(Ro/(Rt+Ro));
            return Vs;
        }

        //returns mean voltage from n samples 
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
        //resistance values for active state
        float Rvalue[4] = {0,84.0,490.0,1940.0};

        float U=0;
        float I=0;
        float P=0;

        //read voltage and calculate current and power based on state
        void process(u8_t state){
            U = calculateAvgVolt(getVoltage(),32);

            if(state == 0){
                I = 0.0;
                P = 0.0;
                return;
            }

            I = U / Rvalue[state];;
            P = U * I;
        }

        //auto set adc offset
        void setOffset(){
            u32_t offsetSum=0;
            for(u8_t loop = 0;loop < 10; loop++){
                offsetSum = offsetSum + analogRead(adcPin);
                delay(5);
            }
            offset = offsetSum/10;
        }
};