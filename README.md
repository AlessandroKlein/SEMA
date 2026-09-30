# SEMA

# Sistema de Estación Meteorológica Autónoma

Sistema modular de adquisición, procesamiento, almacenamiento, visualización y comunicación de variables meteorológicas y ambientales basado principalmente en ESP32.

> **Principio fundamental:**
> **El hardware define las capacidades. La configuración web define cómo se utilizan.**

---

# 1. Descripción general

**SEMA (Sistema de Estación Meteorológica Autónoma)** es una plataforma modular diseñada para construir estaciones meteorológicas y ambientales de diferentes niveles de complejidad utilizando un mismo firmware y una arquitectura configurable.

El sistema deberá poder utilizarse tanto en:

* estaciones meteorológicas domésticas;
* estaciones rurales;
* agricultura;
* monitoreo ambiental;
* invernaderos;
* estaciones solares;
* monitoreo de calidad del aire;
* monitoreo de suelo;
* estaciones industriales;
* sistemas IoT;
* redes de estaciones distribuidas;
* estaciones autónomas alimentadas mediante panel solar y batería;
* nodos remotos LoRa;
* nodos Zigbee;
* redes RS485/Modbus;
* redes CAN Bus;
* estaciones conectadas a un servidor central.

La característica principal será la **configuración completa mediante interfaz web**.

El usuario no deberá tener que modificar el firmware para cambiar:

* qué sensores están instalados;
* qué modelo de sensor se utiliza;
* qué GPIO utiliza un sensor;
* qué bus utiliza;
* qué dirección I²C tiene;
* qué canal de un expansor utiliza;
* qué canal ADC utiliza;
* qué sensor representa cada entrada;
* qué unidades utiliza;
* parámetros de calibración;
* intervalos de lectura;
* filtros;
* promedios;
* límites de alarma;
* almacenamiento;
* comunicaciones;
* funciones opcionales;
* módulos habilitados.

El objetivo es que un mismo firmware pueda utilizarse en instalaciones muy diferentes.

---

# 2. Filosofía del proyecto

SEMA seguirá el principio general establecido en el Design System de AlessandroKlein:

```text
SIMPLE POR FUERA
MODULAR POR DENTRO
```

La arquitectura deberá permitir:

```text
Agregar capacidad
      ↓
Registrar módulo
      ↓
Detectar hardware
      ↓
Configurar
      ↓
Habilitar
      ↓
Utilizar
```

y evitar:

```text
Cambiar firmware
      ↓
Cambiar código
      ↓
Recompilar
      ↓
Reinstalar
      ↓
Modificar interfaz
```

cuando la modificación pueda resolverse mediante configuración.

---

# 3. Objetivos

SEMA tendrá como objetivos principales:

1. Medir variables meteorológicas.
2. Medir variables ambientales.
3. Medir variables eléctricas.
4. Medir variables relacionadas con suelo cuando sea necesario.
5. Registrar históricos.
6. Mostrar información en tiempo real.
7. Permitir funcionamiento completamente autónomo.
8. Permitir funcionamiento sin Internet.
9. Disponer de una interfaz web local.
10. Disponer de API.
11. Permitir comunicación con servidores centrales.
12. Permitir MQTT.
13. Permitir LoRa.
14. Permitir Zigbee.
15. Permitir RS485/Modbus.
16. Permitir CAN Bus.
17. Permitir Ethernet cuando el hardware lo soporte.
18. Permitir Wi-Fi.
19. Permitir almacenamiento local.
20. Permitir actualización OTA.
21. Permitir configuración completa desde la web.
22. Permitir utilizar diferentes modelos de sensores para una misma magnitud.
23. Permitir agregar sensores nuevos sin rediseñar el sistema completo.
24. Permitir deshabilitar funcionalidades que no sean utilizadas.
25. Permitir detectar errores de sensores.
26. Permitir detectar pérdida de comunicación.
27. Permitir diagnosticar buses y expansores.
28. Permitir monitorear batería y panel solar.
29. Permitir funcionamiento con energía solar.
30. Permitir integrar múltiples estaciones en una misma red.

---

# 4. Principio fundamental de arquitectura

SEMA deberá separar cuatro conceptos.

## 4.1 Hardware disponible

Es todo aquello que el firmware es capaz de soportar.

Ejemplo:

```text
ESP32
I²C
SPI
UART
ADC
1-Wire
CAN
RS485
74HC595
74HC165
MCP23017
ADS1115
ADS1015
LoRa
Zigbee
Ethernet
SD
```

---

## 4.2 Hardware instalado

Es el hardware que físicamente existe en una estación determinada.

Ejemplo:

```text
ESP32-S3

I²C
 ├── BME280
 ├── BH1750
 └── SCD41

SPI
 └── AS3935

UART
 └── PMS5003

RS485
 └── Sensor de suelo Modbus

LoRa
 └── SX1276

74HC165
 └── Entradas digitales

ADS1115
 └── Piranómetro
```

---

## 4.3 Sensores habilitados

Son los sensores que el usuario decidió utilizar.

Por ejemplo:

```text
Temperatura       ✓
Humedad           ✓
Presión           ✓
CO₂               ✓
Luz               ✓
UV                ✗
PM2.5             ✗
Rayos             ✓
Radiación solar   ✓
Suelo             ✗
```

---

## 4.4 Funciones

Las funciones utilizan las mediciones disponibles.

Ejemplo:

```text
Temperatura
    ↓
Registro histórico

Temperatura + Humedad
    ↓
Punto de rocío

Temperatura + Presión
    ↓
Tendencia atmosférica

Radiación solar
    ↓
Energía solar estimada

Viento + Dirección + Lluvia
    ↓
Datos meteorológicos

Batería + Panel solar
    ↓
Estado energético
```

La lógica funcional no deberá depender innecesariamente del modelo físico del sensor.

---

# 5. Arquitectura general

```text
                         SERVIDOR CENTRAL
                                │
                   HTTPS / MQTT / API / etc.
                                │
              ┌─────────────────┼─────────────────┐
              │                 │                 │
           SEMA #1           SEMA #2           SEMA #N
              │                 │                 │
       ┌──────┴──────┐   ┌──────┴──────┐   ┌──────┴──────┐
       │             │   │             │   │             │
    Sensores      Buses Sensores     Buses Sensores    Buses
       │             │   │             │   │             │
       └─────────────┴───┴─────────────┴───┴─────────────┘
```

Cada estación deberá poder funcionar independientemente.

Si el servidor central desaparece:

```text
Servidor ❌
     │
     X
     │
   SEMA
     │
     ├── Sensores
     ├── Registro
     ├── Alarmas
     ├── Automatización
     ├── Energía
     └── Comunicaciones disponibles
```

La estación deberá continuar adquiriendo y almacenando datos siempre que disponga de energía y hardware funcional.

---

# 6. Microcontrolador

El controlador principal será un ESP32.

La arquitectura deberá intentar mantener compatibilidad con diferentes generaciones y variantes:

* ESP32-WROOM;
* ESP32 DevKit;
* ESP32 DOIT DevKit V1;
* ESP32-S3;
* ESP32-C3;
* ESP32-C5;
* ESP32-C6;
* otras variantes compatibles cuando los periféricos necesarios estén disponibles.

El firmware deberá abstraer las diferencias de hardware siempre que sea razonablemente posible.

La selección de la placa deberá formar parte de la configuración de compilación, pero la configuración funcional de la estación deberá realizarse desde la web.

---

# 7. Arquitectura de sensores

Los sensores no deberán estar programados como una lista rígida del tipo:

```cpp
if (sensor == SHT31)
```

La arquitectura deberá utilizar abstracciones.

Ejemplo:

```text
TemperatureHumiditySensor
        │
        ├── AHT10
        ├── AHT20
        ├── AHT21
        ├── AHT30
        ├── DHT22
        ├── SHT31
        ├── SHT40
        ├── BME280
        ├── SHT3x
        ├── SHT4x
        └── Sensor Modbus
```

La aplicación deberá trabajar con:

```text
TEMPERATURE
HUMIDITY
```

y no con:

```text
SHT31
```

como concepto funcional.

---

# 8. Catálogo inicial de sensores

SEMA deberá comenzar con una base de sensores suficientemente amplia para permitir diferentes configuraciones desde el principio.

El catálogo deberá ser ampliable.

---

# 9. Temperatura

Se deberán contemplar como mínimo:

```text
DS18B20
AHT10
AHT20
AHT21
AHT30
DHT22
SHT31
SHT40
SHT3x
SHT4x
BME280
BMP280
```

También deberá existir la posibilidad futura de utilizar:

```text
PT100
PT1000
NTC
Sensores Modbus
Sensores industriales
```

cuando exista el hardware de acondicionamiento correspondiente.

---

# 10. Humedad relativa

Se deberán contemplar:

```text
AHT10
AHT20
AHT21
AHT30
DHT22
SHT31
SHT40
SHT3x
SHT4x
BME280
```

El usuario deberá seleccionar desde la web:

```text
Magnitud:
Humedad relativa

Sensor:
SHT40

Interfaz:
I²C

Dirección:
0x44
```

---

# 11. Presión atmosférica

Sensores iniciales:

```text
BMP280
BME280
```

Arquitectura futura:

```text
BMP3xx
BME68x
Sensores industriales
Sensores Modbus
```

La interfaz deberá mostrar:

```text
Presión:
1013.4 hPa
```

y permitir seleccionar:

* hPa;
* Pa;
* kPa;
* mbar;
* inHg;
* otras unidades compatibles.

---

# 12. DS18B20

El DS18B20 utilizará 1-Wire.

Ejemplo:

```text
GPIO
 │
 ├── DS18B20 #1
 ├── DS18B20 #2
 ├── DS18B20 #3
 └── DS18B20 #N
```

Configuración típica:

```text
DATA ─── 4.7 kΩ ─── 3.3 V
```

La web deberá permitir:

```text
Bus 1-Wire #1

GPIO:
13

Pull-up:
4.7 kΩ

Sensores detectados:
  28-XXXXXXXXXXXX
  28-YYYYYYYYYYYY

Sensor 1:
Nombre: Temperatura exterior

Sensor 2:
Nombre: Temperatura suelo
```

Cada dispositivo deberá poder recibir un nombre lógico.

---

# 13. Sensores de luminosidad

Se deberán contemplar inicialmente:

```text
BH1750
OPT3001
```

Posteriormente:

```text
VEML7700
TSL2591
LTR390
Sensores PAR
PPFD
Piranómetros
```

El sistema deberá diferenciar:

```text
LUX
```

de:

```text
PAR
PPFD
Radiación solar
```

No se deberá asumir:

```text
Lux = PPFD
```

---

# 14. Radiación ultravioleta

Se deberán contemplar:

```text
VEML6075
LTR390
```

Magnitudes posibles:

```text
UVA
UVB
UV Index
UV total
```

La interfaz deberá indicar claramente qué magnitud proporciona realmente cada sensor.

---

# 15. CO₂ y calidad del aire

Se deberán contemplar:

```text
CCS811
SCD30
SCD40
SCD41
```

La arquitectura deberá distinguir:

```text
CO₂ estimado
```

de:

```text
CO₂ medido mediante NDIR
```

Los sensores NDIR deberán tratarse como una categoría independiente.

---

# 16. Material particulado

Se deberá incorporar inicialmente:

```text
PMS5003
```

y permitir ampliar posteriormente a:

```text
PMS7003
PMSA003
SPS30
otros sensores compatibles
```

Variables:

```text
PM1.0
PM2.5
PM10
```

cuando el sensor las proporcione.

La comunicación será normalmente mediante:

```text
UART
```

---

# 17. Descargas eléctricas atmosféricas

Se deberá contemplar:

```text
AS3935 Franklin Lightning Sensor
```

mediante:

```text
I²C
```

o:

```text
SPI
```

según el hardware utilizado.

Variables:

```text
Detección de rayo
Distancia estimada
Actividad de tormenta
Contador de eventos
```

Los datos deberán registrarse como eventos independientes.

---

# 18. Monitoreo de CO

SEMA podrá soportar sensores de CO de diferentes tecnologías.

Ejemplos:

```text
MQ-7
SPEC Sensors 3-ULPSM-CO 968-001
MICS-5524
ZE07-CO
```

El sistema deberá distinguir entre:

```text
sensor analógico
```

y:

