#include <Arduino.h>
#include <EEPROM.h>


#define EEPROM_size 128
#define write_interval 21600 //6h in sec

class energy{
    private:
        union {
            float asFloat;
            uint8_t asByte[4];  
        } ve; 

        u64_t lastWrite = 0;

        void writeEnergy(float Enow){
            ve.asFloat = E;
            EEPROM.write(0,ve.asByte[0]);
            EEPROM.write(1,ve.asByte[1]);
            EEPROM.write(2,ve.asByte[2]);
            EEPROM.write(3,ve.asByte[3]);

            EEPROM.commit();
        }

    public:

    //energy in Wh
    float E = 48.0; 

    //init eeprom and energy metering
    void begin(){
        EEPROM.begin(EEPROM_size);
        //read old data from eeprom
        ve.asByte[0] = EEPROM.read(0);
        ve.asByte[1] = EEPROM.read(1);
        ve.asByte[2] = EEPROM.read(2);
        ve.asByte[3] = EEPROM.read(3);
        //set data from eeprom to energy value.
       
        if(isnan(ve.asFloat)){
            writeEnergy(0.0);
            return;
        }
        E = ve.asFloat;
    }

    //calculate energy based on avg power, call this every 500ms
    void calculateEnergy(float Pavg){
        E = E + (Pavg / 7200.00);
        
        if((millis() - lastWrite) > (write_interval * 1000) ){
            writeEnergy(E);
            lastWrite = millis();
        }
    }

};