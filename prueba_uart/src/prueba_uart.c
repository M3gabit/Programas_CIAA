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

static int habilitar = 0;
static int anlogOn = 0;
static int ch1 = 0;
static int ch2 = 0;

static uint8_t aux_adc = 0;
static uint8_t var_aux = 0;

static uint8_t x1 = 0;
static uint8_t x2 = 0;
static uint8_t x3 = 0;
static uint8_t x4 = 0;

static uint8_t num1 = 0;
static uint8_t num2 = 0;
static uint8_t num3 = 0;
static uint8_t num4 = 0;

static uint8_t y1 = 0;
static uint8_t y2 = 0;
static uint8_t y3 = 0;
static uint8_t y4 = 0;

static uint8_t nu1 = 0;
static uint8_t nu2 = 0;
static uint8_t nu3 = 0;
static uint8_t nu4 = 0;

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

   // open ADC ch3
   fd_adc = ciaaPOSIX_open("/dev/serial/aio/in/0", ciaaPOSIX_O_RDONLY);
   ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_SAMPLE_RATE, 100000);
   ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_1);

   //ActivateTask(Analogic);

   /* activate example tasks */
   Periodic_Task_Counter = 0;
   //Activa la tarea PeriodicTask cada 200 ticks
   //SetRelAlarm(ActivatePeriodicTask, 100, 100);

   ActivateTask(SerialEchoTask);
   //SetRelAlarm(AnalogicoUno, 100,100);
   // SetRelAlarm(AnalogicoDos, 120,100);
   // SetRelAlarm(AnalogicoTres, 140,100);

   /* end InitTask */
   TerminateTask();
}

TASK(SerialEchoTask)
{
    uint32_t outputs;  /* to store outputs status                */
    int32_t ret;      /* return value variable for posix calls  */
    char buf[20];   /* buffer for uart operation              */
    char buf2[20];
    char iniciar[] = "iniciar\0";
    char handshake[] = "123ok\r\n";
    char desconec[] = "desconectar\0";
    char analogico_on[] = "an_on\0";
    char analogico_off[] = "an_off\0";
    char analog1_on[] = "ch1_on\0";
    char analog1_off[] = "ch1_off\0";
    char analog2_on[] = "ch2_on\0";
    char analog2_off[] = "ch2_off\0";
    char analog3_on[] = "ch3_on\0";
    char analog3_off[] = "ch3_off\0";

    static int alarm1 = 0;
    static int alarm2 = 0;
    static int alarm3 = 0;
    static int var = 0;

   while(1)
   {
       // Se queda en esta instruccion hasta que recibe algo por la uart
       ret = ciaaPOSIX_read(fd_uart1, buf, 20);
       // Compara si lo que llego es igual al strin iniciar
       if (ciaaPOSIX_strcmp(buf,iniciar) == 0)
       {
           // Envia por la uart la palabra handshake y activa la alarma Handshake
           ciaaPOSIX_write(fd_uart1, handshake, ciaaPOSIX_strlen(handshake));
           SetRelAlarm(Handshake, 100, 100);
       }
       // Despues del handshake se activa esto
       if (habilitar == 1)
       {
           if(ciaaPOSIX_strcmp(buf,analog1_on) == 0)
           {
               // Si por la uart recibe analog1_on, entra a este if y se queda
               // esperando en la instruccion que sigue
               // Despues de analog1_on, le mando un tiempo en ms (por ejemplo 325)
               // que esta formado por 4 caracteres ("0" "3" "2" "5") y a eso lo
               // almaceno en buf2
               ret = ciaaPOSIX_read(fd_uart1, buf2, 20);
               // Esta cuenta es lo primero que se me ocurrio para convertir los 4
               // caracteres que estan en ascii a valor numerico en base 10
               var = ((buf2[0]-48)*1000) + ((buf2[1]-48)*100) + (buf2[2]-48)*10 + buf2[3]-48;
               // Teniendo el valor numerico, activo la alarma del ADC y le paso como parametro
               // el valor en ms que obtuve (var)
               SetRelAlarm(AnalogicoUno, var,var);
               alarm1=1;
           }

           if(ciaaPOSIX_strcmp(buf,analog1_off) == 0)
           {
               // Si recibe analog1_off, cancela la alarma y apaga el led correspondiente
               alarm1=0;
               CancelAlarm(AnalogicoUno);
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xfff7;//Apago el led 1
               ciaaPOSIX_write(fd_out, &outputs, 2);
           }

           if(ciaaPOSIX_strcmp(buf,analog2_on) == 0)
           {
               // ret = ciaaPOSIX_read(fd_uart1, buf2, 20);
               // var = ((buf2[0]-48)*1000) + ((buf2[1]-48)*100) + (buf2[2]-48)*10 + buf2[3]-48;
               SetRelAlarm(AnalogicoDos, 100, 100);
               alarm2=1;
           }

           if(ciaaPOSIX_strcmp(buf,analog2_off) == 0)
           {
               alarm2=0;
               CancelAlarm(AnalogicoDos);
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xfffb;//Apago el led 2
               ciaaPOSIX_write(fd_out, &outputs, 2);
           }

           if(ciaaPOSIX_strcmp(buf,analog3_on) == 0)
           {
               // ret = ciaaPOSIX_read(fd_uart1, buf2, 20);
               // var = ((buf2[0]-48)*1000) + ((buf2[1]-48)*100) + (buf2[2]-48)*10 + buf2[3]-48;
               SetRelAlarm(AnalogicoTres, 100, 100);
               alarm3=1;
           }

           if(ciaaPOSIX_strcmp(buf,analog3_off) == 0)
           {
               alarm3=0;
               CancelAlarm(AnalogicoTres);
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xfffd;//Apago el led 3
               ciaaPOSIX_write(fd_out, &outputs, 2);
           }

           if (ciaaPOSIX_strcmp(buf,desconec) == 0)
           {
               // Si recibe desconectar, cancela todas la alarmas y apaga todos los leds
               habilitar =0;
               if(alarm1==1)
               {
                   CancelAlarm(AnalogicoUno);
                   alarm1=0;
               }

               if(alarm2==1)
               {
                   CancelAlarm(AnalogicoDos);
                   alarm2=0;
               }

               if(alarm3==1)
               {
                   CancelAlarm(AnalogicoTres);
                   alarm3=0;
               }
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xffdf;//Apago el led verde
               ciaaPOSIX_write(fd_out, &outputs, 2);
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xfff7;//Apago el led 1
               ciaaPOSIX_write(fd_out, &outputs, 2);
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xfffb;//Apago el led 2
               ciaaPOSIX_write(fd_out, &outputs, 2);
               ciaaPOSIX_read(fd_out, &outputs, 2);
               outputs &= 0xfffd;//Apago el led 3
               ciaaPOSIX_write(fd_out, &outputs, 2);
           }
       }
   }
}