```text
sensor digital/UART
```

cuando corresponda.

Los sensores electroquímicos y sensores de gases deberán disponer de parámetros específicos de calibración.

---

# 19. Estación de viento

Se deberán contemplar sensores meteorológicos como:

```text
WH-SP-WS01
WH-SP-WD
```

para:

```text
velocidad del viento
dirección del viento
```

La configuración eléctrica podrá incluir los componentes necesarios, por ejemplo:

### Velocidad

```text
DATA
 │
 ├── 10 kΩ → 3.3 V
 │
 └── 100 nF → GND
```

### Dirección

```text
DATA
 │
 ├── 10 kΩ → 3.3 V
 │
 └── 1 µF → GND
```

Los valores deberán documentarse como parte del perfil de hardware y no como supuestos universales para cualquier sensor.

---

# 20. Pluviómetro

Se deberá soportar:

```text
WH-SP-RG
```

y otros pluviómetros de pulsos.

La arquitectura deberá trabajar con:

```text
pulsos
```

y convertirlos mediante configuración a:

```text
mm
mm/h
mm/día
mm acumulados
```

El factor de conversión deberá ser configurable desde la web.

---

# 21. Radiación solar

SEMA deberá permitir incorporar:

```text
Piranómetro fotovoltaico
```

u otros sensores de radiación solar.

Podrá utilizar:

```text
ADC interno
ADS1115
otros ADC
RS485 / Modbus
```

según el sensor.

Parámetros configurables:

```text
Escala
Offset
Ganancia
Unidad
Calibración
Filtro
Promedio
Intervalo
```

---

# 22. Humedad de suelo

Aunque SEMA sea principalmente meteorológico, deberá permitir incorporar sensores de suelo.

Opciones:

```text
Capacitive Soil Moisture Sensor v1.2
```

y sensores industriales:

```text
RS485
Modbus RTU
```

Variables:

```text
Humedad
Temperatura
Conductividad
EC
NPK
```

cuando el sensor las proporcione.

La humedad analógica deberá calibrarse por instalación.

---

# 23. RS485 / Modbus RTU

SEMA deberá incorporar soporte para:

```text
RS485
```

como uno de sus principales buses de sensores industriales.

La interfaz física podrá utilizar:

```text
ADM2483
```

u otros transceptores aislados compatibles.

También se deberán contemplar transceptores no aislados cuando la instalación lo permita.

Arquitectura:

```text
SEMA
 │
 └── RS485
       │
       ├── Sensor pH
       ├── Sensor EC
       ├── Sensor temperatura
       ├── Sensor humedad
       ├── Sensor CO₂
       ├── Sensor radiación
       ├── Sensor suelo
       ├── Sensor presión
       └── Sensor meteorológico
```

La configuración deberá permitir:

```text
Baudrate
Paridad
Stop bits
Slave ID
Timeout
Retries
Polling interval
Registros
Tipo de dato
Endianness
Escala
Offset
```

---

# 24. Base de datos de sensores Modbus

SEMA deberá incorporar una arquitectura de perfiles.

Ejemplo:

```text
Sensor:
SoilSense-01

Protocolo:
Modbus RTU

Slave ID:
4

Baudrate:
9600

Registro:
0x0001

Tipo:
UINT16

Scale:
0.1

Unidad:
%
```

Esto permitirá agregar sensores sin modificar toda la aplicación.

---

# 25. CAN Bus

SEMA deberá contemplar CAN Bus mediante transceptores como:

```text
SN65HVD23X
```

o equivalentes compatibles.

La arquitectura deberá permitir:

```text
CAN
 │
 ├── Sensores
 ├── nodos remotos
 ├── módulos de adquisición
 ├── módulos de energía
 └── otros controladores
```

Se deberá contemplar:

```text
CAN ID
Baudrate
Filtros
Prioridad
Payload
Timeout
Estado del nodo
```

La capa de aplicación deberá mantenerse separada de la capa física CAN.

---

# 26. Expansores de entradas y salidas

La arquitectura deberá reutilizar la filosofía ya utilizada en el proyecto Invernadero.

Los expansores no deberán estar limitados a una única función.

La web deberá permitir configurar qué representa cada canal.

Ejemplo:

```text
MCP23017 #1
 │
 ├── GPIO0 → Pluviómetro
 ├── GPIO1 → Sensor de lluvia
 ├── GPIO2 → Alarma
 ├── GPIO3 → Entrada digital
 └── GPIO4 → Reserva
```

El mismo expansor podrá utilizarse para diferentes funciones según la instalación.

---

# 27. 74HC595 / 74HCT595

Se deberá soportar la utilización de:

```text
74HC595
74HCT595
```

para expansión de salidas.

Ejemplo:

```text
ESP32
 │
 ├── DATA
 ├── CLOCK
 └── LATCH
       │
       ▼
  74HC595 #1
       │
       ▼
  74HC595 #2
       │
       ▼
  74HC595 #N
```

Cada dispositivo proporciona:

```text
8 salidas
```

Ejemplo:

```text
1 × 74HC595 → 8
2 × 74HC595 → 16
4 × 74HC595 → 32
8 × 74HC595 → 64
```

La cantidad deberá ser configurable.

---

# 28. Configuración individual de cada salida

Desde la web se deberá poder configurar:

```text
Expansor:
74HC595 #2

Salida:
Q5

Nombre:
Alarma exterior

Tipo:
Digital

Estado activo:
HIGH

Invertida:
NO
```

O:

```text
Expansor:
74HC595 #1

Salida:
Q3

Nombre:
Indicador solar

Tipo:
PWM
```

---

# 29. PWM

Cuando el hardware lo permita, SEMA deberá contemplar:

```text
PWM
```

mediante:

* periféricos PWM del ESP32;
* temporizadores;
* DMA;
* técnicas de actualización eficiente para expansores.

Para salidas mediante 74HC595 se podrá implementar una arquitectura de Soft-PWM asistida por hardware.

Objetivo:

```text
CPU
 ↓
Buffer PWM
 ↓
DMA / Timer
 ↓
74HC595
```

para reducir la intervención del procesador.

Las frecuencias deberán ser configurables según el uso.

No se deberá asumir que cualquier salida mediante 74HC595 es apropiada para PWM de alta frecuencia.

---

# 30. MCP23017

Se deberá soportar:

```text
MCP23017
```

como expansor I²C.

La configuración deberá permitir seleccionar:

```text
Dirección I²C
Modo
Entrada
Salida
Pull-up
Inversión
Interrupción
Función
Nombre
```

Ejemplo:

```text
MCP23017
0x21

GPIOA0 → Sensor lluvia
GPIOA1 → Anemómetro
GPIOA2 → Alarma
GPIOB0 → Reserva
```

---

# 31. Expansores de entradas

Cuando se necesiten muchas entradas digitales se podrán incorporar:

```text
74HC165
```

u otros expansores compatibles.

El objetivo será evitar utilizar un GPIO del ESP32 para cada entrada.

---

# 32. ADC externos

Los sensores analógicos deberán poder utilizar diferentes ADC.

Como mínimo se deberá contemplar:

```text
ADC interno ESP32
ADS1115
```

y posteriormente:

```text
ADS1015
MCP3008
MCP3208
ADS8688
otros ADC
```

La arquitectura deberá abstraer:

```text
ADC
 │
 ├── Canal
 ├── Resolución
 ├── Ganancia
 ├── Escala
 ├── Offset
 ├── Filtro
 └── Calibración
```

---

# 33. Configuración de ADC

Cada canal deberá permitir:

```text
Nombre
Sensor
Magnitud
Unidad
Escala
Offset
Mínimo
Máximo
Calibración
Filtro
Promedio
Intervalo
```

Ejemplo:

```text
ADS1115 #1

A0
 ├── Sensor: Piranómetro
 ├── Magnitud: Radiación solar
 ├── Unidad: W/m²
 ├── Ganancia: ...
 ├── Offset: ...
 └── Calibración: habilitada
```

---

# 34. LoRa

SEMA deberá poder incorporar comunicación LoRa para estaciones remotas.

Arquitectura:

```text
SEMA
 │
 └── LoRa
       │
       └── Gateway
             │
             └── Servidor
```

El sistema deberá permitir configurar:

```text
Frecuencia
Spreading Factor
Bandwidth
Coding Rate
TX Power
Node ID
Network ID
Intervalo
Modo de ahorro energético
```

cuando el hardware/protocolo utilizado lo permita.

---

# 35. Zigbee

Se deberá contemplar la incorporación del módulo:

```text
RF-BM-2652P2
```

basado en un SoC compatible con Zigbee.

El módulo funcionará como:

```text
Co-procesador de red
```

mientras el ESP32 actuará como MCU principal.

Arquitectura:

```text
ESP32
 │
 └── UART
       │
       ▼
RF-BM-2652P2
       │
       ▼
Zigbee Network
```

La arquitectura deberá contemplar:

```text
Standalone
```

y:

```text
Zigbee conectado a una red existente
```

La integración Zigbee deberá estar separada del Core de SEMA.

---

# 36. Energía autónoma

SEMA deberá estar preparado para estaciones alimentadas mediante:

```text
Panel solar
     ↓
Controlador solar
     ↓
Batería
     ↓
SEMA
```

Se deberán poder medir:

```text
Tensión batería
Corriente batería
Potencia batería
Estado de carga
Tensión panel
Corriente panel
Potencia panel
Consumo
```

cuando el hardware de medición correspondiente esté instalado.

---

# 37. Sensor de tensión de batería

La medición de batería deberá configurarse como cualquier otro sensor analógico.

Ejemplo:

```text
ADC
 │
 └── Divisor resistivo
       │
       └── Batería
```

La configuración deberá incluir:

```text
Tensión máxima
Divisor
Escala
Offset
Calibración
```

Ejemplo:

```text
BatteryVoltage

Entrada:
ADS1115 A0

Unidad:
V

Factor:
11.0
```

---

# 38. Gestión energética

SEMA deberá disponer de un módulo:

```text
Energy Management
```

que permita determinar:

```text
NORMAL
LOW BATTERY
CRITICAL BATTERY
CHARGING
SOLAR AVAILABLE
SOLAR ABSENT
```

y opcionalmente modificar:

```text
intervalos de medición
```

```text
frecuencia de comunicación
```

```text
modo de suspensión
```

según el nivel de batería.

---

# 39. Deep Sleep

Las estaciones remotas deberán poder utilizar:

```text
Deep Sleep
```

cuando la aplicación lo permita.

Ejemplo:

```text
Despertar
   ↓
Inicializar
   ↓
Medir
   ↓
Procesar
   ↓
Guardar
   ↓
Transmitir
   ↓
Dormir
```

El intervalo deberá ser configurable.

---

# 40. Almacenamiento

SEMA deberá poder almacenar datos localmente.

Opciones:

```text
NVS
LittleFS
SD
microSD
servidor remoto
```

El almacenamiento deberá poder configurarse.

---

# 41. Registro histórico

Cada medición deberá poder registrar:

```text
timestamp
sensor_id
magnitud
valor
unidad
calidad
estado
```

Ejemplo:

```json
{
  "timestamp": 1790000000,
  "sensor": "TEMP_EXT",
  "value": 24.6,
  "unit": "C",
  "quality": "VALID"
}
```

---

# 42. Calidad de datos

SEMA no deberá considerar válida una medición simplemente porque el sensor respondió.

Cada lectura deberá poder tener un estado:

```text
VALID
INVALID
STALE
TIMEOUT
OUT_OF_RANGE
CALIBRATION_ERROR
COMMUNICATION_ERROR
SENSOR_DISCONNECTED
```

Esto será especialmente importante para sistemas profesionales.

---

# 43. Calibración

La calibración deberá formar parte de la configuración del sensor.

Ejemplo:

```text
Sensor
 │
 ├── Offset
 ├── Gain
 ├── Puntos de calibración
 ├── Mínimo
 └── Máximo
```

Para sensores más complejos:

```text
Calibración multipunto
```

---

# 44. Detección de sensores

Cuando sea posible, SEMA deberá detectar automáticamente:

```text
I²C
1-Wire
SPI
UART
RS485
expansores
ADC
```

Ejemplo:

```text
Bus I²C

0x23 → BH1750
0x44 → SHT31
0x76 → BME280
```

La detección no deberá modificar automáticamente configuraciones críticas.

