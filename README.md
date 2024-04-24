AVANCES Y ANOTACIONES DIA A DIA: 
  https://docs.google.com/document/d/1bW7psUk2b6T97GLkWwQHPA3Kji6ypLHm2-XdxxwD0p8/edit 

** SITUACIÓN ACTUAL **
24 ABRIL 2024
Avances para reunión Avances K oto TIEMPO INVERTIDO 16 HORAS
Investigación adicional sobre envió de datos en Pluto SDR 
8 HORAS 
  Recibiendo información.
  https://wiki.analog.com/university/tools/pluto/controlling_the_transceiver_and_transferring_data 
  Primera prueba de transmisión recepción, aplicaciones standalone
  https://github.com/analogdevicesinc/libiio/blob/libiio-v0/examples/ad9361-iiostream.c 
  Guia transmisión y recepción de archivo IQ 
  https://github.com/ulysse71/plutosdr 
  Transmisión y recepción datos IQ  PYTHON, SOLO TRANSMISIÓN
  https://github.com/naseem119/Wireless-ADALM-Pluto/tree/main 
  AD9361 iiostream sin hacer nada con los datos de recepción REVISAR
  https://github.com/analogdevicesinc/libiio/blob/main/examples/ad9361-iiostream.c 
  
    Links de información adicional: 	
    Ejemplos GNU RADIO para pluto SDR de un tercero:
    https://github.com/patel999jay/plutoSDR 
    Proyecto Chino transmisión imagenes PYTHON Incompleto: 
    https://github.com/Icexbb/PlutoSDR-ImageTransmission/tree/master 
    
    Ejemplo transmisión raspberry pi y pluto SDR
    https://patel999jay.github.io/publication/live-rf-image-transmission-using-ofdm-with-rpi-and-plutosdr/  ARTICULO SIN ACCESO, DE PAGA IEEE
    Tabla para saber bandas libres de transmisión en México:
    https://cnaf.ift.org.mx/Consulta/GetFile?type=3
    
    INTERESANTE 
    Recepción datos de aviones que pasan: 
    https://github.com/antirez/dump1090/tree/master 
    
    STM32 CON IMÁGENES
    https://github.com/BartlomiejWos/Prototype-SDR-Receiver-For-NOAA-Image-Acquisition/blob/main/src%20-%20STM32%20SPI%20-%20ADC%20-%20DAC%20-%20USB/main.c 
    Pluto para transmitir video a otro dispositivo
    https://github.com/patel999jay/Pluto-DATV-test 

Prueba de colocar un dato constante de un archivo a otro en lenguaje C.
  Fuera de pluto, un dato que se va modificando en el programa. 15 MINUTOS
  
  En pluto, crear un archivo en almacenamiento de pluto y editarlo. 2 HORAS
    Pluto no permite crear archivos en su almacenamiento, solo puede trabajar con archivos IQ (Binarios) y ASCII (2 columnas).  
    Pruebas investigadas requieren que el documento se encuentre fuera del plutoSDR y se le haga el llamado desde el dispositivo de origen. 

Investigación buffer recibe solo esa información para enviar y así la recibe: https://wiki.analog.com/resources/eval/user-guides/ad-fmcomms2-ebz/software/basic_iq_datafiles?s[]=basic&s[]=iq&s[]=datafiles#example_files 

Envío de un dato que no sea tiempo de buffer y ver en terminal. 6 HORAS
Se mandan los archivos pero nuestras salidas no son constantes a pesar de ser el mismo archivo, para empezar se necesita un archivo IQ que si se comprenda que se manda, y ver el porque no son constantes las salidas en los datos de recepción. 

PRÓXIMAS ACTIVIDADES.
Envío de un dato que no sea tiempo de buffer y ver en terminal.
  Seguir comprendiendo el envió a través del buffer que esta pasando.

Leer datos de una imagen y guardarlos en un doc sin enviarlos.
  Descomponer la imagen en datos IQ o ASCII 
     Anotaciones adicionales: 
        Imagen 16x16 píxeles, favicon.ico, 318 bytes (imagen más pequeña en pluto sdr) 

Enviar datos de imagen a un doc  
  Descomponer la imagen en datos IQ o ASCII 
  Enviar el archivo
  Recibir el archivo
  Componerlo desde el archivo IQ o ASCII
    Anotaciones adicionales: 
    Imagen 16x16 píxeles, favicon.ico, 318 bytes (imagen más pequeña en pluto sdr) 
