#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


extern store_flash Flash_array;

uint8_t Zone(uint16_t pista)
{
  uint8_t zone;
  zone=255;
  
  if (Tipo_dispositivo==0x10)
  {
    //if(data_flash[28]==0x00)  // 6200 Ohm s
    if(Flash_array.Type_resistor==0x00)  // 6200 Ohm s
    {
      
      if (pista>=976 && pista<=1024) zone=1;
      if (pista>=928 && pista<976) zone=2;
      if (pista>=877 && pista<928) zone=3;
      if (pista>=829 && pista<877) zone=4;
      if (pista>=792 && pista<829) zone=5;
      if (pista>=750 && pista<792) zone=6;
      if (pista>=703 && pista<750) zone=7;
      if (pista>=658 && pista<703) zone=8;
      if (pista>=613 && pista<658) zone=9;
      if (pista>=574 && pista<613) zone=10;
      if (pista>=528 && pista<574) zone=11;
      if (pista>=250 && pista<528) zone=12;
      if (pista<250) zone=255;
    }
    //if(data_flash[28]==0x01)  // 6800 Ohms
    if(Flash_array.Type_resistor==0x01)  // 6800 Ohms
    {
      if (pista>=974 && pista<=1024) zone=1;
      if (pista>=929 && pista<974) zone=2;
      if (pista>=886 && pista<929) zone=3;
      if (pista>=844 && pista<886) zone=4;
      if (pista>=805 && pista<844) zone=5;
      if (pista>=765 && pista<805) zone=6;
      if (pista>=725 && pista<765) zone=7;
      if (pista>=684 && pista<725) zone=8;
      if (pista>=640 && pista<684) zone=9;
      if (pista>=599 && pista<640) zone=10;
      if (pista>=557 && pista<599) zone=11;
      if (pista>=267 && pista<557) zone=12;
      if (pista<267) zone=255;
    }
  }
  
  //SENSOR 250MM = 
  if ((Tipo_dispositivo==0x11) ||(Tipo_dispositivo==0x12))
  {
    //if(data_flash[28]==0x00)  // 6200 Ohm s
    if(Flash_array.Type_resistor==0x00)  // 6200 Ohm s
    {
      
      if (pista>=966 && pista<=1024) zone=1;
      if (pista>=918 && pista<966) zone=2;
      if (pista>=874 && pista<918) zone=3;
      if (pista>=832 && pista<874) zone=4;
      if (pista>=793 && pista<832) zone=5;
      if (pista>=756 && pista<793) zone=6;
      if (pista>=718 && pista<756) zone=7;
      if (pista>=677 && pista<718) zone=8;
      if (pista>=637 && pista<677) zone=9;
      if (pista>=597 && pista<637) zone=10;
      if (pista>=554 && pista<597) zone=11;
      if (pista>=507 && pista<554) zone=12;
      if (pista>=462 && pista<507) zone=13;
      if (pista>=422 && pista<462) zone=14;
      if (pista>=376 && pista<422) zone=15;
      if (pista>=323 && pista<376) zone=16;
      if (pista>=146 && pista<323) zone=17;
      if (pista<146) zone=255;
      
    }
    //if(data_flash[28]==0x01)  // 6800 Ohms
    if(data_flash[28]==0x01)  // 6800 Ohms
    {
      if (pista>=966 && pista<=1024) zone=1;
      if (pista>=918 && pista<966) zone=2;
      if (pista>=874 && pista<918) zone=3;
      if (pista>=832 && pista<874) zone=4;
      if (pista>=793 && pista<832) zone=5;
      if (pista>=756 && pista<793) zone=6;
      if (pista>=718 && pista<756) zone=7;
      if (pista>=677 && pista<718) zone=8;
      if (pista>=637 && pista<677) zone=9;
      if (pista>=597 && pista<637) zone=10;
      if (pista>=554 && pista<597) zone=11;
      if (pista>=507 && pista<554) zone=12;
      if (pista>=462 && pista<507) zone=13;
      if (pista>=422 && pista<462) zone=14;
      if (pista>=376 && pista<422) zone=15;
      if (pista>=323 && pista<376) zone=16;
      if (pista>=146 && pista<323) zone=17;
      if (pista<146) zone=255;
    }
  }
  
  // SENSOR 70.2 = 1.3 X 54
  
  if (Tipo_dispositivo==0x13)
  {
    
    //if(data_flash[28]==0x00)  // 6200 Ohm s
    if(Flash_array.Type_resistor==0x00)  // 6200 Ohm s
    {
      
      if (pista>=976 && pista<1024)zone=1;
      if (pista>=938 && pista<976) zone=2;
      if (pista>=901 && pista<938) zone=3;
      if (pista>=864 && pista<901) zone=4;
      if (pista>=827 && pista<864) zone=5;
      if (pista>=789 && pista<827) zone=6;
      if (pista>=753 && pista<789) zone=7;
      if (pista>=715 && pista<753) zone=8;
      if (pista>=678 && pista<715) zone=9;
      if (pista>=643 && pista<678) zone=10;
      if (pista>=606 && pista<643) zone=11;
      if (pista>=567 && pista<606) zone=12;
      if (pista>=532 && pista<567) zone=13;
      if (pista>=501 && pista<532) zone=14;
      if (pista>=466 && pista<501) zone=15;
      if (pista>=426 && pista<466) zone=16;
      if (pista>=390 && pista<426) zone=17;
      if (pista>=358 && pista<390) zone=18;
      if (pista>=324 && pista<358) zone=19;
      if (pista>=291 && pista<324) zone=20;
      if (pista>=260 && pista<291) zone=21;
      if (pista>=227 && pista<260) zone=22;
      if (pista>=191 && pista<227) zone=23;
      if (pista>=157 && pista<191) zone=24;
      if (pista>=126 && pista<157) zone=25;
      if (pista>= 92 && pista<126) zone=26;
      if (pista>= 56 && pista<92)  zone=27;
      if (pista>= 30 && pista<56)  zone=28;
      if (pista<30) zone=255;
    }
    //if(data_flash[28]==0x01)  // 6800 Ohms
    if(Flash_array.Type_resistor==0x01)  // 6800 Ohms
    {
      if (pista>=976 && pista<1024) zone=1;
      if (pista>=938 && pista<976) zone=2;
      if (pista>=901 && pista<938) zone=3;
      if (pista>=864 && pista<901) zone=4;
      if (pista>=827 && pista<864) zone=5;
      if (pista>=789 && pista<827) zone=6;
      if (pista>=753 && pista<789) zone=7;
      if (pista>=715 && pista<753) zone=8;
      if (pista>=678 && pista<715) zone=9;
      if (pista>=643 && pista<678) zone=10;
      if (pista>=606 && pista<643) zone=11;
      if (pista>=567 && pista<606) zone=12;
      if (pista>=532 && pista<567) zone=13;
      if (pista>=501 && pista<532) zone=14;
      if (pista>=466 && pista<501) zone=15;
      if (pista>=426 && pista<466) zone=16;
      if (pista>=390 && pista<426) zone=17;
      if (pista>=358 && pista<390) zone=18;
      if (pista>=324 && pista<358) zone=19;
      if (pista>=291 && pista<324) zone=20;
      if (pista>=260 && pista<291) zone=21;
      if (pista>=227 && pista<260) zone=22;
      if (pista>=191 && pista<227) zone=23;
      if (pista>=157 && pista<191) zone=24;
      if (pista>=126 && pista<157) zone=25;
      if (pista>= 92 && pista<126) zone=26;
      if (pista>= 56 && pista<92)  zone=27;
      if (pista>= 30 && pista<56)  zone=28;
      if (pista<30) zone=255;
      
      
    }
  }
  return zone;
}