En su lugar:

```text
Se detectó un dispositivo I²C en 0x44.

Tipo posible:
SHT3x

[ AGREGAR ]
[ IGNORAR ]
```

---

# 45. Configuración de hardware desde la web

La configuración física deberá poder realizarse desde:

```text
WEB LOCAL
```

especialmente:

```text
GPIO
I²C
SPI
UART
CAN
RS485
ADC
expansores
74HC595
74HC165
MCP23017
ADS1115
LoRa
Zigbee
Ethernet
SD
```

El usuario deberá poder seleccionar qué función utiliza cada recurso.

---

# 46. Separación entre hardware y función

Esto será obligatorio.

### Hardware

```text
GPIO 21 = SDA
GPIO 22 = SCL
GPIO 17 = UART RX
GPIO 16 = UART TX
```

### Dispositivo

```text
I²C #1
UART #1
ADC #1
RS485 #1
```

### Función

```text
SHT40
PMS5003
Sensor Modbus
```

### Magnitud

```text
Temperatura
PM2.5
CO₂
```

Estos niveles no deberán mezclarse.

---

# 47. Sistema de sensores

Cada sensor deberá tener un identificador lógico.

Ejemplo:

```text
TEMP_EXT_01
HUM_EXT_01
PRESS_EXT_01
LIGHT_01
CO2_01
RAIN_01
WIND_SPEED_01
WIND_DIR_01
SOLAR_01
BATTERY_01
```

El usuario podrá cambiar el nombre desde la web.

---

# 48. Página de sensores

La interfaz deberá permitir:

```text
Sensores
 ├── Instalados
 ├── Detectados
 ├── Disponibles
 ├── Deshabilitados
 └── Con errores
```

Cada sensor deberá mostrar:

```text
Nombre
Tipo
Modelo
Interfaz
Dirección
Canal
Valor
Unidad
Estado
Última lectura
Calibración
```

---

# 49. Configuración de un sensor

Ejemplo:

```text
SENSOR

Nombre:
Temperatura exterior

Magnitud:
Temperatura

Modelo:
SHT40

Interfaz:
I²C

Bus:
I²C #1

Dirección:
0x44

Intervalo:
10 s

Filtro:
Promedio móvil

Muestras:
5

Estado:
Habilitado
```

---

# 50. Página Dashboard

El Dashboard deberá ser modular.

Ejemplo:

```text
Dashboard
 │
 ├── Estado de estación
 ├── Temperatura
 ├── Humedad
 ├── Presión
 ├── Viento
 ├── Lluvia
 ├── Radiación solar
 ├── UV
 ├── CO₂
 ├── Calidad del aire
 ├── Rayos
 ├── Energía
 ├── Comunicación
 └── Alarmas
```

Cada bloque podrá:

* instalarse;
* registrarse;
* habilitarse;
* deshabilitarse;
* ocultarse;
* configurarse;
* actualizarse.

No todos los bloques estarán presentes en todas las estaciones.

---

# 51. Página de configuración

La configuración deberá organizarse por módulos.

```text
Configuración
 │
 ├── Estación
 ├── Red
 ├── Hardware
 ├── Buses
 ├── Expansores
 ├── Sensores
 ├── Calibración
 ├── Almacenamiento
 ├── Energía
 ├── LoRa
 ├── Zigbee
 ├── RS485
 ├── CAN
 ├── MQTT
 ├── Servidor
 ├── Alarmas
 ├── Usuarios
 ├── OTA
 └── Diagnóstico
```

---

# 52. Módulos opcionales

SEMA deberá utilizar módulos independientes.

Ejemplo:

```text
CORE
 │
 ├── Sensor Engine
 ├── Configuration
 ├── Web Server
 ├── API
 └── Diagnostics

OPTIONAL
 │
 ├── Weather
 ├── Air Quality
 ├── Lightning
 ├── Soil
 ├── Energy
 ├── LoRa
 ├── Zigbee
 ├── RS485
 ├── CAN
 ├── MQTT
 ├── SD
 └── OTA
```

Una instalación sin LoRa no deberá necesitar tener activa toda la lógica LoRa.

---

# 53. Ciclo de vida de módulos

Los módulos deberán seguir conceptualmente:

```text
AVAILABLE
    ↓
INSTALLED
    ↓
CONFIGURED
    ↓
ENABLED
    ↓
RUNNING
```

y:

```text
RUNNING
    ↓
DISABLED
    ↓
UNINSTALLED
```

Estados de error:

```text
INSTALL_ERROR
CONFIG_ERROR
RUNTIME_ERROR
UPDATE_ERROR
```

---

# 54. API

SEMA deberá disponer de API.

Ejemplo:

```text
GET /api/status
GET /api/sensors
GET /api/sensors/{id}
GET /api/weather
GET /api/energy
GET /api/events
GET /api/alarms
GET /api/config
```

Y operaciones protegidas:

```text
POST /api/config
POST /api/sensors
PUT /api/sensors/{id}
DELETE /api/sensors/{id}
POST /api/restart
POST /api/ota
```

---

# 55. WebSocket

La interfaz deberá poder recibir datos en tiempo real mediante:

```text
WebSocket
```

para evitar consultas continuas innecesarias.

Ejemplo:

```text
ESP32
  │
  └── WebSocket
        │
        └── Dashboard
```

---

# 56. MQTT

MQTT será opcional.

Configuración:

```text
Broker
Port
Username
Password
TLS
Client ID
Topic Prefix
QoS
Retain
Keep Alive
```

Ejemplo:

```text
sema/estacion01/temperature
sema/estacion01/humidity
sema/estacion01/pressure
sema/estacion01/wind
sema/estacion01/rain
```

---

# 57. Servidor central

SEMA podrá funcionar como:

```text
ESTACIÓN AUTÓNOMA
```

o:

```text
ESTACIÓN CON SERVIDOR CENTRAL
```

En ambos casos la estación deberá continuar funcionando localmente.

El servidor central podrá utilizarse para:

* visualizar estaciones;
* almacenar históricos;
* comparar estaciones;
* administrar usuarios;
* generar informes;
* recibir alarmas;
* administrar firmware;
* visualizar mapas.

La configuración física del hardware deberá permanecer preferentemente como responsabilidad local.

---

# 58. Configuración local del hardware

Por seguridad:

```text
GPIO
I²C
SPI
UART
RS485
CAN
expansores
ADC
```

deberán configurarse preferentemente desde la:

```text
WEB LOCAL
```

El servidor central podrá visualizar esta configuración como:

```text
SOLO LECTURA
```

salvo que el administrador habilite explícitamente la modificación remota.

---

# 59. Red

SEMA deberá soportar, según el hardware:

```text
Wi-Fi
Ethernet
LoRa
Zigbee
RS485
CAN
```

La configuración de red deberá permitir:

```text
DHCP
IP estática
Gateway
DNS
NTP
Zona horaria
mDNS
Hostname
```

---

# 60. Access Point inicial

Una estación sin configurar deberá crear un AP identificable.

Ejemplo:

```text
SEMA-A1B2C3
```

donde:

```text
A1B2C3
```

corresponde a una parte de la identidad única del dispositivo.

La contraseña no deberá ser universal.

---

# 61. mDNS

El hostname deberá ser configurable.

Ejemplos:

```text
sema-01.local
sema-rural-01.local
sema-techo.local
sema-estacion-norte.local
```

---

# 62. Zona horaria

Se deberán utilizar preferentemente identificadores IANA.

Ejemplo:

```text
America/Argentina/Buenos_Aires
```

La estación deberá poder configurar:

```text
NTP automático
NTP personalizado
Zona horaria
```

---

# 63. Alarmas

SEMA deberá disponer de un sistema de alarmas modular.

Ejemplos:

```text
Temperatura alta
Temperatura baja
Humedad alta
Humedad baja
Presión anormal
Viento fuerte
Lluvia
Actividad eléctrica
CO₂ alto
PM2.5 alto
Batería baja
Batería crítica
Pérdida de comunicación
Sensor desconectado
Sensor fuera de rango
```

Cada alarma deberá permitir:

```text
Habilitar
Deshabilitar
Umbral
Histéresis
Tiempo
Prioridad
Acción
Notificación
```

---

# 64. Estado de la estación

La estación deberá tener estados globales.

```text
ONLINE
RUN
DEGRADED
SAFE
ERROR
MAINTENANCE
OTA
SLEEP
```

Ejemplo:

```text
ONLINE
42 sensores
1 advertencia
0 errores
Batería: 87 %
```

---

# 65. Diagnóstico

La página de diagnóstico deberá permitir revisar:

```text
CPU
RAM
Flash
Temperatura interna
Wi-Fi
Ethernet
I²C
SPI
UART
RS485
CAN
LoRa
Zigbee
ADC
Expansores
Sensores
Almacenamiento
Batería
```

También deberá mostrar:

```text
Último reinicio
Último error
Última alarma
Tiempo de funcionamiento
Versión firmware
Versión configuración
```

---

# 66. Detección de conflictos

El sistema deberá detectar configuraciones incompatibles.

Ejemplo:

```text
ERROR

GPIO 21 está asignado simultáneamente a:

I²C SDA
y
Sensor digital

[ VOLVER A CONFIGURACIÓN ]
```

También deberá detectar:

```text
Dirección I²C duplicada
GPIO duplicado
Canal ADC duplicado
Slave ID Modbus duplicado
CAN ID conflictivo
UART ocupado
Expansor duplicado
```

---

# 67. Validación antes de guardar

Toda configuración crítica deberá pasar por:

```text
Editar
 ↓
Validar
 ↓
Mostrar conflictos
 ↓
Confirmar
 ↓
Guardar
 ↓
Aplicar
```

Cuando sea necesario:

```text
Reiniciar
```

---

# 68. Importación y exportación

SEMA deberá permitir:

```text
Exportar configuración completa
Exportar sensores
Exportar hardware
Exportar red
Exportar comunicaciones
Exportar alarmas
Exportar calibración
```

También:

```text
Importar configuración
```

Antes de aplicar:

```text
Validar
Mostrar cambios
Confirmar
Crear backup
Aplicar
```

---

# 69. Clonado

Se deberá poder clonar una configuración entre estaciones.

Ejemplo:

```text
SEMA A
  ↓
config.json
  ↓
SEMA B
```

Pero nunca deberán clonarse:

```text
MAC
UID
device_id
claves privadas
certificados
credenciales únicas
```

La nueva estación deberá generar su propia identidad.

---

# 70. Plantillas

Se podrán crear plantillas.

Ejemplos:

```text
template-basic.json
template-weather.json
template-agriculture.json
template-air-quality.json
template-solar.json
template-industrial.json
template-remote-lora.json
```

Al importar:

```text
Plantilla
   ↓
Detectar hardware
   ↓
Validar capacidades
   ↓
Adaptar
   ↓
Aplicar
```

---

# 71. OTA

SEMA deberá soportar actualización OTA.

La interfaz deberá mostrar:

```text
Versión actual
Versión disponible
Fecha
Cambios
Compatibilidad
```

Y permitir:

```text
Buscar actualización
Descargar
Verificar
Instalar
Reiniciar
```

Deberá existir recuperación ante errores de actualización siempre que el hardware lo permita.

---

# 72. Seguridad

La interfaz deberá contemplar autenticación.

Se deberán diferenciar al menos:

```text
Administrador
Técnico
Usuario
Solo lectura
```

El acceso a:

```text
GPIO
buses
expansores
calibración
red
OTA
```

deberá estar protegido.

---

# 73. Configuración de sensores desde la web

Este será uno de los principios centrales de SEMA.

Ejemplo:

```text
MAGNITUD

Temperatura y humedad
```

El usuario podrá seleccionar:

```text
Sensor:
AHT20
AHT21
AHT30
SHT31
SHT40
DHT22
BME280
...
```

Después:

```text
Interfaz:
I²C

Bus:
I²C #1

Dirección:
0x44
```

El sistema deberá adaptar automáticamente la configuración disponible al sensor seleccionado.

---

# 74. Sensores opcionales

No todos los usuarios necesitarán todos los sensores.

Por ejemplo:

```text
Estación económica

✓ Temperatura
✓ Humedad
✓ Presión
✓ Lluvia
✓ Viento

✗ CO₂
✗ PM2.5
✗ UV
✗ Rayos
✗ Suelo
```