TASK(Leds) //Tarea de Handshake
{
    static int counter = 0;
    uint32_t outputs;
    ciaaPOSIX_read(fd_out, &outputs, 2);
    outputs ^= 0x0400;
    ciaaPOSIX_write(fd_out, &outputs, 2);
    if (counter ++ >= 20)
    {
        // Hace parpadear uno de los leds 20 veces, cancela la
        // alarma y pone el flag habilitar en 1
        CancelAlarm(Handshake);
        counter=0;
        habilitar= 1;
        ciaaPOSIX_read(fd_out, &outputs, 1);
        outputs |= 0x02;
        ciaaPOSIX_write(fd_out, &outputs, 1);
    }
    TerminateTask();
}

TASK(PeriodicTask)
{
    //No se activa nunca
}

TASK(AnalogicUno)
{
    uint32_t outputs = 0;
    uint16_t hr_ciaaAdc;
    // Leo el ADC
    ciaaPOSIX_read(fd_adc, &hr_ciaaAdc, sizeof(hr_ciaaAdc));
    //num1, num2, num3 y num4 contienen el valor del ADC (0 a 1023) pasado a caracteres
    // (valor ascii), por ejemplo "1" "0" "2" "3"
    x1 = (hr_ciaaAdc/1000);
    num1 = x1+48;

    x2=((hr_ciaaAdc-x1*1000)/100);
    num2=x2+48;

    x3=((hr_ciaaAdc-x1*1000-x2*100)/10);
    num3=x3+48;

    x4=(hr_ciaaAdc-x1*1000-x2*100-x3*10);
    num4=x4+48;

    char dato[] = {'A',num1,num2,num3,num4,'\r\n'};
    // Mando por la uart "A" para indicar que es del canal 1 del ADC mas el numero
    // en forma de caracteres
    ciaaPOSIX_write(fd_uart1, dato, ciaaPOSIX_strlen(dato));
    // Parpadea un led cada vez que muestrea y envia
    ciaaPOSIX_read(fd_out, &outputs, 2);
    outputs ^= 0x1000;
    ciaaPOSIX_write(fd_out, &outputs, 2);
    ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_1);
    TerminateTask();
}

TASK(AnalogicDos)
{
    uint32_t outputs;
    uint16_t hr_ciaaAdc2;
    ciaaPOSIX_read(fd_adc, &hr_ciaaAdc2, sizeof(hr_ciaaAdc2));//CH2

    y1 = (hr_ciaaAdc2/1000);
    nu1 = y1+48;

    y2=((hr_ciaaAdc2-y1*1000)/100);
    nu2=y2+48;

    y3=((hr_ciaaAdc2-y1*1000-y2*100)/10);
    nu3=y3+48;

    y4=(hr_ciaaAdc2-y1*1000-y2*100-y3*10);
    nu4=y4+48;

    char dato2[] = {'B',nu1,nu2,nu3,nu4,'\r\n'};
    ciaaPOSIX_write(fd_uart1, dato2, ciaaPOSIX_strlen(dato2));
    ciaaPOSIX_read(fd_out, &outputs, 2);
    outputs ^= 0x2000;
    ciaaPOSIX_write(fd_out, &outputs, 2);
    ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_2);
    TerminateTask();
}

TASK(AnalogicTres)
{
    uint16_t hr_ciaaAdc3;
    uint32_t outputs;
    ciaaPOSIX_read(fd_adc, &hr_ciaaAdc3, sizeof(hr_ciaaAdc3));//CH3

    x1 = (hr_ciaaAdc3/1000);
    num1 = x1+48;

    x2=((hr_ciaaAdc3-x1*1000)/100);
    num2=x2+48;

    x3=((hr_ciaaAdc3-x1*1000-x2*100)/10);
    num3=x3+48;

    x4=(hr_ciaaAdc3-x1*1000-x2*100-x3*10);
    num4=x4+48;

    char dato3[] = {'C',num1,num2,num3,num4,'\r\n'};
    ciaaPOSIX_write(fd_uart1, dato3, ciaaPOSIX_strlen(dato3));
    ciaaPOSIX_read(fd_out, &outputs, 2);
    outputs ^= 0x4000;
    ciaaPOSIX_write(fd_out, &outputs, 2);
    ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_3);
    TerminateTask();
}