float Sensor_Analisys(uint16_t Pista1,uint16_t Pista2)
{
  uint8_t Zone1,Zone2;
  Resistor_Cuted=0xFD;
  
  Zone1 = Zone(Pista1);
  Zone2 = Zone(Pista2);
  
  
  //NRF_LOG_RAW_INFO("Tipo dispositivo %x\r\n",Tipo_dispositivo);
  //NRF_LOG_FLUSH();
  
  Sensor_connected = true;
  if (Tipo_dispositivo==0x10)
  {
    if (Zone1==1 && Zone2==1) Resistor_Cuted = 0;
    if (Zone1==2 && Zone2==1) Resistor_Cuted = 1;
    if (Zone1==3 && Zone2==1) Resistor_Cuted = 2;
    if (Zone1==4 && Zone2==1) Resistor_Cuted = 3;
    if (Zone1==5 && Zone2==1) Resistor_Cuted = 4;
    if (Zone1==6 && Zone2==1) Resistor_Cuted = 5;
    if (Zone1==7 && Zone2==1) Resistor_Cuted = 6;
    if (Zone1==8 && Zone2==1) Resistor_Cuted = 7;
    if (Zone1==9 && Zone2==1) Resistor_Cuted = 8;
    if (Zone1==10 && Zone2==1) Resistor_Cuted = 9;
    if (Zone1==11 && Zone2==1) Resistor_Cuted = 10;
    if (Zone1==12 && Zone2==1) Resistor_Cuted = 11;
    if (Zone1==12 && Zone2==2) Resistor_Cuted = 12;
    if (Zone1==12 && Zone2==3) Resistor_Cuted = 13;
    if (Zone1==12 && Zone2==4) Resistor_Cuted = 14;
    if (Zone1==12 && Zone2==5) Resistor_Cuted = 15;
    if (Zone1==12 && Zone2==6) Resistor_Cuted = 16;
    if (Zone1==12 && Zone2==7) Resistor_Cuted = 17;
    if (Zone1==12 && Zone2==8) Resistor_Cuted = 18;
    if (Zone1==12 && Zone2==9) Resistor_Cuted = 19;
    if (Zone1==12 && Zone2==10) Resistor_Cuted = 20;
    if (Zone1==12 && Zone2==11) Resistor_Cuted = 21;
    if (Zone1==12 && Zone2==12) Resistor_Cuted = 22;
    
    
    
    Lenght_Cut = ((float)Resistor_Cuted )* ((float)Step_of_resistor);
    
    if (Zone1==255 || Zone2==255) 
    {Sensor_connected = false;
    Lenght_Cut = 255;
    }
    NRF_LOG_RAW_INFO("Zona 1 %i  y Zona 2 %i\r\n",Zone1,Zone2);
    NRF_LOG_FLUSH();
    
  }
  
  //SENSOR 70.2 = 1.3 X 54
  if (Tipo_dispositivo==0x13)
  {
    if (Zone1==1 && Zone2==1) Resistor_Cuted = 0;
    if (Zone1==2 && Zone2==1) Resistor_Cuted = 1;
    if (Zone1==3 && Zone2==1) Resistor_Cuted = 2;
    if (Zone1==4 && Zone2==1) Resistor_Cuted = 3;
    if (Zone1==5 && Zone2==1) Resistor_Cuted = 4;
    if (Zone1==6 && Zone2==1) Resistor_Cuted = 5;
    if (Zone1==7 && Zone2==1) Resistor_Cuted = 6;
    if (Zone1==8 && Zone2==1) Resistor_Cuted = 7;
    if (Zone1==9 && Zone2==1) Resistor_Cuted = 8;
    if (Zone1==10 && Zone2==1) Resistor_Cuted = 9;
    if (Zone1==11 && Zone2==1) Resistor_Cuted = 10;
    if (Zone1==12 && Zone2==1) Resistor_Cuted = 11;
    if (Zone1==13 && Zone2==1) Resistor_Cuted = 12;
    if (Zone1==14 && Zone2==1) Resistor_Cuted = 13;
    if (Zone1==15 && Zone2==1) Resistor_Cuted = 14;
    if (Zone1==16 && Zone2==1) Resistor_Cuted = 15;
    if (Zone1==17 && Zone2==1) Resistor_Cuted = 16;
    if (Zone1==18 && Zone2==1) Resistor_Cuted = 17;
    if (Zone1==19 && Zone2==1) Resistor_Cuted = 18;
    if (Zone1==20 && Zone2==1) Resistor_Cuted = 19;
    if (Zone1==21 && Zone2==1) Resistor_Cuted = 20;
    if (Zone1==22 && Zone2==1) Resistor_Cuted = 21;
    if (Zone1==23 && Zone2==1) Resistor_Cuted = 22;
    if (Zone1==24 && Zone2==1) Resistor_Cuted = 23;
    if (Zone1==25 && Zone2==1) Resistor_Cuted = 24;
    if (Zone1==26 && Zone2==1) Resistor_Cuted = 25;
    if (Zone1==27 && Zone2==1) Resistor_Cuted = 26;
    if (Zone1==28 && Zone2==1) Resistor_Cuted = 27;
    
    if (Zone1==28 && Zone2==2) Resistor_Cuted = 28;
    if (Zone1==28 && Zone2==3) Resistor_Cuted = 29;
    if (Zone1==28 && Zone2==4) Resistor_Cuted = 30;
    if (Zone1==28 && Zone2==5) Resistor_Cuted = 31;
    if (Zone1==28 && Zone2==6) Resistor_Cuted = 32;
    if (Zone1==28 && Zone2==7) Resistor_Cuted = 33;
    if (Zone1==28 && Zone2==8) Resistor_Cuted = 34;
    if (Zone1==28 && Zone2==9) Resistor_Cuted = 35;
    if (Zone1==28 && Zone2==10) Resistor_Cuted = 36;
    if (Zone1==28 && Zone2==11) Resistor_Cuted = 37;
    if (Zone1==28 && Zone2==12) Resistor_Cuted = 38;
    if (Zone1==28 && Zone2==13) Resistor_Cuted = 39;
    if (Zone1==28 && Zone2==14) Resistor_Cuted = 40;
    if (Zone1==28 && Zone2==15) Resistor_Cuted = 41;
    if (Zone1==28 && Zone2==16) Resistor_Cuted = 42;
    if (Zone1==28 && Zone2==17) Resistor_Cuted = 43;
    if (Zone1==28 && Zone2==18) Resistor_Cuted = 44;
    if (Zone1==28 && Zone2==19) Resistor_Cuted = 45;
    if (Zone1==28 && Zone2==20) Resistor_Cuted = 46;
    if (Zone1==28 && Zone2==21) Resistor_Cuted = 47;
    if (Zone1==28 && Zone2==22) Resistor_Cuted = 48;
    if (Zone1==28 && Zone2==23) Resistor_Cuted = 49;
    if (Zone1==28 && Zone2==24) Resistor_Cuted = 50;
    if (Zone1==28 && Zone2==25) Resistor_Cuted = 51;
    if (Zone1==28 && Zone2==26) Resistor_Cuted = 52;
    if (Zone1==28 && Zone2==27) Resistor_Cuted = 53;
    if (Zone1==28 && Zone2==28) Resistor_Cuted = 54;
    
    Resistor_Cuted=Resistor_Cuted;
    NRF_LOG_RAW_INFO("Zona 1 %i  y Zona 2 %i\r\n",Zone1,Zone2);
    NRF_LOG_FLUSH();
    
    
    Lenght_Cut = ((float)Resistor_Cuted)* ((float)Step_of_resistor);
    if (Zone1==255 || Zone2==255) 
    {Sensor_connected = false;
    Lenght_Cut = 255;
    }
  }
  
  // SENSOR 250MM
  if ((Tipo_dispositivo==0x11) ||(Tipo_dispositivo==0x12))
  {
    if (Zone1==1 && Zone2==1) Resistor_Cuted = 0;
    if (Zone1==2 && Zone2==1) Resistor_Cuted = 1;
    if (Zone1==3 && Zone2==1) Resistor_Cuted = 2;
    if (Zone1==4 && Zone2==1) Resistor_Cuted = 3;
    if (Zone1==5 && Zone2==1) Resistor_Cuted = 4;
    if (Zone1==6 && Zone2==1) Resistor_Cuted = 5;
    if (Zone1==7 && Zone2==1) Resistor_Cuted = 6;
    if (Zone1==8 && Zone2==1) Resistor_Cuted = 7;
    if (Zone1==9 && Zone2==1) Resistor_Cuted = 8;
    if (Zone1==10 && Zone2==1) Resistor_Cuted = 9;
    if (Zone1==11 && Zone2==1) Resistor_Cuted = 10;
    if (Zone1==12 && Zone2==1) Resistor_Cuted = 11;
    if (Zone1==13 && Zone2==1) Resistor_Cuted = 12;
    if (Zone1==14 && Zone2==1) Resistor_Cuted = 13;
    if (Zone1==15 && Zone2==1) Resistor_Cuted = 14;
    if (Zone1==16 && Zone2==1) Resistor_Cuted = 15;
    if (Zone1==17 && Zone2==1) Resistor_Cuted = 16;
    if (Zone1==17 && Zone2==2) Resistor_Cuted = 17;
    if (Zone1==17 && Zone2==3) Resistor_Cuted = 18;
    if (Zone1==17 && Zone2==4) Resistor_Cuted = 19;
    if (Zone1==17 && Zone2==5) Resistor_Cuted = 20;
    if (Zone1==17 && Zone2==6) Resistor_Cuted = 21;
    if (Zone1==17 && Zone2==7) Resistor_Cuted = 22;
    if (Zone1==17 && Zone2==8) Resistor_Cuted = 23;
    if (Zone1==17 && Zone2==9) Resistor_Cuted = 24;
    if (Zone1==17 && Zone2==10) Resistor_Cuted = 25;
    if (Zone1==17 && Zone2==11) Resistor_Cuted = 26;
    if (Zone1==17 && Zone2==12) Resistor_Cuted = 27;
    if (Zone1==17 && Zone2==13) Resistor_Cuted = 28;
    if (Zone1==17 && Zone2==14) Resistor_Cuted = 29;
    if (Zone1==17 && Zone2==15) Resistor_Cuted = 30;
    if (Zone1==17 && Zone2==16) Resistor_Cuted = 31;    
    if (Zone1==17 && Zone2==17) Resistor_Cuted = 32;
    
    
    
    
    if (Tipo_dispositivo==0x12) {Resistor_Cuted = Resistor_Cuted-8;}
    
    Lenght_Cut = ((float)Resistor_Cuted )* ((float)Step_of_resistor);
    
    if (Zone1==255 || Zone2==255) 
    {Sensor_connected = false;
    Lenght_Cut = 0xff;
    }
    
  }
  
  return  Lenght_Cut;
}