Otra estación podrá utilizar:

```text
✓ Temperatura
✓ Humedad
✓ Presión
✓ CO₂
✓ PM2.5
✓ UV
✓ Rayos
✓ Radiación
✓ Suelo
✓ Energía
✓ LoRa
✓ Zigbee
✓ RS485
✓ CAN
```

El firmware deberá soportar ambos escenarios.

---

# 75. Base de datos de sensores

SEMA deberá evolucionar hacia una base de datos interna de sensores.

Cada sensor deberá definir conceptualmente:

```text
ID
Fabricante
Modelo
Magnitudes
Interfaz
Dirección
Canales
Rango
Precisión
Unidades
Configuración
Calibración
Dependencias
Requisitos eléctricos
```

Ejemplo:

```json
{
  "id": "sht40",
  "manufacturer": "Sensirion",
  "type": "temperature_humidity",
  "interface": "i2c",
  "measurements": [
    "temperature",
    "humidity"
  ]
}
```

---

# 76. Capabilities

Cada módulo deberá declarar sus capacidades.

Ejemplo:

```text
SCD41
 ├── I2C
 ├── Temperature
 ├── Humidity
 └── CO2
```

Mientras:

```text
BH1750
 ├── I2C
 └── Illuminance
```

La interfaz web deberá utilizar estas capacidades para mostrar solamente las opciones correspondientes.

---

# 77. Arquitectura de firmware

Se propone:

```text
src/
│
├── core/
│
├── config/
│
├── hardware/
│
├── buses/
│
├── sensors/
│
├── actuators/
│
├── communications/
│
├── storage/
│
├── energy/
│
├── alarms/
│
├── diagnostics/
│
├── web/
│
├── api/
│
├── ota/
│
└── modules/
```

Los detalles podrán evolucionar durante el desarrollo.

---

# 78. Arquitectura conceptual

```text
                    SEMA CORE
                        │
        ┌───────────────┼────────────────┐
        │               │                │
    Hardware         Sensors          Services
        │               │                │
        │               │                ├── Web
        │               │                ├── API
        │               │                ├── MQTT
        │               │                ├── OTA
        │               │                └── Storage
        │               │
        ├── GPIO        ├── Temperature
        ├── I²C         ├── Humidity
        ├── SPI         ├── Pressure
        ├── UART        ├── Light
        ├── ADC         ├── UV
        ├── RS485       ├── CO₂
        ├── CAN         ├── PM
        ├── 1-Wire      ├── Wind
        ├── MCP23017    ├── Rain
        ├── 74HC595     ├── Solar
        ├── 74HC165     ├── Lightning
        └── ADS1115     ├── Soil
                        └── Energy
```

---

# 79. Diseño modular de páginas

Las páginas nunca deberán convertirse en grandes bloques monolíticos.

Ejemplo:

```text
Dashboard
 │
 ├── StationStatus
 ├── TemperatureWidget
 ├── HumidityWidget
 ├── PressureWidget
 ├── WindWidget
 ├── RainWidget
 ├── SolarWidget
 ├── UVWidget
 ├── AirQualityWidget
 ├── LightningWidget
 ├── EnergyWidget
 └── AlarmWidget
```

Si una estación no tiene UV:

```text
UVWidget
   ↓
NO INSTALADO
```

o simplemente:

```text
No mostrar
```

según la configuración del Dashboard.

---

# 80. Módulos instalables

El sistema deberá permitir evolucionar hacia módulos instalables.

Ejemplo:

```text
Weather Core
Air Quality Module
Lightning Module
Solar Energy Module
Soil Module
LoRa Module
Zigbee Module
Modbus Module
CAN Module
MQTT Module
```

Cada módulo deberá declarar:

```text
ID
Versión
Dependencias
Capabilities
Configuración
Permisos
Estado
```

---

# 81. Telemetría

SEMA deberá generar una estructura de telemetría común.

Ejemplo:

```json
{
  "station": "SEMA-01",
  "timestamp": 1790000000,
  "measurements": {
    "temperature": 24.8,
    "humidity": 61.2,
    "pressure": 1012.8,
    "wind_speed": 14.3,
    "wind_direction": 185,
    "rain": 0,
    "solar_radiation": 643
  }
}
```

Los campos deberán generarse dinámicamente según los sensores instalados.

---

# 82. Unidades

El sistema deberá permitir configurar unidades.

Ejemplos:

```text
Temperatura:
°C / °F / K

Presión:
hPa / Pa / kPa / mbar / inHg

Viento:
km/h / m/s / mph / knots

Lluvia:
mm / in

Radiación:
W/m²

Luminosidad:
lux

Distancia:
m / ft
```

---

# 83. Cálculos derivados

SEMA podrá calcular magnitudes que no requieren un sensor independiente.

Ejemplos:

```text
Punto de rocío
Índice de calor
Sensación térmica
Presión reducida al nivel del mar
Índice UV
Acumulación de lluvia
Ráfaga máxima
Promedio de viento
Energía solar acumulada
```

Los cálculos deberán estar separados de los drivers de sensores.

---

# 84. Calidad y validación de datos

Antes de almacenar una lectura se deberá poder aplicar:

```text
Rango mínimo
Rango máximo
Filtro
Promedio
Mediana
Descarte de outliers
Validación temporal
Validación de comunicación
```

Esto será especialmente importante para sensores exteriores.

---

# 85. Protección frente a fallos

Un sensor defectuoso no deberá bloquear toda la estación.

Ejemplo:

```text
SHT40 ❌
     │
     ↓
Sensor ERROR
     │
     ↓
SEMA continúa
     │
     ├── Presión ✓
     ├── Viento ✓
     ├── Lluvia ✓
     ├── CO₂ ✓
     └── Energía ✓
```

---

# 86. Watchdog

El sistema deberá utilizar mecanismos de recuperación ante bloqueo.

Deberá contemplarse:

```text
Watchdog
Timeout de sensores
Timeout de buses
Reintentos
Reinicio controlado
Registro del motivo
```

---

# 87. Registro de eventos

Se deberán registrar eventos como:

```text
BOOT
REBOOT
SENSOR_CONNECTED
SENSOR_DISCONNECTED
SENSOR_ERROR
CONFIG_CHANGED
ALARM
ALARM_CLEAR
NETWORK_CONNECTED
NETWORK_DISCONNECTED
OTA_START
OTA_SUCCESS
OTA_ERROR
LOW_BATTERY
```

---

# 88. Modo mantenimiento

SEMA deberá disponer de:

```text
MAINTENANCE MODE
```

para tareas técnicas.

Durante este modo podrán modificarse:

```text
GPIO
buses
sensores
expansores
calibración
red
firmware
```

según permisos.

---

# 89. Diagnóstico de buses

Cada bus deberá disponer de herramientas de diagnóstico.

### I²C

```text
Scan
Dirección
ACK
Error
```

### 1-Wire

```text
Scan
ROM
Familia
Temperatura
```

### UART

```text
RX
TX
Baudrate
Frames
```

### RS485

```text
Slave
Registro
Respuesta
CRC
Timeout
```

### CAN

```text
ID
Frame
DLC
Payload
Errores
```

---

# 90. Configuración protegida del hardware

Las opciones que puedan dejar inutilizable la estación deberán mostrar advertencias.

Ejemplo:

```text
⚠ ADVERTENCIA

Cambiar GPIO21 puede desconectar el bus I²C.

¿Desea continuar?

[CANCELAR] [CONTINUAR]
```

---

# 91. Backup automático

Antes de cambios importantes:

```text
Configuración actual
       ↓
Backup
       ↓
Modificar
       ↓
Validar
       ↓
Aplicar
```

Si la nueva configuración falla:

```text
Rollback
```

cuando sea técnicamente posible.

---

# 92. Versionado de configuración

La configuración deberá tener versión.

Ejemplo:

```json
{
  "config_version": 3,
  "schema_version": 1
}
```

Esto permitirá migrar configuraciones antiguas.

---

# 93. Compatibilidad futura

La arquitectura deberá permitir incorporar posteriormente:

```text
Sensores meteorológicos comerciales
Sensores industriales
Modbus TCP
Ethernet
PoE
GNSS/GPS
RTC
SD
eMMC
LTE
4G
NB-IoT
Cat-M1
Thread
Matter
otros protocolos IoT
```

sin modificar el Core innecesariamente.

---

# 94. Hardware futuro

SEMA deberá evolucionar eventualmente hacia una PCB propia.

La PCB deberá poder contemplar:

```text
ESP32
Protección de alimentación
I²C
SPI
UART
RS485 aislado
CAN
ADC
Expansores
LoRa
Zigbee
Ethernet
SD
RTC
Medición de batería
Entradas protegidas
Protección ESD
Protección contra sobretensión
```

La implementación final deberá definirse después de validar el prototipo.

---

# 95. Diseño para exterior

Cuando SEMA se utilice como estación exterior deberán considerarse:

```text
Protección IP
Condensación
Radiación UV
Temperatura extrema
Descargas atmosféricas
ESD
Sobretensiones
Ruido electromagnético
Protección de entradas
Protección de alimentación
```

La electrónica no deberá considerarse protegida únicamente por estar dentro de una caja.

---

# 96. Separación entre sensores y electrónica

Cuando sea necesario, los sensores deberán poder conectarse mediante:

```text
cables largos
RS485
CAN
nodos remotos
```

No se deberá asumir que todos los sensores estarán físicamente junto al ESP32.

---

# 97. Arquitectura distribuida

SEMA deberá poder evolucionar desde:

```text
ESP32
 │
 ├── sensores locales
 └── comunicaciones
```

hacia:

```text
                         SEMA CENTRAL
                              │
              ┌───────────────┼───────────────┐
              │               │               │
           Nodo #1         Nodo #2         Nodo #3
              │               │               │
           Sensores        Sensores        Sensores
```

Los nodos podrán utilizar:

```text
RS485
CAN
LoRa
Zigbee
Wi-Fi
```

según la aplicación.

---

# 98. Integración con estaciones meteorológicas existentes

SEMA deberá poder recibir información de estaciones externas cuando exista una interfaz disponible.

Por ejemplo:

```text
HTTP JSON
MQTT
Modbus
RS485
CAN
Wi-Fi
```

Los nombres de los campos deberán ser configurables.

Ejemplo:

```json
{
  "weather": {
    "enabled": true,
    "url": "http://192.168.1.50/weather.json",
    "interval_ms": 60000,
    "key_temp": "temp",
    "key_hum": "hum",
    "key_wind": "wind",
    "key_rain": "rain",
    "key_pressure": "pressure",
    "key_light": "lux"
  }
}
```

Nunca se deberá asumir que todas las estaciones utilizan los mismos nombres de campos.

---

# 99. Servidor central y estaciones externas

SEMA podrá utilizar una estación meteorológica existente como fuente externa.

Ejemplo:

```text
Estación meteorológica
        │
        │ HTTP / MQTT / Modbus
        ▼
       SEMA
        │
        ├── Datos propios
        ├── Datos externos
        └── Datos calculados
```

La interfaz deberá identificar claramente el origen de cada medición.

---

# 100. Arquitectura final esperada

La arquitectura completa deberá poder evolucionar hacia:

```text
                              SERVIDOR
                                  │
                       HTTPS / MQTT / API
                                  │
                 ┌────────────────┼────────────────┐
                 │                │                │
              SEMA #1          SEMA #2          SEMA #N
                 │                │                │
       ┌─────────┼─────────┐      │                │
       │         │         │      │                │
      I²C       SPI      UART    LoRa            Zigbee
       │         │         │
       │         │         └── PMS5003
       │         │
       │         └── AS3935
       │
       ├── SHT40
       ├── BME280
       ├── BH1750
       ├── SCD41
       └── OPT3001

       RS485
         │
         ├── pH
         ├── EC
         ├── Suelo
         ├── Radiación
         └── Sensores industriales

       CAN
         │
         ├── nodos remotos
         └── módulos externos

       ADC
         │
         ├── Piranómetro
         ├── Batería
         ├── Panel solar
         └── Sensores analógicos

       EXPANSORES
         │
         ├── MCP23017
         ├── 74HC165
         └── 74HC595
```

---

# 101. Principio de escalabilidad

Una instalación pequeña:

```text
ESP32
 │
 ├── SHT40
 ├── BMP280
 ├── BH1750
 └── Pluviómetro
```

