# Proyecto_Final_Control_De_Compuertas
 
![Diagrama](https://github.com/sillHD/Proyecto_Final_Control_De_Compuertas/blob/main/image.jpg)

Documentación del Proyecto STM32 Nucleo L476RG

1. Introducción

Este documento describe la implementación de un sistema basado en la placa STM32 Nucleo L476RG, que gestiona la comunicación entre un módulo WiFi, una pantalla OLED SSD1306 y la interacción de un teclado y botones físicos para controlar el estado de un LED.

2. Hardware Utilizado

STM32 Nucleo L476RG: Microcontrolador principal del sistema.

Módulo WiFi (ESP): Se encarga de la comunicación inalámbrica.

Pantalla OLED SSD1306: Muestra el estado de la puerta.

Teclado: Permite la entrada de datos.

Botones físicos: Controlan el estado de un LED.

LED indicador: Indica cambios en el sistema.

3. Software y Configuración

Entorno de desarrollo: VSCode con STM32CubeMX y CMake.

Lenguaje de programación: C.

Comunicación serie: UART1, UART2 y UART3 utilizados para la comunicación con los diferentes periféricos.

4. Implementación del Sistema

El sistema se divide en tres módulos principales que interactúan a través de UART:

4.1. UART3 - Módulo WiFi

Se encarga de la transmisión de datos.

Envía y recibe datos relacionados con la conexión WiFi.

Recibe el estado de actualización del sistema.

Envía datos provenientes del teclado.

4.2. UART2 - Entrada de Teclado

Recibe datos ingresados por el teclado.

Envía información al módulo WiFi para su procesamiento.

Actualiza la pantalla OLED SSD1306 con el estado actual del sistema.

4.3. UART1 - Control del LED

Detecta la interacción con los botones físicos.

Cambia el estado del LED al presionar los botones.

Envía el estado del LED para su monitoreo.

5. Funcionamiento General

El teclado envía entradas a través de UART2.

La pantalla SSD1306 se actualiza con la información del teclado.

El módulo WiFi transmite los datos relevantes a través de UART3.

Los botones controlan el estado del LED mediante UART1.

El estado del LED se transmite para su visualización.

6. Conclusiones

El sistema desarrollado permite la comunicación fluida entre los diferentes módulos, garantizando la actualización del estado del sistema en la pantalla OLED y la interacción con el usuario mediante teclas y botones. Se pueden implementar mejoras futuras como la optimización de la comunicación serie o la integración con otros sensores.
pasamelo a leguaje c
