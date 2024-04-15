/* Equipo 2 Laboratorio 2 Unidad 2 */

// FileName:        system_functions.c
// Dependencies:    system.h
// Processor:       MSP432
// Board:           MSP43P401R
// Program Version: CCS V8.3 TI
// Company:         Texas Instruments
// Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
// Co-authors:      Jorge Gabriel Lozano Rodriguez, Pedro Mancinas Hernández y Raúl Montes Montes.
// Updated:         12/2018

#include "system.h"
#include "Drivers/BSP.h"         // Incluye los drivers proporcionados. Esto es para las funciones de interrupción solamente.
#define SYSTEM_CLOCK 48000000    // Aquí debe estar la frecuencia de MCLK (velocidad del sistema en Hz).

bool delay = false;              // Variable necesaria para el fucnionamiento del Time.r

void Timer32_INT1(void);         // Función de interrupción.
void Delay_ms(uint32_t time);    // Función de delay.


/* FUNCTION *********************************************************************************
*
* Function Name    : System_InicialiceIO
* Returned Value   : None.
* Comments         :
*
********************************************************************************************/

void System_InicialiceIO(void)
{
    GPIO_init_board();
}

/* FUNCTION ******************************************************************************
*
* Function Name    : System_InicialiceUART
* Returned Value   : None.
* Comments         :
*
*****************************************************************************************/
void System_InicialiceUART(void)
{
    UART_init();
}

/* FUNCTION ******************************************************************************
*
* Function Name    : funcion_inicial
* Returned Value   : None.
* Comments         :
*
*****************************************************************************************/
void funcion_inicial(void)
{
    GPIO_setOutput(BSP_LED1_PORT, BSP_LED1, 0);              // Apagar los led´s de manera inicial.
    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 0);
    GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 0);
    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 0);
 //   UART_putsf(MAIN_UART, "Hola version 8.\r\n");          // Impresiones inecesarias a fines de los objetivos.
 //   printf("Hello\n");
}

/* FUNCTION ******************************************************************************
*
* Function Name    : process_events
* Returned Value   : None.
* Comments         :
*
*****************************************************************************************/

