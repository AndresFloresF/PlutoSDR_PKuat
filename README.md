# Adalm Pluto SDR - Proyecto KOTO 
### Empleado en: Banda S, Banda UHF, Intercomunicación con computadora.
[AVANCES Y ANOTACIONES DIA A DIA](https://docs.google.com/document/d/1bW7psUk2b6T97GLkWwQHPA3Kji6ypLHm2-XdxxwD0p8/edit) 

[CARPETA DOCUMENTOS DRIVE Y MINUTAS REUNIONES](https://docs.google.com/document/d/1bW7psUk2b6T97GLkWwQHPA3Kji6ypLHm2-XdxxwD0p8/edit) 
## Banda S  
### 1era vertiente
Se tiene entendido que la participación del Pluto SDR deberá de poder enviar y recibir imágenes mediante código C o C ++ haciendo su codificación y decodificación, dejando de lado la interfaz GNU Radio.

**Status Actual**

Ya se cuenta con un código de transmisión y recepción de documentos IQ, se debe de probar que envie y reciba de forma correcta el documento.
Queda pendiente realizar la codificación y decodificación de la imágen en archivos IQ y viceversa.

**Actividades desarrolladas**
- [x] Proceso de instalación herramientas de trabajo para Pluto SDR (Ubuntu, Vivado)//7 Feb -15 Feb 2024, reinstalado Agosto 2024 PC UAT
- [x] Instalación GNU Radio con elementos de Pluto SDR *21 Feb - 20 Marzo 2024*, reinstalado Agosto 2024 PC UAT

**Se solicita dejar de lado GNU Radio para desarrollo de puro código C desde cero** *5 Abril 2024*

- [x] Primera aplicación StandAlone *5 Abril - 9 Abril 2024*
- [x] Solicitud de antenas para buscar recibir algo e intentar guardarlo *15 Mayo - 2 Julio 2024*
- [x] Prueba de generación de archivos en PlutoSDR *23 Abril - 15 Mayo 2024*
- [x] Revisión de Tesis Lizeth Sanchez *1 Mayo 2024 - 27 Mayo 2024*

**En Proceso**
- [ ] Generar documento de preguntas Tesis Lizeth para reunión 31 Julio 2024 - ? 2024 (sin fecha de reunión todavía pero urgen ya las dudas por escrito). 
- [ ] **Enviar** un dato o documento que no se información del buffer y **recibirlo** correctamente *23 Abril - ? 2024*

**Planteadas a futuro**
- [ ] 

### 2da vertiente 
Pluto SDR se encuentra dentro de los sistemas del nanosatélite KOTO, la transmisión de la imágen se realiza desde ahí y deberá de poder recibirse en el modem satelital adquirido por el Dr Alberto, en donde se deberá de poder decodificar la información y mostrar la imágen.

**Status Actual**

Se han realizado pruebas de transmisión de información al modem satelital, en donde se puede visualizar en el espectro que llega la información, sin embargo no se ha logrado acceder de forma apropiada al contenido de esta recepción.

(Trabajo Pendiente con el modem). 

**Actividades desarrolladas**
- [x] Instalación GNU Radio con elementos de Pluto SDR *21 Feb - 20 Marzo 2024, reinstalado Agosto 2024 PC UAT*
- [x] Envió de código C alcanzado a la fecha en 1er vertiente para ver que al menos llegue algo en espectro  *22 Agosto 2024*
      
**En Proceso**
- [x] Recreación de bloques GNU Radio Tesis Lizeth Sanchez para confirmar envió correcto de información e intentar decodificar en Modem Satelital. *22 Agosto 2024 Parte en Pluto SDR lograda por el momento.*
      
**Planteadas a futuro**

Observar que va demandando el modem satelital acorde a su avance y si esta vertiente termina siendo la buena... 

## Banda UHF
Aún se está definiendo cuál es el lugar del Pluto SDR en esta banda, sin embargo parece acercarse a la intercomunicación.

**Status Actual**

Se está trabajando en ello para arrancar

**Actividades desarrolladas**
- [x] Revisión de repositorio anterior y generar dudas mas anotaciones para próxima reunión. *27 Agosto -4 Septiembre 2024*

**En proceso**
- [ ] En espera de próxima reunión o respuestas de comentarios generados. 4 Sep 2024 - ? 2024 

**Planteadas a futuro**
- [ ] .
## Intercomunicación 
Pluto SDR deberá de comunicarse mediante un HUB de puertos USB, y colocando el código dentro de una memoria, deberá de hacer las interacciones SPI e I2C con otros módulos, para poder llevar algoritmos dentro del mismo.

**Status Actual**

Se ha logrado que el Pluto SDR desarrolle códigos desde una memoria USB.
Se empieza a realizar pruebas de intentar comunicar al módulo SPI 

**Actividades desarrolladas**
- [x] Intento de ejecutar códigos ADCS en Pluto SDR (solo una prueba de un código) *5 Agosto 2024 - 20 Agosto 2024*
- [x] Primeras pruebas de PlutoSDR arrancando con códigos en puerto USB. (USB OTG) *19 Agosto - 20 Agosto 2024*
      
**En proceso**
- [ ] Periodo de investigación para conocer como conectar entre módulos mencionados. *19 Agosto - ? 2024*

**Planteadas a futuro**
- [ ] Primera prueba con módulos ( no se donde están 4 Sep 2024)


## Comenzando
Instrucciones para iniciar a trabajar en el Proyecto KOTO. 

### Pre-requisitos 📋
Computadora del satélite y medio para trabajar con Pluto SDR
**Sistema Operativo:** Ubuntu 18.04.6 LTS (Bionic Beaver)
Se deberá realizar su instalación: 
Enlace recomendado[Septiembre 2024]
```
[Ubuntu 18.04.6](https://releases.ubuntu.com/18.04.6/)
```
Para trabajar con Pluto SDR es recomendable acorde a lo que se ha ido haciendo:
**GNU Radio** Se recomienda versión 3.8 (Versión compatible con Ubuntu 18.04.6)  


**Compilador:** Vivado Versión 2019.1.3 
Solo si se quiere usar este compilador, pluto SDR ha funcionado compilando código con el de Linux(Linaro toolchain).
[Información para compilación de códigos](https://wiki.analog.com/university/tools/pluto/devs/embedded_code)

Procedimiento recomendado para instalar Vivado [Septiembre 2024]:

Descarga: [Compilador Vivado Xilinx](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive.html)

Instalación
```
chmod +x [fileName]
sudo ./[filename]
```
Verificación (a 2018.2 es la prueba realizada, pero sin problema se puede cambiar a 2019.1) 
```
source /opt/Xilinx/Vivado/2018.2/settings64.sh
arm-linux-gnueabihf-gcc --version
```
## Enlaces de interés para el desarrollo
### Generales 
[Última actualización Septiembre 2024]
- [Recibiendo información.](https://wiki.analog.com/university/tools/pluto/controlling_the_transceiver_and_transferring_data) 
- [Primera prueba de transmisión recepción, aplicaciones standalone](https://github.com/analogdevicesinc/libiio/blob/libiio-v0/examples/ad9361-iiostream.c) 

### Actividades Banda S 
[Última actualización Septiembre 2024]

**Referencias base** 
- [Guia transmisión y recepción de archivo IQ](https://github.com/ulysse71/plutosdr) 
- [Transmisión  datos IQ  PYTHON](https://github.com/naseem119/Wireless-ADALM-Pluto/tree/main)
- [AD9361 iiostream sin hacer nada con los datos de recepción](https://github.com/analogdevicesinc/libiio/blob/main/examples/ad9361-iiostream.c)  REVISAR SEP 2024
  
**Adicionales interesantes**
  - [Ejemplos GNU RADIO para pluto SDR de un tercero](https://github.com/patel999jay/plutoSDR)
  - [Proyecto Chino transmisión imagenes PYTHON Incompleto]( https://github.com/Icexbb/PlutoSDR-ImageTransmission/tree/master) 
  - [Ejemplo transmisión raspberry pi y pluto SDR](https://patel999jay.github.io/publication/live-rf-image-transmission-using-ofdm-with-rpi-and-plutosdr/)  **ARTÍCULO SIN ACCESO, DE PAGA IEEE**
  - [Tabla para saber bandas libres de transmisión en México](https://cnaf.ift.org.mx/Consulta/GetFile?type=3)

 **INTERESANTE** 
    - [Recepción datos de aviones que pasan](https://github.com/antirez/dump1090/tree/master) 
    - [STM32 CON IMÁGENES](https://github.com/BartlomiejWos/Prototype-SDR-Receiver-For-NOAA-Image-Acquisition/blob/main/src%20-%20STM32%20SPI%20-%20ADC%20-%20DAC%20-%20USB/main.c)
    - [Pluto para transmitir video a otro dispositivo](https://github.com/patel999jay/Pluto-DATV-test) 

### Actividades Banda UHF
[Última actualización Septiembre 2024]
- .

### Intercomunicación entre SPI e I2C
[Última actualización Septiembre 2024]
- .

## Participantes ✒️
_Menciona a todos aquellos que ayudaron a levantar el proyecto desde sus inicios_
### Líderes del proyecto
* **Dr. Jose Alberto Ramirez** - *Subsistema Telecomunicaciones*
* **Dr. Rafael Chavez Moreno** - *Líder de proyecto*

### ¿ Desarrolladores ?
* **Ing. Lizeth Sanchez** - *Trabajo Inicial*
* **FALTA AGREGAR ANTERIORES**
* **Ing. Giomar Martínez** - Banda UHF
* **Ing. Cesar**- Banda UHF
* **Ing. Andres Flores** - *Banda S - Envió y transmisión Imágen*
* **Ing. Isac Valencia** - *Banda UHF - Circuito*
* **Ing. Carlos Castillo** - *Banda S - Modem Satelital*
* **Ing. Erick Gomez Nieto** - *Banda S - Codificación y Decodificación*

También puedes mirar la lista de todos los [contribuyentes](https://github.com/your/project/contributors) quienes han participado en este proyecto.
