
//06/12/2024   088  - Se cambia de lular tiempo ADV y dormido, no se cargaba dato inicial.
//28/11/2024   087  - Se cambia guardadode historia por sensibilidad a que graba una lectura por día.

// 28/11/2024  086  - Se arregla limitante de largo de historia
//                  - Se agrega indicador de sensibilidad

// 25/10/2024 0.84  Se ajusta el calculo de medianas (se mueve hacia afuera el encender ADC)
//                  

//11/10/2024 Agregan campos a la historioa y la cantidad de lineas de historias se deja fijo con un "Define."

//02-10-2024 Se corrige mac en Adcvertising.para que funcione con IOS
              //Se elimina inicio de address mac que estaba en dos partes del programa
              // Cambia a version 0.83

//12/09/2024 Se elimina el nombre SMARTW y se agrega la mac del dispositivo al final de la cadena FF (para el uso con IOS)
             //cambia a 0.82


//21/08/2024 Se corrige envio de 0x53 dos veces al enviar los datos de configuracion 
//           se agrega a la matrix de historia la mac original y la mac custom
//           se cambia a version 081


//02/07/2024 Se corrigue problema de que la escala de 

//14/06/2024 Se agrega funcionalidad de mediana y calibracion de ADC, toma 70 valores y obtiene la mediana, el ciclo anterior se repite 20 veces y se obtiene la mediana.


//08/06/2024  Se corrige envio de historial. La modificacion se realiza para el historial y la configuracion.

//26/05/2024 Se crea TAbla de historial.
//             se baja el historial con el comando 98
//             Se limpia la historia con el comando 97


// 25/05/2024  Se crean Archivos .h para modularizar el codigo y simplificar el archivo central.

// 19-04-2024 Se agrega LED 4

// 14-3-23 Correcciones de offset de perno 54 divisiones
//         - Se envia Valor de bateria el ADC completo desde 358 870 
//            - valores de Bateria        870 366 cuando tiene 4.25
//            -                           859 35B Cuando tiene 4.2V
//             -                          756 2f4 Cuadno tiene 3.7V  
//              Nivel de bateria es 256*2 es la escala comienza del valor mas alto de bateria de lipo 870 4.25V                              
//             Se agrega proteccion de tiempo de advertising y tiempo dormido. solo acepta ciertos limites.
//             Se agrega matrix de historia de cambio de desgaste.

// 24-1-23 cambio de variables de offset a int8_t

//19/11/22 Se agregara tiempo de advertising diferido con horario
//         se usara horario a de 8:00 a 19:00 horas para realizar advertising cada 3 minutos.
//         se usara horario de en otro horario emitirá cada 3 horas.

//8/3/22 se observa en emisor 10-36 que se resetea mientras esta en advertising, pero el voltaje se observa estable en 2.99 a 3.00.
//       se aumenta el tiempo entre advertising 64 a 90  (APP_ADV_INTERVAL)
//7/3/22 Se programa funcionalidad de grabacion de error

//10/2/22
//se agrega funcionalidad de offset positivo y negativo, punto medio valor 128, se maneja en mm
// se agrega version y de envio por celular.
// 

// 16/7/21
// REVISAR PORQUE SE RESETEA POR VIBRACION  Estaba el UART encendido, solo se deja UART encendido cuando esta conectado a celular
// Se agrega configuracion de mac x uart        OK
// cambio de tiempos de envio x UART            OK
// Configuracion del tipo de sensor (tipo 120 mm o Esparrago)
// configuracion del Tipo de emisor (el de 6.2 Kohm o 6.8 Kohm)
// 6/10/21 
// agrega luz encendida al conectarse con el equipo.
// se agrega comando 52 para resetear puntos de errores
// se agrega en comando 99 las lineas de donde ocurre el error
//


// 16/7/21
// REVISAR PORQUE SE RESETEA POR VIBRACION  Estaba el UART encendido, solo se deja UART encendido cuando esta conectado a celular
// Se agrega configuracion de mac x uart        OK
// cambio de tiempos de envio x UART            OK
// Configuracion del tipo de sensor (tipo 120 mm o Esparrago)
// configuracion del Tipo de emisor (el de 6.2 Kohm o 6.8 Kohm)
// 6/10/21 
// agrega luz encendida al conectarse con el equipo.
// se agrega comando 52 para resetear puntos de errores
// se agrega en comando 99 las lineas de donde ocurre el error
//


// USAR PCA10040 y S132
// Version 2 de PCB para desgaste Kupfer
// 6uA en Sleep
// Se cambia forma de inicio de luces (una a la vez)
// Se comienza prueba de 6 dispositivos con 15 seg adv y 43 seg sleep, 3 con bateria LIPO y 3 con bateria Lipocloro




//Faltantes
// 1.- programacion de tiempos a distancia                 
// 2.- Grabacion de configuracion en EPROM              OK
// 3.- Actualizacion Via celular  MAC.                  OK
// 4.- Actualizacion Via celular  Mac de Fabrica        OK   
// 3.- Actualizacion Via celular. Save Original Mac     OK            
// 3.- Actualizacion Via celular. Tiempo adv            OK
// 3.- Actualizacion Via celular. Tiempo Sleep          OK
// 3.- Actualizacion Via celular. Tipo de Sensor        OK

// WatchDOG
// Envio de mensaje distinto cuando se deja presionado el boton por mas de 2 segundos.  OK

// Al conectarse se le envian los siguientes comandos
// 01 cantidad de veces que se ha re-iniciado  01xxxxx en decimal       Comando 01
// 02 modifica tiempo de dormido 02xxxxx   x en segundos                Comando 02      OK
// 03 modifica tiempo de ADV     03xxxxx   x en segundos                Comando 03      OK
// 04 Carga un contador de advertising 04xxxxx                          Comando 04      
// 05 Reset del equipo                                                  Comando 05      OK
// 06 grabamos Fecha y hora formato "060YYYY.MM.DD HH.MM.SS"            
// 07 Leed Hora nodo                                                    
// 20 modifica la MAC 20AABBCCDDEEFF                                    Comando 20      OK
// 21 Comienza a usar Mac de fabrica                                    Comando 21      OK
// 22 Comienza a Usar Mac Cargada                                       Comando 22      OK
// 30 Modifica el tipo de Sensor 10, 11 , 12 o 13 ejemplo 3010          Comando 30      Ok
// 3010 Perno     de 120 mm Espaciamiento 2 mm Total 22 divisiones
// 3011 Esparrago de 250 mm Espaciamiento 6 mm total 32 Divisiones
// 3012 Esparrago de 200 mm Espaciamiento 6 mm total 23 Divisiones        
// 3013 Esparrago de 110 mm Espaciamiento 1.3 mm total 54 Divisiones  


// 31 Modifica tipo de resistencia 6200 o 6800  Standard 6200 (3100)    Comando 31      OK
// 40 Modifica tipo de Bateria 4000 Tipo 3.6V y 4001 Tipo 4.2 Recargable        (standard 4000 3.6V)
// 50 Modifica el offset de la primera division, cuando el perno esta mas adentro que el borde de la placa
//    por defecto es Cero, 50xx  en milimetros 
// 51 
// 52 GENERA RESET DE LAS LINEAS DONDE SE RESETEA EL PROGRAMA