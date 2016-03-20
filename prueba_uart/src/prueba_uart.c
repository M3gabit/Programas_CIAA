
#include "os.h"               /* <= operating system header */
#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */
#include "prueba_uart.h"         /* <= own header */
#include "adc_dac.h"

static int32_t fd_in;

static int32_t fd_adc;

static int32_t fd_out;

static int32_t fd_uart1;

static uint32_t Periodic_Task_Counter;

static uint8_t x1 = 0;
static uint8_t x2 = 0;
static uint8_t x3 = 0;
static uint8_t x4 = 0;

static uint8_t num1 = 0;
static uint8_t num2 = 0;
static uint8_t num3 = 0;
static uint8_t num4 = 0;


static uint16_t hr_ciaaDac;
static uint16_t aux;


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

TASK(InitTask)
{
   ciaak_start();

   // PULSADORES
   fd_in = ciaaPOSIX_open("/dev/dio/in/0", ciaaPOSIX_O_RDONLY);

   // LEDS
   fd_out = ciaaPOSIX_open("/dev/dio/out/0", ciaaPOSIX_O_RDWR);

   /* open UART connected to USB bridge (FT2232) */
   fd_uart1 = ciaaPOSIX_open("/dev/serial/uart/1", ciaaPOSIX_O_RDWR);

   ciaaPOSIX_ioctl(fd_uart1, ciaaPOSIX_IOCTL_SET_BAUDRATE, (void *)ciaaBAUDRATE_115200);

   ciaaPOSIX_ioctl(fd_uart1, ciaaPOSIX_IOCTL_SET_FIFO_TRIGGER_LEVEL, (void *)ciaaFIFO_TRIGGER_LEVEL3);


   fd_adc = ciaaPOSIX_open("/dev/serial/aio/in/0", ciaaPOSIX_O_RDONLY);
   ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_SAMPLE_RATE, 100000);
   ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_3);

   ActivateTask(Analogic);


   /* activate example tasks */
   Periodic_Task_Counter = 0;
   //Activa la tarea PeriodicTask cada 200 ticks
   SetRelAlarm(ActivatePeriodicTask, 100, 100);

   //ActivateTask(SerialEchoTask);

   /* end InitTask */
   TerminateTask();
}

TASK(SerialEchoTask)
{
   char buf[20];   /* buffer for uart operation              */
   uint8_t outputs;  /* to store outputs status                */
   int32_t ret;      /* return value variable for posix calls  */
   char message[] = "Hi! :)\nSerialechotask: waiting for characters...\n";


   while(1)
   {
   
   }
}


TASK(PeriodicTask)
{
    uint8_t outputs = 0;
    aux=hr_ciaaDac;


    x1 = (hr_ciaaDac/1000);
    num1 = x1+48;

	x2=((hr_ciaaDac-x1*1000)/100);
	num2=x2+48;

	x3=((hr_ciaaDac-x1*1000-x2*100)/10);
	num3=x3+48;

	x4=(hr_ciaaDac-x1*1000-x2*100-x3*10);
	num4=x4+48;

	char dato[] = {num1,num2,num3,num4};
   
   // char message[] = "\n";
	//ciaaPOSIX_write(fd_uart1, &hr_ciaaDac, sizeof(hr_ciaaDac));
	ciaaPOSIX_write(fd_uart1, dato, ciaaPOSIX_strlen(dato));

	//myvar++;
   ciaaPOSIX_read(fd_out, &outputs, 1);
   outputs ^= 0x04;
   ciaaPOSIX_write(fd_out, &outputs, 1);
   //if (myvar > 100)
   //{
   //	myvar = 0;
   //}
   

   TerminateTask();
}


TASK(Analogic)
{
   

   /* Read ADC. */
   ciaaPOSIX_read(fd_adc, &hr_ciaaDac, sizeof(hr_ciaaDac));

   /* Signal gain. */
   hr_ciaaDac >>= 0;

   /* end of Blinking */
   TerminateTask();
}