deberá utilizar el mismo concepto arquitectónico que una instalación profesional:

```text
ESP32-S3
 │
 ├── I²C
 ├── SPI
 ├── UART
 ├── RS485
 ├── CAN
 ├── LoRa
 ├── Zigbee
 ├── Ethernet
 ├── ADS1115
 ├── MCP23017
 ├── 74HC595
 ├── 74HC165
 ├── SD
 ├── sensores meteorológicos
 ├── sensores industriales
 └── sistema solar
```

La diferencia deberá estar principalmente en la configuración y en los módulos habilitados.

---

# 102. Regla principal del proyecto

SEMA no deberá diseñarse como:

```text
"Una estación meteorológica con determinados sensores."
```

Deberá diseñarse como:

```text
"Una plataforma configurable para construir diferentes estaciones meteorológicas y ambientales."
```

Por lo tanto:

```text
SENSOR ≠ FUNCIÓN
GPIO ≠ SENSOR
BUS ≠ SENSOR
MODELO ≠ MAGNITUD
HARDWARE ≠ CONFIGURACIÓN
```

La abstracción deberá permitir:

```text
Magnitud
    ↓
Sensor
    ↓
Interfaz
    ↓
Bus
    ↓
Canal
    ↓
Procesamiento
    ↓
Calibración
    ↓
Dato
    ↓
Alarma / Histórico / API / Dashboard
```

---

# 103. Definition of Done

SEMA no deberá considerarse funcionalmente terminado solamente porque pueda leer algunos sensores.

Como mínimo deberá cumplirse:

```text
[ ] Core funcionando
[ ] Configuración persistente
[ ] Web local
[ ] API
[ ] Dashboard modular
[ ] Sistema de módulos
[ ] Sistema de sensores
[ ] Catálogo de sensores
[ ] Detección I²C
[ ] Detección 1-Wire
[ ] Configuración GPIO
[ ] Configuración ADC
[ ] MCP23017
[ ] 74HC595
[ ] 74HC165
[ ] RS485
[ ] Modbus RTU
[ ] CAN
[ ] LoRa
[ ] Zigbee
[ ] Medición energética
[ ] Almacenamiento
[ ] Histórico
[ ] Alarmas
[ ] Diagnóstico
[ ] Calibración
[ ] Validación de configuración
[ ] Backup
[ ] Importación
[ ] Exportación
[ ] OTA
[ ] Seguridad
[ ] Watchdog
[ ] Documentación
```

Los módulos que todavía no estén implementados podrán permanecer:

```text
AVAILABLE
```

sin ser obligatoriamente:

```text
ENABLED
```

---

# 104. Evolución del proyecto

SEMA deberá desarrollarse incrementalmente.

### Fase 1 — Core

```text
ESP32
Web
Configuración
NVS
Diagnóstico
```

### Fase 2 — Sensores básicos

```text
DS18B20
AHT20/AHT21/AHT30
SHT31/SHT40
BME280
BMP280
BH1750
```

### Fase 3 — Expansión

```text
MCP23017
74HC595
74HC165
ADS1115
```

### Fase 4 — Meteorología

```text
Viento
Lluvia
Radiación
UV
Rayos
```

### Fase 5 — Calidad ambiental

```text
CO₂
PM
CO
```

### Fase 6 — Industrial

```text
RS485
Modbus
CAN
```

### Fase 7 — Comunicaciones remotas

```text
LoRa
Zigbee
Ethernet
MQTT
Servidor central
```

### Fase 8 — Energía

```text
Panel solar
Batería
Medición energética
Deep Sleep
```

### Fase 9 — Plataforma distribuida

```text
Múltiples SEMA
Nodos remotos
Servidor central
Históricos
Mapas
Alertas
```

---

# 105. Principio final

SEMA deberá cumplir:

```text
┌───────────────────────────────────────┐
│                 SEMA                  │
│                                       │
│  SIMPLE POR FUERA                     │
│                                       │
│  MODULAR POR DENTRO                   │
│                                       │
│  CONFIGURABLE DESDE LA WEB            │
│                                       │
│  HARDWARE INDEPENDIENTE DE LA FUNCIÓN │
│                                       │
│  SENSORES INTERCAMBIABLES             │
│                                       │
│  PROTOCOLOS MODULARES                 │
│                                       │
│  FUNCIONAMIENTO AUTÓNOMO              │
│                                       │
│  PREPARADO PARA CRECER                │
└───────────────────────────────────────┘
```

# 106. API pública de SEMA

SEMA deberá disponer de una API HTTP/REST propia para permitir que aplicaciones externas puedan consultar los datos de la estación.

La API deberá ser independiente de la interfaz web.

Arquitectura:

```text
                         SEMA
                          │
                    Data Engine
                          │
             ┌────────────┼────────────┐
             │            │            │
            WEB          API        MQTT
             │            │            │
          Usuario    Aplicaciones   IoT
```

La API deberá permitir consultar:

```text
GET /api/v1/status
GET /api/v1/station
GET /api/v1/sensors
GET /api/v1/sensors/{id}
GET /api/v1/weather/current
GET /api/v1/weather/history
GET /api/v1/energy
GET /api/v1/alarms
GET /api/v1/events
GET /api/v1/system
GET /api/v1/network
GET /api/v1/diagnostics
```

---

# 107. API JSON

El formato principal de intercambio deberá ser:

```text
application/json
```

Ejemplo:

```json
{
  "station": {
    "id": "SEMA-001",
    "name": "Estacion Norte",
    "firmware": "1.0.0"
  },
  "timestamp": "2026-09-30T15:00:00Z",
  "measurements": {
    "temperature": 24.7,
    "humidity": 61.2,
    "pressure": 1014.2,
    "wind_speed": 12.4,
    "wind_direction": 182,
    "rain_rate": 0,
    "rain_daily": 3.4,
    "solar_radiation": 643,
    "uv_index": 4.2,
    "co2": 421
  }
}
```

La estructura deberá ser dinámica.

SEMA no deberá enviar campos inexistentes simplemente para completar un JSON.

---

# 108. Versionado de API

La API deberá estar versionada.

Ejemplo:

```text
/api/v1/
```

Posteriormente:

```text
/api/v2/
```

Una actualización de firmware no deberá romper inmediatamente las aplicaciones existentes.

La compatibilidad deberá mantenerse durante un período razonable.

---

# 109. API de mediciones

Deberá existir una respuesta normalizada para cualquier magnitud.

Ejemplo:

```json
{
  "id": "temperature_outdoor",
  "type": "temperature",
  "value": 24.7,
  "unit": "°C",
  "timestamp": "2026-09-30T15:00:00Z",
  "quality": "VALID",
  "source": {
    "sensor": "SHT40",
    "interface": "I2C",
    "bus": 1,
    "address": "0x44"
  }
}
```

Esto permitirá que aplicaciones externas sepan:

* qué se midió;
* cuánto dio;
* en qué unidad;
* cuándo se midió;
* si la medición es válida;
* de qué sensor proviene.

---

# 110. API de históricos

La API deberá permitir consultar históricos.

Ejemplo:

```text
GET /api/v1/history?from=...&to=...
```

Filtros:

```text
sensor
magnitud
fecha inicial
fecha final
intervalo
calidad
```

Ejemplo:

```text
GET /api/v1/history?
sensor=temperature_outdoor
&from=2026-09-01T00:00:00Z
&to=2026-09-30T23:59:59Z
&interval=10m
```

La respuesta podrá entregar:

```json
{
  "sensor": "temperature_outdoor",
  "unit": "°C",
  "samples": [
    {
      "timestamp": "2026-09-30T14:00:00Z",
      "value": 23.8
    },
    {
      "timestamp": "2026-09-30T14:10:00Z",
      "value": 24.1
    }
  ]
}
```

---

# 111. API de eventos

Los eventos meteorológicos deberán poder consultarse independientemente de las mediciones.

Ejemplo:

```text
GET /api/v1/events
```

Eventos:

```text
RAIN_START
RAIN_STOP
LIGHTNING
HIGH_WIND
HEAVY_RAIN
FROST
HEAT
LOW_BATTERY
SENSOR_ERROR
```

Ejemplo:

```json
{
  "event": "RAIN_START",
  "timestamp": "2026-09-30T14:32:11Z",
  "source": "rain_gauge",
  "value": 1
}
```

---

# 112. API Server / API Client

SEMA deberá poder funcionar en ambos sentidos.

## API Server

SEMA publica:

```text
GET /api/v1/...
```

## API Client

SEMA consulta o publica hacia:

```text
servidores externos
```

Esto permitirá:

```text
Aplicación → SEMA
```

y:

```text
SEMA → Aplicación
```

---

# 113. Webhooks

SEMA deberá poder enviar eventos mediante HTTP POST.

Ejemplo:

```text
POST https://servidor.example.com/sema/event
```

Para:

```text
lluvia
rayo
viento fuerte
alarma
batería baja
sensor desconectado
```

El usuario deberá poder configurar:

```text
URL
método
headers
autenticación
TLS
timeout
reintentos
intervalo
eventos habilitados
```

---

# 114. Publicación hacia servicios externos

SEMA deberá disponer de un sistema genérico:

```text
External Services
```

Cada servicio será un módulo independiente.

Ejemplo:

```text
External Services
 │
 ├── PWSWeather
 ├── ThingSpeak
 ├── Weathercloud
 ├── Windy
 ├── MQTT
 ├── HTTP/REST
 ├── Webhook
 └── Custom API
```

Esto permitirá agregar nuevos servicios sin modificar el Core.

---

# 115. PWSWeather

SEMA deberá contemplar integración con:

```text
PWSWeather
```

PWSWeather está orientado a estaciones meteorológicas personales y permite publicar datos de estaciones para visualización y archivo; además dispone de integraciones con aplicaciones externas.

La integración deberá ser configurable.

Campos potenciales:

```text
Station ID
Username
Password / API credential
Interval
Temperature
Humidity
Pressure
Wind
Rain
Solar
UV
```

La implementación concreta deberá respetar el método de publicación vigente de PWSWeather.

No se deberá hardcodear una URL antigua.

---

# 116. ThingSpeak

SEMA deberá incorporar un módulo:

```text
ThingSpeak
```

ThingSpeak permite actualizar canales mediante HTTP GET o POST y admite datos enviados mediante JSON.

Configuración:

```text
Enable
Channel ID
Write API Key
Update interval
Field mapping
Latitude
Longitude
Elevation
Status
```

Ejemplo:

```text
Field 1 → Temperature
Field 2 → Humidity
Field 3 → Pressure
Field 4 → Wind Speed
Field 5 → Rain
Field 6 → Solar Radiation
```

La asignación deberá poder modificarse desde la web.

---

# 117. Weathercloud

SEMA deberá incorporar:

```text
Weathercloud
```

Weathercloud dispone de una API de envío de mediciones mediante `WID` y `Key`. Su documentación contempla variables meteorológicas como temperatura, humedad, presión y batería, entre otras.

Configuración:

```text
Enable
WID
Key
Upload interval
Sensor mapping
```

La frecuencia de publicación deberá respetar los límites del servicio y del plan utilizado.

---

# 118. Windy Stations

SEMA deberá incorporar:

```text
Windy Stations
```

La integración deberá utilizar la API vigente de Windy Stations.

La documentación actual indica que la API v2 es la especificación vigente desde enero de 2026 y que la API anterior será retirada al finalizar 2026.

Configuración:

```text
Enable
Station ID
Station Password
API Key
Upload interval
Field mapping
```

SEMA deberá poder publicar las observaciones compatibles.

---

# 119. Arquitectura de servicios externos

No se deberá implementar:

```cpp
if (thingSpeak)
if (windy)
if (weathercloud)
```

dentro del código principal.

Se deberá utilizar:

```text
ExternalPublisher
       │
       ├── ThingSpeakPublisher
       ├── WeathercloudPublisher
       ├── WindyPublisher
       ├── PWSWeatherPublisher
       └── CustomHTTPPublisher
```

Todos recibirán datos desde el mismo:

```text
Measurement Engine
```

---

# 120. Mapeo de variables externas

Cada servicio deberá tener un mapeo independiente.

Ejemplo:

```text
SEMA:
temperature_outdoor
        │
        ├── ThingSpeak → field1
        ├── Weathercloud → temp
        ├── Windy → temperature
        └── MQTT → temperature
```