void process_events(void)
{

    TIMER32_1->CONTROL |= (0x00000001) | (0x00000002);               // Modo de disparo único, tamaño de 32 bits, prescaler de 1.
    Int_registerInterrupt(INT_T32_INT1, Timer32_INT1);               // Registra y habilita la interrupción deseada.
    Int_enableInterrupt(INT_T32_INT1);

    static bool text_flag = TRUE;                                    // Variables de manejo de acceso e iteraciones.
    static int activo = 0, cuenta_Ciclos = 0, cuenta_Push = 0;

    while (1)                                                        // Inicio primer while.
    {

        cuenta_Ciclos = 1;                                           // inicializamos la variable para acceso al while.

        if (text_flag)                                              // Comprobacion para una unica impresion del texto.
        {
            UART_putsf(MAIN_UART, "PRESIONE CUALQUIER TECLA PARA INICIAR!!!\r\n");
            text_flag = FALSE;
        }

        if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)                    // Comprueba si el bit correspondiente a la interrupción de recepción (RXIFG) está activo.
        {
            activo = 1;
            EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;                  // Limpia TXIFG para la siguiente iteración.
        }
        if (activo)                                              // Solo entra si se detecta que se puso una tecla
        {

            UART_putsf(MAIN_UART, "Iniciando programa.\r\n");
            GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 1);          // Se inicia con el Led azul.

            while (cuenta_Ciclos)
            {
                if ((GPIO_getInputPinValue(BSP_BUTTON1_PORT, BSP_BUTTON1) != BOARD_BUTTON_NORMAL_STATE))  // Entra cuando se presiona el botón 1.
                {
                    cuenta_Ciclos++;                              // Conteo de las iteraiciones.

                    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 0);  // Enciende el Led rojo y apaga el azul se imprime pausa.
                    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 1);
                    UART_putsf(MAIN_UART, "PAUSA.\r\n");

                    delay = true;                                // Pone en alto la variable que controla la interrupcion del Timer.

                    while (delay)
                    {
                        Delay_ms(3000);                          // Se espera 3 segundos.
                    }

                    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 0);   // Enciende el led azul y apaga el rojo
                    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 1);
                    UART_putsf(MAIN_UART, "Programa ejecutandose.\r\n");

                    if (cuenta_Ciclos >= 7)                       //Verifica si la cantidad de ciclos es mayor a 6, que que se empezo en 1.
                    {
                        cuenta_Ciclos = 0;
                        cuenta_Push = 0;
                    }
                    while (GPIO_getInputPinValue(BSP_BUTTON1_PORT, BSP_BUTTON1) != BOARD_BUTTON_NORMAL_STATE); // Evita cambios inesperados si se deja presionado el boton 1
                }

                if (GPIO_getInputPinValue(BSP_BUTTON2_PORT, BSP_BUTTON2) != BOARD_BUTTON_NORMAL_STATE)          // Verifica si se presiono el boton 2
                {
                    cuenta_Push++;                                                                              // Conteo de segundo boton
                    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 1);                                                 //Enciende led amarillo(rojo y verde) y apaga azul
                    GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 1);
                    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 0);
                    UART_putsf(MAIN_UART, "Presione de nuevo para apagar\r\n");
                    while (GPIO_getInputPinValue(BSP_BUTTON2_PORT, BSP_BUTTON2) != BOARD_BUTTON_NORMAL_STATE); // Evita cambios inesperados si se deja presionado el boton 2
                }

                if (cuenta_Push == 1)                                                                           // Verificacion si se presiono el boton 2
                {
                    delay = true;                                // Pone en alto la variable que controla la interrupcion del Timer, para clavarse en el while.

                    BITBAND_PERI(TIMER32_1->CONTROL, TIMER32_CONTROL_ENABLE_OFS) = 1;                      // Habilita timer, luego la interrupción y carga valor de conteo a 3 segundos.
                    BITBAND_PERI(TIMER32_1->CONTROL, TIMER32_CONTROL_IE_OFS) = 1;
                    TIMER32_1->LOAD = 3000 * (__SYSTEM_CLOCK / 1000); // mS.
                    while (delay)                                                                           // Espera 3 segundos a que el usuario presione por segunda vez el boton.
                    {
                        if (GPIO_getInputPinValue(BSP_BUTTON2_PORT, BSP_BUTTON2) != BOARD_BUTTON_NORMAL_STATE)
                        {

                            GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 0);                                     // Apaga led amarillo.
                            GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 0);
                            while (GPIO_getInputPinValue(BSP_BUTTON2_PORT, BSP_BUTTON2) != BOARD_BUTTON_NORMAL_STATE);  // Evita cambios inesperados si se deja presionado el boton 2
                            cuenta_Push= 0;
                            goto salida;                                     // Goto directo al paso 5(Salida del programa)
                        }

                    }
                    UART_putsf(MAIN_UART, "Programa ejecutandose.\r\n");     // Si no se  presiona dos veces el boton dos se regres al el paso 3 (ejecucion del programa).
                    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 0);
                    GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 0);
                    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 1);
                    cuenta_Push= 0;
                }
            }
salida:               // Direccion del goto para salida del programma.
            GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 0);
            UART_putsf(MAIN_UART, "Programa terminado\r\n");
            activo = 0;                                                       // Desactiva señal que controla el if de ejecucion del sistema hasta que el usario ingrese una tecla.
            text_flag = TRUE;                                                // Pone en alto la bandera para poder imprimir el texto inicial(Regresar al paso 1).
        }
    }                                                                   // Fin while inicial
}

void Delay_ms(uint32_t time)
{
    BITBAND_PERI(TIMER32_1->CONTROL, TIMER32_CONTROL_ENABLE_OFS) = 1;  // Habilita timer, luego la interrupción y carga valor de conteo.
    BITBAND_PERI(TIMER32_1->CONTROL, TIMER32_CONTROL_IE_OFS) = 1;
    TIMER32_1->LOAD = time * (__SYSTEM_CLOCK / 1000); // mS.
    delay = true;                                                     // Se mantiene aqui para poder hacer la espera de 3 segundso sin que el usuario pueda mover algo.
    while (delay);
}

void Timer32_INT1(void)                                            // Interrupcion para desactivar el conteo.
{
    TIMER32_1->INTCLR = 0x00000000;
    delay = false;
}
