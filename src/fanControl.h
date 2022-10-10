#include <Arduino.h>

class fanControl{
    private:
        float TempMax;


    public:
        uint16_t Speed = 0;

        void Speeede(float T1, float T2){
            if(T1>T2){
                TempMax = T1;
            }else{
                TempMax = T2;
            }

            if(Speed > 0){
                if(TempMax < 45){
                    Speed = 0;
                }else{
                    Speed = 20 + (TempMax - 45);
                }
            }else{
                if(TempMax > 55){
                    Speed = 20 + (TempMax - 45);
                }
            }
            
        }

        int getPwmSpeed(){
            if (Speed > 100.00){
                Speed = 100;
            }
            return Speed * 2.55;
        }

};