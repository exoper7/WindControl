#include <Arduino.h>

class tempSensor{
  private:
    float tmp8;
    float nowTemp;
    
    float getTemperature(float Rt){
        //int RawADC = analogRead(adcPin);
        float Tmp;
        Tmp = 1.00 / (invT0 + invBeta*(log (Rt/R0)));
        Tmp = Tmp - 273.15; // Convert Kelvin to Celcius
        return Tmp; 
    }

    float calculateAvgTemp(float TempActual,uint8_t n){
        if((TempActual>((tmp8/n)+3))||(TempActual<((tmp8/n)-3))){
            tmp8=TempActual*n;
        }else{
            tmp8 += TempActual;
            TempActual = tmp8 / (n+1);
            tmp8 -= TempActual;
        }
        return TempActual;
    }

    float GetResistance(){
        int RawADC = analogRead(adcPin);
        float Res;
        Res = Rref/((adcMax / (float) RawADC )- 1.00);
        return Res; 
    }

  public:
    //termistor parameters
    float invBeta = 1.00 / 3950.00;
    float adcMax = 4096.00;
    float invT0 = 1.00 / 298.15;
    unsigned int Rref=9930;
    double R0=10000;
    //ADC Pin connected to temp sensor
    u8_t adcPin;
    float avgTemp;
    float Resistance;

    //Returns temperature reading
    float Temp(void){
        Resistance = GetResistance();
        return getTemperature(Resistance);
    }

    //Returns avg temperature reading
    float GetAvgTemp(void){
        Resistance = GetResistance();
        nowTemp = getTemperature(Resistance);
        avgTemp = calculateAvgTemp(nowTemp,16);
        return avgTemp;
    } 



    int GetADC(void){
        return analogRead(adcPin);
    }

};