Esto evita modificar los sensores cuando cambia un servicio externo.

---

# 121. Publicador HTTP genérico

Además de servicios conocidos, SEMA deberá disponer de:

```text
Custom HTTP Publisher
```

Configuración:

```text
URL
HTTP method
Headers
Authentication
Content-Type
Body template
Interval
Timeout
Retry
TLS
```

Ejemplo:

```http
POST /weather HTTP/1.1
Content-Type: application/json
Authorization: Bearer XXXXX
```

Body:

```json
{
  "temperature": "{{temperature}}",
  "humidity": "{{humidity}}",
  "pressure": "{{pressure}}"
}
```

SEMA deberá sustituir dinámicamente las variables.

---

# 122. Cola de publicación

Las comunicaciones externas no deberán bloquear la adquisición de sensores.

Arquitectura:

```text
Sensor
  ↓
Measurement Engine
  ↓
Local Storage
  ↓
Publish Queue
  ↓
┌────────────┬─────────────┬────────────┐
ThingSpeak   Windy      Weathercloud   MQTT
```

Si Internet falla:

```text
Internet ❌
     ↓
Queue
     ↓
Reintentar
```

---

# 123. Store and Forward

SEMA deberá implementar:

```text
STORE & FORWARD
```

Ejemplo:

```text
Medición
   ↓
Guardar localmente
   ↓
Intentar enviar
   ↓
Internet ❌
   ↓
Conservar
   ↓
Internet ✓
   ↓
Transmitir pendientes
```

Esto evitará perder datos por cortes de Internet.

---

# 124. Política de reintentos

Cada servicio deberá permitir:

```text
Timeout
Retry count
Retry delay
Backoff
Maximum queue
```

Se recomienda utilizar:

```text
Exponential Backoff
```

para evitar saturar servidores externos.

---

# 125. Estado de servicios externos

El Dashboard deberá mostrar:

```text
ThingSpeak      ✓ Connected
Weathercloud    ✓ Connected
Windy           ✓ Connected
PWSWeather     ⚠ Retry
MQTT            ✗ Offline
```

También:

```text
Último envío
Último error
Cantidad de reintentos
Datos pendientes
```

---

# 126. Sincronización de tiempo

SEMA deberá disponer de un sistema centralizado de tiempo.

Fuentes:

```text
NTP
RTC
GPS/GNSS
Servidor central
```

Prioridad configurable.

Ejemplo:

```text
NTP
 ↓
RTC
 ↓
GPS
```

o:

```text
GPS
 ↓
NTP
 ↓
RTC
```

según la aplicación.

---

# 127. NTP

La sincronización NTP deberá ser configurable.

Parámetros:

```text
Enable
Primary server
Secondary server
Tertiary server
Sync interval
Timezone
DST
Retry
```

Valores iniciales posibles:

```text
pool.ntp.org
time.google.com
time.cloudflare.com
```

El usuario deberá poder modificarlos.

---

# 128. Estado del reloj

SEMA deberá conocer:

```text
TIME_UNSYNCED
TIME_SYNCING
TIME_SYNCED
TIME_DEGRADED
```

No se deberán registrar datos como históricos válidos si el timestamp no es confiable, salvo que el usuario configure explícitamente un comportamiento alternativo.

---

# 129. RTC

SEMA deberá poder utilizar RTC externo.

Ejemplos:

```text
DS3231
PCF8563
RV-3028
```

Esto será especialmente útil cuando:

```text
Internet ❌
NTP ❌
Deep Sleep ✓
```

El RTC permitirá conservar la referencia temporal.

---

# 130. Timestamp monotónico

Además del reloj absoluto, SEMA deberá utilizar un contador monotónico interno para medir:

```text
intervalos
duraciones
timeouts
sleep
runtime
```

Nunca se deberá utilizar exclusivamente el reloj NTP para calcular duraciones.

---

# 131. Watchdog Timer

SEMA deberá incorporar Watchdog Timer.

Deberá contemplarse como mínimo:

```text
Hardware Watchdog
Task Watchdog
Software health monitor
```

El sistema deberá poder detectar:

```text
Task bloqueada
Bus bloqueado
Sensor bloqueado
Web bloqueada
Comunicación bloqueada
```

---

# 132. Watchdog por tareas

Las tareas críticas deberán registrar actividad.

Ejemplo:

```text
SensorTask
WebTask
NetworkTask
StorageTask
CommunicationTask
MeasurementTask
```

Cada tarea deberá indicar:

```text
last_activity
```

El Health Monitor comprobará:

```text
¿La tarea continúa respondiendo?
```

---

# 133. Recuperación mediante Watchdog

Cuando se produzca un bloqueo:

```text
Task bloqueada
      ↓
Intentar recuperación
      ↓
Reiniciar módulo
      ↓
Si falla
      ↓
Reiniciar bus
      ↓
Si falla
      ↓
Watchdog reset
```

Después del reinicio:

```text
Registrar motivo
```

Ejemplo:

```text
Last reset:
TASK_WDT
Task:
RS485
```

---

# 134. Contador de reinicios

SEMA deberá almacenar:

```text
boot_count
watchdog_count
brownout_count
manual_reset_count
software_reset_count
panic_count
```

Esto permitirá detectar estaciones inestables.

---

# 135. Diagnóstico post-reinicio

Después de un reinicio inesperado:

```text
BOOT
 ↓
Detectar reset reason
 ↓
Registrar
 ↓
Inicializar
 ↓
Comprobar configuración
 ↓
Comprobar sensores
 ↓
Comprobar buses
 ↓
RUN
```

---

# 136. Modo de bajo consumo

SEMA deberá soportar:

```text
Active
Light Sleep
Deep Sleep
```

según las capacidades del ESP32 utilizado.

La política energética deberá ser configurable.

---

# 137. Perfil energético

Se deberán poder crear perfiles:

```text
PERFORMANCE
NORMAL
LOW_POWER
ULTRA_LOW_POWER
```

Ejemplo:

```text
NORMAL
 ├── medir cada 10 s
 ├── Wi-Fi permanente
 └── dashboard activo
```

```text
LOW_POWER
 ├── medir cada 1 min
 ├── Wi-Fi por intervalos
 └── comunicación periódica
```

```text
ULTRA_LOW_POWER
 ├── Deep Sleep
 ├── despertar por RTC
 ├── despertar por lluvia
 ├── medir
 ├── transmitir
 └── volver a dormir
```

---

# 138. Wake-up por RTC

SEMA deberá poder despertar mediante:

```text
RTC Timer
```

Ejemplo:

```text
Dormir 5 minutos
      ↓
Wake
      ↓
Medir
      ↓
Transmitir
      ↓
Dormir
```

---

# 139. Wake-up por GPIO

Se deberá soportar:

```text
GPIO Wake-up
```

cuando el ESP32 y el modo de sleep seleccionado lo permitan.

Esto permitirá despertar por eventos externos.

---

# 140. External Interrupt / INT

SEMA deberá disponer de una abstracción:

```text
External Interrupt
```

para sensores que dispongan de una salida:

```text
INT
IRQ
ALERT
DRDY
PULSE
```

Ejemplos:

```text
Sensor de lluvia
AS3935
MCP23017
sensores de alarma
sensores industriales
```

---

# 141. Wake-up por lluvia

Una estación de bajo consumo podrá funcionar así:

```text
                 DEEP SLEEP
                     │
                     │
               lluvia detectada
                     │
                     ▼
                GPIO / INT
                     │
                     ▼
                  WAKE UP
                     │
          ┌──────────┼──────────┐
          │          │          │
       medir      registrar   transmitir
          │          │          │
          └──────────┼──────────┘
                     │
                     ▼
                DEEP SLEEP
```

Esto permitirá que una estación normalmente dormida pueda reaccionar inmediatamente ante lluvia.

---

# 142. Sensor de lluvia como fuente de interrupción

La arquitectura deberá permitir definir:

```text
Sensor:
WH-SP-RG

Function:
Rain Event

GPIO:
XX

Interrupt:
Enabled

Trigger:
FALLING

Debounce:
100 ms

Wake-up:
Enabled
```

---

# 143. Debounce de interrupciones

Las entradas de pulsos deberán soportar:

```text
Debounce
Minimum pulse width
Pulse counter
Interrupt filter
```

Esto será fundamental para:

```text
pluviómetros
anemómetros
sensores reed
```

---

# 144. Contadores de pulsos

SEMA deberá disponer de un motor genérico:

```text
Pulse Counter
```

que pueda utilizarse para:

```text
anemómetro
pluviómetro
caudalímetro
contador energético
sensor de pulsos
```

Cada contador deberá permitir configurar:

```text
GPIO
Pull-up
Pull-down
Edge
Debounce
Pulses/unit
Unit
Maximum frequency
```

---

# 145. Wake-up + contador de pulsos

Cuando sea posible, los pulsos deberán poder contabilizarse incluso durante modos de bajo consumo compatibles con el hardware.

Esto permitirá:

```text
Deep Sleep
   ↓
Rain pulse
   ↓
Wake
   ↓
Recuperar contador
```

La implementación concreta deberá depender de las capacidades del ESP32 utilizado.

---

# 146. Política de suspensión

SEMA deberá permitir seleccionar qué eventos pueden despertar la estación.

Ejemplo:

```text
Wake sources

[x] RTC
[x] Rain
[x] Lightning
[x] External INT
[x] Button
[ ] Motion
[ ] CAN
[ ] UART
```

---

# 147. Configuración de sleep desde web

La página deberá mostrar:

```text
Low Power
────────────────────

Modo:
Normal

Sleep:
Enabled

Sleep after:
60 s

Wake RTC:
300 s

Wake on Rain:
Enabled

Wake on External INT:
Enabled

Wake on Button:
Enabled
```

---

# 148. Reset de configuración

SEMA deberá disponer de un mecanismo seguro para:

```text
Reset Configuration
```

Opciones:

```text
Reset network
Reset sensors
Reset hardware mapping
Reset all
Factory reset
```

---

# 149. Reset parcial

El usuario deberá poder restablecer solamente:

```text
Wi-Fi
```

sin borrar:

```text
sensores
calibración
históricos
```

También:

```text
Sensores
```

sin modificar:

```text
red
usuarios
OTA
```

---

# 150. Factory Reset

Deberá existir:

```text
Factory Reset
```

que restaure:

```text
configuración
red
sensores
buses
módulos
alarmas
servicios
```

pero deberá tratarse cuidadosamente el almacenamiento histórico.

Se deberá preguntar:

```text
¿Eliminar también históricos?

[ NO ]
[ SÍ ]
```

---

# 151. Reset mediante botón físico

Se deberá permitir configurar un GPIO como:

```text
FACTORY RESET BUTTON
```

Ejemplo:

```text
Presionar 5 segundos:
Reset network

Presionar 10 segundos:
Factory reset
```

El tiempo deberá ser configurable.

---

# 152. Reset de emergencia

En caso de configuración incorrecta:

```text
Boot
 ↓
Detectar configuración inválida
 ↓
Safe Mode
```

El Safe Mode deberá permitir:

```text
conectarse
reparar configuración
hacer rollback
```

sin intentar inicializar todos los periféricos problemáticos.

---

# 153. Safe Mode

SEMA deberá disponer de:

```text
NORMAL MODE
SAFE MODE
MAINTENANCE MODE
```

### Safe Mode

Solamente:

```text
Core
Network
Web
Configuration
Diagnostics
```

Deberán iniciarse los módulos críticos mínimos.

---

# 154. Watchdog + Safe Mode

Si una configuración provoca múltiples reinicios:

```text
BOOT
 ↓
CRASH
 ↓
BOOT
 ↓
CRASH
 ↓
BOOT
 ↓
CRASH
```

SEMA deberá detectar:

```text
BOOT LOOP
```

y entrar automáticamente en:

```text
SAFE MODE
```

---

# 155. Gestión de errores de sensores

Un sensor que produzca errores repetidos podrá pasar temporalmente a:

```text
DEGRADED
```

en lugar de reiniciar toda la estación.

Ejemplo:

```text
SHT40
 │
 ├── Timeout × 1
 ├── Timeout × 2
 ├── Timeout × 3
 └── Sensor Error
```

SEMA podrá:

```text
reinicializar sensor
```

sin reiniciar todo el ESP32.

---

