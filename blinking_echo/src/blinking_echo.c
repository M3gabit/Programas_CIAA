/** \brief Blinking_echo example source file
 **
 ** This is a mini example of the CIAA Firmware to test the periodical
 ** task excecution and serial port funcionality.
 ** To run this sample in x86 plataform you must enable the funcionality of
 ** uart device setting a value of une or more of folowing macros defined
 ** in header file modules/plataforms/x86/inc/ciaaDriverUart_Internal.h
 **/

/*==================[inclusions]=============================================*/
#include "os.h"               /* <= operating system header */
#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */
#include "blinking_echo.h"         /* <= own header */

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/** \brief File descriptor for digital input ports
 *
 * Device path /dev/dio/in/0
 */
static int32_t fd_in;

/** \brief File descriptor for digital output ports
 *
 * Device path /dev/dio/out/0
 */
static int32_t fd_out;

/** \brief File descriptor of the USB uart
 *
 * Device path /dev/serial/uart/1
 */
static int32_t fd_uart1;

/** \brief File descriptor of the RS232 uart
 *
 * Device path /dev/serial/uart/2
 */
//static int32_t fd_uart2;

/** \brief Periodic Task Counter
 *
 */
static uint32_t Periodic_Task_Counter;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

int main(void)
{
   StartOS(AppMode1);
   return 0;
}


void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}

/** \brief Initial task
 *
 * This task is started automatically in the application mode 1.
 */
TASK(InitTask)
{
   /* init CIAA kernel and devices */
   ciaak_start();

   //ciaaPOSIX_printf("Init Task...\n");

   /* open CIAA digital inputs */
   // PULSADORES
   fd_in = ciaaPOSIX_open("/dev/dio/in/0", ciaaPOSIX_O_RDONLY);

   /* open CIAA digital outputs */
   // LEDS
   fd_out = ciaaPOSIX_open("/dev/dio/out/0", ciaaPOSIX_O_RDWR);

   /* open UART connected to USB bridge (FT2232) */
   fd_uart1 = ciaaPOSIX_open("/dev/serial/uart/1", ciaaPOSIX_O_RDWR);

   /* open UART connected to RS232 connector */
   //fd_uart2 = ciaaPOSIX_open("/dev/serial/uart/2", ciaaPOSIX_O_RDWR);

   /* change baud rate for uart usb */
   ciaaPOSIX_ioctl(fd_uart1, ciaaPOSIX_IOCTL_SET_BAUDRATE, (void *)ciaaBAUDRATE_115200);

   /* change FIFO TRIGGER LEVEL for uart usb */
   ciaaPOSIX_ioctl(fd_uart1, ciaaPOSIX_IOCTL_SET_FIFO_TRIGGER_LEVEL, (void *)ciaaFIFO_TRIGGER_LEVEL3);

   /* activate example tasks */
   Periodic_Task_Counter = 0;
   //Activa la tarea PeriodicTask cada 200 ticks
   SetRelAlarm(ActivatePeriodicTask, 200, 200);

   /* Activates the SerialEchoTask task */
   ActivateTask(SerialEchoTask);

   /* end InitTask */
   TerminateTask();
}

/** \brief Serial Echo Task
 *
 * This tasks waits for input data from fd_uart1 and writes the received data
 * to fd_uart1 and fd_uart2. This taks alos blinkgs the output 5.
 *
 */
TASK(SerialEchoTask)
{
   char buf[20];   /* buffer for uart operation              */
   uint8_t outputs;  /* to store outputs status                */
   int32_t ret;      /* return value variable for posix calls  */
	char lr[] = "led_r\0";
   char lv[] = "led_v\0";
   char la[] = "led_a\0";
   char l1[] = "led_1\0";
   char l2[] = "led_2\0";
   char l3[] = "led_3\0";
   char respr[] = "Led r conmuto\n";
   char respv[] = "Led v conmuto\n";
   char respa[] = "Led a conmuto\n";
   char resp1[] = "Led 1 conmuto\n";
   char resp2[] = "Led 2 conmuto\n";
   char resp3[] = "Led 3 conmuto\n";

   ciaaPOSIX_printf("SerialEchoTask...\n");
   /* send a message to the world :) */
   char message[] = "Hi! :)\nSerialechotask: waiting for characters...\n";
   ciaaPOSIX_write(fd_uart1, message, ciaaPOSIX_strlen(message));

   while(1)
   {

   		//ciaaPOSIX_read(fd_out, &outputs, 1);
         //outputs ^= 0x20;
         //ciaaPOSIX_write(fd_out, &outputs, 1);

      


      ret = ciaaPOSIX_read(fd_uart1, buf, 20);

      //ciaaPOSIX_printf(comando);
      //ciaaPOSIX_printf(buf);

      if(ciaaPOSIX_strcmp(buf,lr) == 0)
      {
         ciaaPOSIX_read(fd_out, &outputs, 1);
         outputs ^= 0x01;
         ciaaPOSIX_write(fd_out, &outputs, 1);
         ciaaPOSIX_write(fd_uart1, respr, ciaaPOSIX_strlen(respr));

      };

      if(ciaaPOSIX_strcmp(buf,lv) == 0)
      {
         ciaaPOSIX_read(fd_out, &outputs, 1);
         outputs ^= 0x02;
         ciaaPOSIX_write(fd_out, &outputs, 1);
         ciaaPOSIX_write(fd_uart1, respv, ciaaPOSIX_strlen(respv));

      };

      if(ciaaPOSIX_strcmp(buf,la) == 0)
      {
         ciaaPOSIX_read(fd_out, &outputs, 1);
         outputs ^= 0x04;
         ciaaPOSIX_write(fd_out, &outputs, 1);
         ciaaPOSIX_write(fd_uart1, respv, ciaaPOSIX_strlen(respa));

      };

      if(ciaaPOSIX_strcmp(buf,l1) == 0)
      {
         ciaaPOSIX_read(fd_out, &outputs, 1);
         outputs ^= 0x20;
         ciaaPOSIX_write(fd_out, &outputs, 1);
         ciaaPOSIX_write(fd_uart1, resp1, ciaaPOSIX_strlen(resp1));

      };

      if(ciaaPOSIX_strcmp(buf,l2) == 0)
      {
         ciaaPOSIX_read(fd_out, &outputs, 1);
         outputs ^= 0x10;
         ciaaPOSIX_write(fd_out, &outputs, 1);
         ciaaPOSIX_write(fd_uart1, resp2, ciaaPOSIX_strlen(resp2));
      };
      
      if(ciaaPOSIX_strcmp(buf,l3) == 0)
      {
          ciaaPOSIX_read(fd_out, &outputs, 1);
          outputs ^= 0x08;
          ciaaPOSIX_write(fd_out, &outputs, 1);
          ciaaPOSIX_write(fd_uart1, resp3, ciaaPOSIX_strlen(resp3));
      };



      /*if(ret > 0)
      {
         ciaaPOSIX_write(fd_uart1, buf, ret);
      }*/
   }
}

/** \brief Periodic Task
 *
 * This task is activated by the Alarm ActivatePeriodicTask.
 * This task copies the status of the inputs bits 0..3 to the output bits 0..3.
 * This task also blinks the output 4
 */
TASK(PeriodicTask)
{
   
    uint8_t outputs = 0;

    ciaaPOSIX_read(fd_out, &outputs, 1);
    outputs ^= 0x08;
    ciaaPOSIX_write(fd_out, &outputs, 1);
   /* variables to store input/output status */
   

   TerminateTask();
}