# 156. Reinicio de periféricos

Los buses deberán permitir:

```text
Reset sensor
Reset bus
Power cycle sensor
Reinitialize driver
```

cuando el hardware lo permita.

Esto será especialmente útil para sensores I²C que queden bloqueados.

---

# 157. Protección del bus I²C

El sistema deberá detectar:

```text
SDA stuck LOW
SCL stuck LOW
No ACK
Address conflict
```

Y podrá intentar:

```text
I²C bus recovery
```

antes de reiniciar el ESP32.

---

# 158. Health Monitor

SEMA deberá tener un módulo central:

```text
Health Monitor
```

que supervise:

```text
CPU
RAM
Flash
Tasks
Sensors
Buses
Network
Storage
Battery
External services
Watchdog
Temperature
```

Estado general:

```text
HEALTHY
WARNING
DEGRADED
CRITICAL
```

---

# 159. Métricas internas

SEMA deberá exponer:

```text
CPU usage
Free heap
Minimum free heap
Task stack high water mark
Wi-Fi RSSI
Packets
HTTP errors
MQTT errors
Sensor errors
I²C errors
RS485 errors
CAN errors
Storage usage
Queue size
```

Estas métricas deberán estar disponibles mediante:

```text
Web
API
Diagnóstico
```

---

# 160. API de salud

Ejemplo:

```text
GET /api/v1/health
```

Respuesta:

```json
{
  "status": "HEALTHY",
  "uptime": 184322,
  "free_heap": 182440,
  "wifi_rssi": -61,
  "sensors": {
    "total": 12,
    "online": 11,
    "error": 1
  },
  "external_services": {
    "mqtt": "ONLINE",
    "windy": "ONLINE",
    "thingspeak": "ONLINE"
  }
}
```

---

# 161. API de configuración

La configuración deberá poder consultarse mediante:

```text
GET /api/v1/config
```

pero las operaciones de modificación deberán requerir autenticación.

Ejemplo:

```text
PUT /api/v1/config/sensors/{id}
```

---

# 162. API de diagnóstico

Deberá existir:

```text
GET /api/v1/diagnostics
```

que entregue información como:

```text
firmware
hardware
reset reason
uptime
memory
tasks
buses
sensor states
network
storage
power
```

---

# 163. Autenticación de API

SEMA deberá soportar diferentes mecanismos según el nivel de seguridad requerido:

```text
API Key
Bearer Token
Basic Authentication
Session Authentication
```

Para instalaciones profesionales deberá priorizarse:

```text
HTTPS
Bearer Token
API Key por aplicación
```

---

# 164. API Keys

El administrador deberá poder crear diferentes claves.

Ejemplo:

```text
Home Assistant
    API Key #1

Servidor Central
    API Key #2

Aplicación móvil
    API Key #3
```

Cada clave podrá tener permisos:

```text
READ
WRITE
CONFIG
DIAGNOSTIC
ADMIN
```

---

# 165. Revocación de API Keys

Desde la web:

```text
API Keys
 │
 ├── Home Assistant ✓
 ├── Server ✓
 ├── Mobile ✓
 └── Old application ✗
```

Una clave revocada no deberá poder acceder.

---

# 166. CORS

La API deberá poder configurar:

```text
CORS
```

permitiendo:

```text
disabled
same-origin
specific origins
all
```

El valor por defecto deberá ser restrictivo.

---

# 167. Rate Limiting

La API deberá disponer de límites.

Ejemplo:

```text
Requests/minute
Requests/hour
```

Esto evitará que una aplicación mal configurada sobrecargue el ESP32.

---

# 168. MQTT Discovery

Para integraciones IoT, SEMA podrá incorporar:

```text
MQTT Discovery
```

especialmente para:

```text
Home Assistant
```

Cada sensor podrá publicarse automáticamente.

Ejemplo conceptual:

```text
sema/SEMA-001/temperature
sema/SEMA-001/humidity
sema/SEMA-001/pressure
```

y sus metadatos correspondientes.

---

# 169. Integración con Home Assistant

SEMA deberá poder funcionar como:

```text
MQTT Device
```

y publicar:

```text
Temperature
Humidity
Pressure
Rain
Wind
UV
Solar
CO₂
Battery
Status
```

Los sensores habilitados deberán aparecer dinámicamente.

---

# 170. Integración con Node-RED

La API JSON y MQTT deberán estar diseñados para facilitar integración con:

```text
Node-RED
```

Ejemplo:

```text
SEMA
 ↓
MQTT
 ↓
Node-RED
 ↓
Automatización
 ↓
Base de datos
```

---

# 171. Integración con InfluxDB / Grafana

SEMA deberá poder integrarse indirectamente mediante:

```text
MQTT
HTTP
API
```

para sistemas como:

```text
InfluxDB
Grafana
```

No será necesario que el ESP32 ejecute directamente una base de datos.

---

# 172. Exportación de datos

La estación deberá poder exportar históricos como:

```text
JSON
CSV
```

y posteriormente:

```text
Parquet
```

si existe suficiente capacidad en un servidor central.

---

# 173. Formato de datos canónico

SEMA deberá definir un formato interno común.

Ejemplo:

```json
{
  "station_id": "SEMA-001",
  "sensor_id": "TEMP_EXT",
  "measurement": "temperature",
  "value": 24.7,
  "unit": "degC",
  "timestamp": "2026-09-30T15:00:00Z",
  "quality": "VALID"
}
```

Todos los publicadores deberán recibir este formato.

---

# 174. Separación entre medición y publicación

Nunca deberá existir:

```text
Sensor → ThingSpeak
```

sino:

```text
Sensor
 ↓
Measurement Engine
 ↓
Canonical Data Model
 ↓
Storage
 ↓
Publishers
```

Esto permitirá cambiar de servicio sin modificar los sensores.

---

# 175. Sincronización de múltiples servicios

SEMA deberá poder publicar simultáneamente:

```text
PWSWeather ✓
ThingSpeak ✓
Weathercloud ✓
Windy ✓
MQTT ✓
Servidor propio ✓
```

sin duplicar la lógica de sensores.

---

# 176. Intervalos independientes

Cada servicio deberá tener su propio intervalo.

Ejemplo:

```text
Sensores:
5 s

Almacenamiento:
10 s

MQTT:
10 s

Servidor:
30 s

ThingSpeak:
60 s

Weathercloud:
10 min

Windy:
configurable
```

No deberá existir un único intervalo global obligatorio.

---

# 177. Priorización de comunicaciones

En estaciones alimentadas por batería:

```text
Prioridad alta:
Alarmas

Prioridad media:
Servidor

Prioridad baja:
Servicios externos
```

Esto permitirá ahorrar energía.

---

# 178. Publicación condicionada

El usuario deberá poder configurar:

```text
Enviar siempre
Enviar si cambia
Enviar por intervalo
Enviar por evento
Enviar cuando despierte
```

Ejemplo:

```text
Rain:
Enviar inmediatamente

Temperature:
Cada 5 min

Battery:
Cada 10 min

Diagnostic:
Cada 1 hora
```

---

# 179. Compresión y ahorro de datos

Para enlaces LoRa o redes limitadas, SEMA deberá poder utilizar:

```text
Payload compacto
Binario
CBOR
JSON reducido
```

El formato completo JSON deberá seguir disponible para API local.

---

# 180. Offline First

SEMA deberá considerarse:

```text
OFFLINE FIRST
```

La ausencia de Internet no deberá impedir:

```text
medir
procesar
almacenar
mostrar localmente
generar alarmas
```

Internet deberá considerarse un servicio adicional.

---

# 181. Política de pérdida de Internet

Cuando:

```text
Internet ❌
```

SEMA deberá:

```text
continuar midiendo
      ↓
guardar
      ↓
marcar servicios externos OFFLINE
      ↓
reintentar
      ↓
sincronizar cuando vuelva
```

---

# 182. Sincronización después de recuperación

Al volver Internet:

```text
Internet ✓
    ↓
NTP
    ↓
Servicios
    ↓
Enviar pendientes
```

El usuario deberá poder configurar si los datos históricos pendientes se transmiten o solamente se envían nuevas mediciones.

---

# 183. Monitoreo de conectividad

SEMA deberá distinguir:

```text
Wi-Fi conectado
```

de:

```text
Internet disponible
```

y:

```text
Servicio externo disponible
```

Son estados diferentes.

Ejemplo:

```text
Wi-Fi      ✓
Gateway    ✓
Internet   ✓
Windy      ✗
MQTT       ✓
```

---

# 184. DNS y conectividad

El sistema deberá poder diagnosticar:

```text
DNS
Gateway
Internet
HTTPS
NTP
```

Esto facilitará detectar problemas externos.

---

# 185. TLS

Los servicios externos deberán utilizar:

```text
HTTPS
```

cuando el servicio lo soporte.

SEMA deberá poder utilizar:

```text
CA certificate
Certificate bundle
Fingerprint
Secure connection
```

según las capacidades del ESP32 y del servicio.

---

# 186. Credenciales

Las credenciales de servicios externos no deberán almacenarse en texto plano en archivos exportados por defecto.

Ejemplo:

```text
ThingSpeak API Key
Windy password
Weathercloud key
MQTT password
Wi-Fi password
```

Deberán marcarse como:

```text
SECRET
```

---

# 187. Exportación segura

Al exportar configuración:

```text
[ ] Incluir credenciales
```

Por defecto:

```text
NO
```

La configuración exportada deberá contener:

```json
{
  "thingspeak": {
    "enabled": true,
    "channel_id": 123456,
    "api_key": "***REDACTED***"
  }
}
```

---

# 188. Ubicación de la estación

SEMA deberá disponer de configuración geográfica.

```text
Latitude
Longitude
Elevation
Timezone
Location name
Country
Region
```

La ubicación deberá utilizarse para:

```text
Windy
PWSWeather
Weathercloud
sunrise/sunset
radiación
reportes
```

---

# 189. Privacidad de ubicación

La ubicación exacta deberá poder ocultarse o redondearse para servicios públicos.

Ejemplo:

```text
Ubicación interna:
-31.xxxxxxxx
-60.xxxxxxxx

Ubicación pública:
-31.xx
-60.xx
```

---

# 190. Astronomía básica

SEMA podrá calcular:

```text
Sunrise
Sunset
Solar noon
Day length
Dawn
Dusk
```

Esto permitirá funcionalidades futuras como:

```text
sleep schedule
solar analysis
radiation analysis
```

---

# 191. Índices meteorológicos derivados

SEMA podrá calcular:

```text
Dew Point
Heat Index
Wind Chill
Feels Like
VPD
Absolute Humidity
Sea Level Pressure
Rain Rate
Daily Rain
Monthly Rain
Wind Gust
Wind Run
```

Los cálculos deberán utilizar unidades normalizadas.

---

# 192. Estadísticas meteorológicas

El sistema podrá calcular:

```text
Minimum
Maximum
Average
Median
Standard deviation
Accumulated
Count
```

por:

```text
hora
día
semana
mes
año
```

---

# 193. Récords meteorológicos

SEMA podrá mantener:

```text
Temperature maximum
Temperature minimum
Wind maximum
Rain maximum
Pressure maximum
Pressure minimum
UV maximum
Solar maximum
```

Cada récord deberá guardar:

```text
value
timestamp
sensor
quality
```

---

# 194. Resumen diario

SEMA podrá generar:

```text
Daily Summary
```

con:

```text
T min
T max
T average
Humidity min/max
Pressure min/max
Rain
Wind max
Wind average
Solar energy
UV max
CO₂ average
```

---

# 195. Integridad de históricos

Los datos almacenados deberán tener mecanismos para detectar corrupción.

Podrá utilizarse:

```text
CRC
Checksum
Sequence number
Timestamp validation
```

especialmente en almacenamiento externo.

---

# 196. Protección ante pérdida de energía

Antes de una pérdida de alimentación, cuando sea posible:

```text
guardar estado
cerrar archivos
actualizar contador
```

El diseño deberá minimizar corrupción de almacenamiento.

---

# 197. Brownout

SEMA deberá detectar:

```text
Brownout
```

cuando el microcontrolador/hardware lo permita.

El evento deberá registrarse como:

```text
RESET_BROWNOUT
```

y no confundirse con:

```text
WATCHDOG
```

---

# 198. Protección de configuración

La configuración deberá almacenarse con:

```text
schema version
CRC/checksum
backup
```

Ejemplo:

```text
config.json
config.backup.json
```

Cuando sea posible, se deberán utilizar mecanismos atómicos de escritura.

---

# 199. Rollback

Ante una configuración inválida:

```text
Nueva configuración
       ↓
Validación
       ↓
Aplicar
       ↓
Fallo
       ↓
Rollback
```

La estación deberá poder regresar a la última configuración válida.

---

# 200. Arquitectura global final

La arquitectura conceptual de SEMA deberá evolucionar hacia:

```text
                              ┌─────────────────────┐
                              │   SERVICIOS CLOUD   │
                              │                     │
                              │ PWSWeather          │
                              │ ThingSpeak          │
                              │ Weathercloud        │
                              │ Windy               │
                              │ Custom APIs         │
                              └──────────┬──────────┘
                                         │
                                  HTTPS / MQTT
                                         │
┌────────────────────────────────────────┼──────────────────────────────┐
│                                      SEMA                             │
│                                                                       │
│  ┌──────────────┐       ┌──────────────────────┐                    │
│  │   HARDWARE   │──────▶│   MEASUREMENT ENGINE │                    │
│  └──────────────┘       └──────────┬───────────┘                    │
│                                    │                                  │
│                           ┌────────┴────────┐                         │
│                           │ CANONICAL DATA  │                         │
│                           └────────┬────────┘                         │
│                                    │                                  │
│               ┌────────────────────┼───────────────────┐              │
│               │                    │                   │              │
│           Storage                API               Publishers         │
│               │                    │                   │              │
│          Histórico              JSON          ┌────────┼────────┐     │
│                                              │        │        │     │
│                                           MQTT     HTTP      Cloud   │
│                                                                       │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                        SYSTEM CORE                             │ │
│  │                                                                │ │
│  │ Configuration │ Health │ Watchdog │ NTP │ Security │ OTA       │ │
│  │                                                                │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                       │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                      POWER MANAGER                             │ │
│  │                                                                │ │
│  │ Active │ Light Sleep │ Deep Sleep │ RTC │ GPIO INT │ Rain Wake │ │
│  │                                                                │ │
│  └────────────────────────────────────────────────────────────────┘ │
└───────────────────────────────────────────────────────────────────────┘
```

---

# 201. Flujo completo de una estación autónoma

El comportamiento ideal para una estación solar deberá ser:

```text
                     ┌─────────────┐
                     │ DEEP SLEEP  │
                     └──────┬──────┘
                            │
                  ┌─────────┴─────────┐
                  │                   │
               RTC Wake           Rain INT
                  │                   │
                  └─────────┬─────────┘
                            ▼
                       INITIALIZE
                            │
                            ▼
                    CHECK HARDWARE
                            │
                            ▼
                       READ SENSORS
                            │
                            ▼
                     VALIDATE DATA
                            │
                            ▼
                    CALCULATE VALUES
                            │
                            ▼
                     STORE LOCALLY
                            │
                            ▼
                     CHECK NETWORK
                            │
                   ┌────────┴────────┐
                   │                 │
                 ONLINE            OFFLINE
                   │                 │
                   ▼                 │
             PUBLISH DATA             │
                   │                 │
                   └────────┬────────┘
                            ▼
                     UPDATE STATUS
                            │
                            ▼
                      ENERGY CHECK
                            │
                            ▼
                       DEEP SLEEP
```

---

# 202. Arquitectura de prioridades

El firmware deberá priorizar:

```text
PRIORIDAD 1
Core / seguridad / watchdog

PRIORIDAD 2
Adquisición de sensores

PRIORIDAD 3
Procesamiento y almacenamiento

PRIORIDAD 4
Alarmas y eventos

PRIORIDAD 5
API local

PRIORIDAD 6
Comunicación

PRIORIDAD 7
Servicios externos
```

Una falla de un servicio externo nunca deberá detener la adquisición de sensores.

---

# 203. Principio de no bloqueo

Ningún servicio externo deberá poder bloquear:

```text
SensorTask
MeasurementTask
StorageTask
Watchdog
```

Por ejemplo:

```text
Windy tarda 10 segundos
```

no deberá provocar:

```text
❌ SensorTask bloqueada
❌ Watchdog
❌ pérdida de mediciones
```

La comunicación deberá ejecutarse mediante tareas/colas apropiadas.

---

# 204. Event Bus

SEMA deberá disponer de un bus interno de eventos.

Ejemplo:

```text
RAIN_START
     │
     ├── Storage
     ├── Alarm
     ├── MQTT
     ├── Webhook
     ├── Dashboard
     └── Wake Manager
```

Esto permitirá que múltiples módulos reaccionen al mismo evento sin estar acoplados entre sí.

---

# 205. Event Manager

Eventos posibles:

```text
SensorEvent
RainEvent
LightningEvent
BatteryEvent
NetworkEvent
AlarmEvent
SystemEvent
WakeEvent
SleepEvent
```

Todos deberán utilizar una estructura común.

---

# 206. Scheduler

SEMA deberá disponer de un scheduler interno para tareas periódicas.

Ejemplo:

```text
Task Scheduler

Read SHT40          10 s
Read BME280         10 s
Read BH1750         30 s
Read PMS5003        60 s
Upload ThingSpeak   60 s
Upload Windy        60 s
Upload Weathercloud 600 s
NTP                 3600 s
Diagnostics         60 s
```

Los intervalos deberán ser configurables.

---

# 207. Scheduler basado en capacidades

Las tareas deberán crearse dinámicamente según los módulos habilitados.

Si:

```text
PMS5003 = disabled
```

no deberá existir una tarea permanente para ese sensor.

---

# 208. Recursos dinámicos

SEMA deberá evitar reservar recursos innecesariamente.

Ejemplo:

```text
No LoRa
→ No LoRa task

No SD
→ No SD task

No Zigbee
→ No Zigbee task

No PMS5003
→ No PMS task
```

Esto reducirá:

```text
RAM
CPU
consumo energético
complejidad
```

---

# 209. Perfil de instalación

La estación podrá tener perfiles:

```text
HOME
WEATHER
AGRICULTURE
INDUSTRIAL
REMOTE
SOLAR
AIR_QUALITY
RESEARCH
```

Cada perfil podrá activar módulos iniciales, pero el usuario deberá poder modificarlos.

---

# 210. Autodetección + configuración guiada

La configuración inicial ideal será:

```text
1. Detectar ESP32
2. Detectar buses
3. Detectar expansores
4. Detectar sensores
5. Mostrar dispositivos
6. Asociar funciones
7. Configurar ubicación
8. Configurar red
9. Configurar energía
10. Configurar servicios
11. Validar
12. Guardar
```

---

# 211. Asistente de configuración

La web deberá disponer de:

```text
SETUP WIZARD
```

pero nunca deberá reemplazar la configuración avanzada.

```text
Configuración rápida
```

y:

```text
Configuración avanzada
```

deberán coexistir.

---

# 212. Modo experto

Los usuarios técnicos deberán poder acceder a:

```text
GPIO
timers
I²C
SPI
UART
interrupts
DMA
ADC
bus timing
RS485
CAN
sleep
watchdog
```

mientras que un usuario normal podrá trabajar solamente con:

```text
Temperatura
Humedad
Viento
Lluvia
etc.
```

---

# 213. Documentación automática

La interfaz podrá generar una página:

```text
System Configuration Report
```

mostrando:

```text
Hardware
Firmware
Sensors
GPIO
Buses
Expansors
Calibration
Network
Services
Energy
```

Esto será útil para mantenimiento.

---

# 214. Inventario de hardware

SEMA deberá mantener un inventario:

```text
ESP32-S3
MCP23017 #1
ADS1115 #1
74HC595 #1
SHT40
BME280
PMS5003
RS485 ADM2483
LoRa
Zigbee
SD
```

Cada dispositivo tendrá:

```text
ID
tipo
modelo
estado
ubicación
firmware
configuración
```

---

# 215. Estado de mantenimiento

Cada componente podrá indicar:

```text
INSTALLED
ACTIVE
DISABLED
FAULT
MAINTENANCE
REPLACEMENT_REQUIRED
```

Esto permitirá construir posteriormente mantenimiento predictivo.

---

# 216. Contadores de funcionamiento

Para componentes importantes:

```text
runtime
read_count
error_count
restart_count
communication_errors
```

Ejemplo:

```text
RS485 #1
Requests: 2,345,120
Errors: 21
CRC Errors: 3
Timeouts: 18
```

---

# 217. Observabilidad

SEMA deberá diseñarse para poder responder:

```text
¿Qué está ocurriendo?
¿Por qué ocurrió?
¿Cuándo ocurrió?
¿Qué sensor lo provocó?
¿Qué módulo estaba ejecutándose?
¿Se perdió información?
```

Esto será más importante que simplemente mostrar un valor.

---

# 218. Registro estructurado

Los logs deberán tener:

```text
timestamp
level
module
event
message
code
```

Ejemplo:

```text
2026-09-30T15:02:12Z
WARN
RS485
MODBUS_TIMEOUT
Slave=4
```

---

# 219. Niveles de log

```text
TRACE
DEBUG
INFO
NOTICE
WARNING
ERROR
CRITICAL
```

El nivel deberá ser configurable.

---

# 220. Logs remotos

Los logs podrán publicarse mediante:

```text
MQTT
HTTP
Servidor central
```

pero el almacenamiento local deberá ser suficiente para diagnosticar fallos cuando no exista conexión.

---

# 221. Seguridad operacional

Una pérdida de:

```text
Internet
Wi-Fi
MQTT
Windy
ThingSpeak
Weathercloud
PWSWeather
```

no deberá provocar:

```text
reinicio
pérdida de configuración
pérdida de sensores
```

---

# 222. Principio final ampliado

SEMA deberá ser:

```text
┌─────────────────────────────────────────────────┐
│                     SEMA                        │
│                                                 │
│   MEDIR                                         │
│      ↓                                          │
│   VALIDAR                                       │
│      ↓                                          │
│   PROCESAR                                      │
│      ↓                                          │
│   ALMACENAR                                     │
│      ↓                                          │
│   PUBLICAR                                      │
│      ↓                                          │
│   SINCRONIZAR                                   │
│      ↓                                          │
│   DORMIR                                        │
│      ↓                                          │
│   DESPERTAR POR EVENTO                          │
│      ↓                                          │
│   VOLVER A MEDIR                                │
│                                                 │
│   TODO CONFIGURABLE DESDE LA WEB                │
└─────────────────────────────────────────────────┘
```

La estación deberá continuar funcionando aun cuando:

```text
Internet ❌
Cloud ❌
MQTT ❌
Servidor ❌
Un sensor ❌
Un bus ❌
```

Siempre que el Core y los recursos necesarios para las funciones restantes estén operativos.

---

# 223. Objetivo de arquitectura final

El objetivo de SEMA no será únicamente:

```text
"Leer sensores."
```

sino:

```text
┌───────────────────────────────────────────────────────┐
│                 SEMA PLATFORM                         │
│                                                       │
│ Hardware configurable                                 │
│       +                                               │
│ Sensores intercambiables                              │
│       +                                               │
│ Buses configurables                                   │
│       +                                               │
│ Expansores configurables                              │
│       +                                               │
│ Data Engine                                           │
│       +                                               │
│ API JSON                                              │
│       +                                               │
│ MQTT                                                  │
│       +                                               │
│ Servicios externos                                    │
│       +                                               │
│ Almacenamiento                                        │
│       +                                               │
│ Energy Manager                                        │
│       +                                               │
│ Watchdog / Health Monitor                             │
│       +                                               │
│ Sleep / Wake Manager                                  │
│       +                                               │
│ Event Bus                                             │
│       +                                               │
│ Web configurable                                      │
│                                                       │
│                    =                                  │
│                                                       │
│       PLATAFORMA METEOROLÓGICA MODULAR                │
└───────────────────────────────────────────────────────┘
```

La configuración deberá ser suficientemente abstracta para que agregar un nuevo sensor, bus, protocolo o servicio externo no obligue a modificar la arquitectura central.

El objetivo final no es construir una única estación meteorológica, sino desarrollar una **plataforma SEMA capaz de adaptarse a diferentes necesidades mediante configuración, módulos y hardware intercambiable sin necesidad de modificar el firmware para cada instalación**.
