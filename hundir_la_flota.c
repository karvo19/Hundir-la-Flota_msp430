#include <msp430.h>
#include "grlib.h"
#include "Crystalfontz128x128_ST7735.h"
#include "HAL_MSP430G2_Crystalfontz128x128_ST7735.h"
#include "stdlib.h"

/* Variables globales */
#define sonido_agua 0
#define sonido_tocado 3
#define sonido_hundido 6

#define LASB 8584
#define SIB 8097
#define DO 7662
#define RE 6802
#define MI 6079
#define FA 5730
#define SOL 5115
#define SOLS 4819
#define LA 4545
#define LAS 4494
#define SI 4292
#define DOH 3824
#define DOSH 3610
#define REH 3407
#define RESH 3215
#define MIH 3035
#define FAH 2865
#define FASH 2702
#define SOLH 2551
#define SOLSH 2410
#define LAH 2273

#define silencio 0

/*
 *
 *
    pitido(DO,1000);
    pitido(RE,1000);
    pitido(MI,1000);
    pitido(FA,1000);
    pitido(SOL,1000);
    pitido(LA,1000);
    pitido(SI,1000);
 *
 *
 */

/* Variables globales */
char tiempo;            //variable para la barra del tiempo
char t;                 //variable auxiliar del timer para controlar el tiempo
char time_out;          //variable que indicaría que se te ha terminado el tiempo del turno de darse el caso

unsigned int ejex, ejey;        //varialbes para leer del joystick
char auxx,auxy;                 //variables auxiliares para el joystick
char posx,posy;                 //variable para las coordenadas de la casilla activa

char aux_boton_1,aux_boton_2;   //variable auxiliar para controlar la pulsacion de los botones
char orientacion;               //variable auxiliar para la colocacion de los barcos, indica su rotacion. 0 = horizontal
char aux_barcos;                //variable auxiliar para saber cuantos barcos pintar
char siguiente;            //variable para pasar al siguiente estado de la maquina de estados

unsigned int barcos_J[6][10];
unsigned int barcos_IA[6][10];

char estado;                    //variable para la maquina de estados
char estado_IA;                 //variable para la maquina de estados de la IA
char victoria;                  //variale que indica si ha habido un ganador

char objetivo;                  //variable para que la IA sepa si tiene un barco localizado
char alrededor;                 //daremos un valor aleatorio para disparar alrederor del objetivo
char agua,aux_agua;                      //se usara para saber si, centrados en un objetivo, dos barcos o mas han creado una falsa direccion a seguir

unsigned int t_pitido;          //variable auxiliar del timer para controlar el tiempo del pitido
char numero_a_contar_pitido;   //variable que se usara para controlar el tiempo que dura cada nota musical
char indice_partitura;  //variable para controlar la nota por la que vamos
char siguiente_nota;    //variable que permite la inicializacion de la siguiente nota

char t_cursor_IA;                   //variable auxiliar del timer para controlar el tiempo de espera entre cada mov del cursor de la IA
char siguiente_casilla_cursor_IA;   //variable que da paso al movimiento del cursor de la IA

char posx_IA,posy_IA,posx_IA_2,posy_IA_2;    //variables para apuntar la IA a un barco en su turno
static char posx_cursor,posy_cursor;    //variables para apuntar la IA a un barco en su turno

char * record_J= (char *) 0x1000;  //punteros a memoria de los record del jugador y la IA
char * record_IA= (char *) 0x1002;
char record_aux_1;                   //variables en la que guardaremos fuera el valor anterior del record
char record_aux_2;
char aux_final;                    //la usaremos para entrar solo una vez en la parte de escribir memoria
char aux_boton_final;               //variable para el boton de borrar el marcador de la flash

Graphics_Context g_sContext;

/* Funciones */
int lee_ch(char canal){
    ADC10CTL0 &= ~ENC;                  //deshabilita el ADC
    ADC10CTL1&=(0x0fff);                //Borra canal anterior
    ADC10CTL1|=canal<<12;               //selecciona nuevo canal
    ADC10CTL0|= ENC;                    //Habilita el ADC
    ADC10CTL0|=ADC10SC;                 //Empieza la conversión
    LPM0;                               //Espera fin en modo LPM0
    return(ADC10MEM);                   //Devuelve valor leido
    }

void inicia_ADC(char canales){
    ADC10CTL0 &= ~ENC;      //deshabilita ADC
    ADC10CTL0 = ADC10ON | ADC10SHT_3 | SREF_0|ADC10IE; //enciende ADC, S/H lento, REF:VCC, con INT
    ADC10CTL1 = CONSEQ_0 | ADC10SSEL_0 | ADC10DIV_0 | SHS_0 | INCH_0;
    //Modo simple, reloj ADC, sin subdivision, Disparo soft, Canal 0
    ADC10AE0 = canales; //habilita los canales indicados
    ADC10CTL0 |= ENC; //Habilita el ADC
}

void conf_reloj(char VEL){
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
    switch(VEL){
    case 1:
        if (CALBC1_1MHZ != 0xFF) {
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
            DCOCTL = CALDCO_1MHZ;
        }
        break;
    case 8:

        if (CALBC1_8MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_8MHZ;      /* Set DCO to 8MHz */
            DCOCTL = CALDCO_8MHZ;
        }
        break;
    case 12:
        if (CALBC1_12MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_12MHZ;     /* Set DCO to 12MHz */
            DCOCTL = CALDCO_12MHZ;
        }
        break;
    case 16:
        if (CALBC1_16MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_16MHZ;     /* Set DCO to 16MHz */
            DCOCTL = CALDCO_16MHZ;
        }
        break;
    default:
        if (CALBC1_1MHZ != 0xFF) {
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
            DCOCTL = CALDCO_1MHZ;
        }
        break;

    }
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

}

char numero_aleatorio(char intervalo, char minimo){
    srand(TA0R);    //establecemos de semilla el valor actual del contador del timer 0
    return (rand()%intervalo+minimo);   //calculamos un numero aleatorio con un intervalo de amplitud intervalo y a partir de minimo
}

unsigned int BIT(char numero_bit){
    return (1<<numero_bit);
}

unsigned int leer(unsigned int barcos[6][10], char i, char pos_x, char pos_y){

    return (barcos[i][pos_y] & BIT(pos_x)); //devuelve si hay 1 o 0 en la coordenada indicada
}

char barco_hundido(unsigned int barcos[6][10], char i)
{
    char hundido;
    char j;

    hundido=1;      //diremos que esta hundido a menos que abajo se decida lo contrario
    for(j=10;j>0;j--)
    {
        if(barcos[i][j-1] != (barcos[i][j-1] & barcos[0][j-1])) //si algun modulo aun no esta tocado:
        {
            hundido=0;                                  //no esta hundido
        }
    }
    return hundido;
}

void escribir(unsigned int barcos[6][10], char i){
    char j;         //variable para el bucle for
    char longitud;  //tamaño del barco

    //definimos una variable de ayuda con la longitud del los barcos
    longitud=i;
    if(longitud<3) longitud++;

    if(orientacion==0)  //si esta en horizontal
    {
        for(j=longitud;j>0;j--)
        {
            barcos[i][posy] |= BIT(posx+j-1);   //guardamos en la capa y linea correspondiente los bits de los modulos del barco
        }
    }
    if(orientacion==1)  //si esta en vertical
    {
        for(j=longitud;j>0;j--) //si esta en vertical
        {
            barcos[i][posy+j-1] |= BIT(posx);   //guardamos en la capa y columna correspondeinte los bits de los modulos del barco
        }
    }
}

void tablero(unsigned int barcos[6][10],char solicitante){
    char i,j,x;   //variables para la iteracion de las casillas del tablero dento del bucle for

    Graphics_Rectangle casilla;
    Graphics_Rectangle marco={12,25,113,126};

    /* pintamos la cuadricula de tablero */
    Graphics_clearDisplay(&g_sContext);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    casilla.yMin=26;
    casilla.yMax=35;
    for (j=10;j>0;j--)
    {
        casilla.xMin=13;
        casilla.xMax=22;
        for (i=10;i>0;i--)
        {
            Graphics_drawRectangle(&g_sContext,&casilla);
            casilla.xMin+=10;
            casilla.xMax+=10;
        }
        casilla.yMin+=10;
        casilla.yMax+=10;
    }
    Graphics_drawRectangle(&g_sContext,&marco);


    /************************** Mostramos los barcos de la IA cuando se acabe la partida *****************************/
    if(victoria==1)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
        for(i=5;i>0;i--)    //recorremos las capas de los 5 barcos
        {
            for(j=10;j>0;j--)   //recorremos todas los vectores de la capa
            {
                for(x=10;x>0;x--)   //recorremos todos los bits de cada componente del vector
                {
                    if(leer(barcos, i, x-1, j-1))   //si hay un barco en esa casilla
                    {
                        casilla.xMin=14+(x-1)*10;
                        casilla.xMax=21+(x-1)*10;
                        casilla.yMin=27+(j-1)*10;
                        casilla.yMax=34+(j-1)*10;
                        Graphics_fillRectangle(&g_sContext,&casilla);
                    }
                }
            }
        }
    }
    /*******************************************************************************************************/


    /************************** Mostramos nuestros barcos mientras la IA juega *****************************/
    if(solicitante==1)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
        for(i=5;i>0;i--)    //recorremos las capas de los 5 barcos
        {
            for(j=10;j>0;j--)   //recorremos todas los vectores de la capa
            {
                for(x=10;x>0;x--)   //recorremos todos los bits de cada componente del vector
                {
                    if(leer(barcos, i, x-1, j-1))   //si hay un barco en esa casilla
                    {
                        casilla.xMin=14+(x-1)*10;
                        casilla.xMax=21+(x-1)*10;
                        casilla.yMin=27+(j-1)*10;
                        casilla.yMax=34+(j-1)*10;
                        Graphics_fillRectangle(&g_sContext,&casilla);
                    }
                }
            }
        }
    }
    /*******************************************************************************************************/


    /******************** HUNDIDO Y PINTAR SI ESTA HUNDIDO ***************************/
    for(i=5;i>0;i--)    //comprobamos los 5 barcos
    {
        if(barco_hundido(barcos, i))          //si el barco esta hundido lo pintamos de rojo en el tablero
        {
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
            for(j=10;j>0;j--)
            {
                for(x=10;x>0;x--)
                {
                    if(leer(barcos, i, x-1, j-1))
                    {
                        casilla.xMin=14+(x-1)*10;
                        casilla.xMax=21+(x-1)*10;
                        casilla.yMin=27+(j-1)*10;
                        casilla.yMax=34+(j-1)*10;
                        Graphics_fillRectangle(&g_sContext,&casilla);
                    }
                }
            }
        }
    }
    /***********************************************************************************/


    /******************** RECONSTRUIR DISPAROS ***************************/
    for(j=10;j>0;j--)   //recorremos todas las componentes del vector
    {
        for(x=10;x>0;x--)   //recorremos todos los bits de cada componente del vector
        {
            if(leer(barcos, 0, x-1, j-1))   //si hay un disparo en esa casilla
            {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_SKY_BLUE); //pintamos del color del agua hasta que se demuestre lo contrario
                for(i=5;i>0;i--)    //comprobamos los 5 barcos
                {
                    if(leer(barcos, i, x-1, j-1)) //si hay alguno de los barcos ahi:
                    {
                        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED); //pintamos en rojo para indicar barco tocado
                    }
                }
                Graphics_fillCircle(&g_sContext,17+(x-1)*10,30+(j-1)*10,2); //circulo de haber disparado, agua o tocado
            }
        }
    }
    /***********************************************************************************/


    /************************* RECONSTRUIR MARCADOR DE VICTORIAS ************************/
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    for(i=0;i<record_aux_1;i++)
    {
        Graphics_fillCircle(&g_sContext,6,120-10*i,3);
    }
    for(i=0;i<record_aux_2;i++)
    {
        Graphics_fillCircle(&g_sContext,120,120-10*i,3);
    }
    /***********************************************************************************/
}

void nuevo_turno(char solicitante){
    //solicitante indica el siguiente al que le toca

    //reseteamos los tiempos para el control de la barra y el tiempo restante del turno
    time_out=0;
    tiempo=0;
    t=14;

    //borramos las variables auxiliares de la pulsacion de los botones
    aux_boton_1=0;
    aux_boton_2=0;
    siguiente=0;

    //movemos el cursor al (0,0)
    posx=0;
    posy=0;
    posx_cursor=0;
    posy_cursor=0;

    //borramos las variables auxiliares del joistick
    auxx=0;
    auxy=0;

    Graphics_Rectangle barra_tiempo={12,3,113,21};

    //restauramos el tablero del jugador que le toque y pintamos la nueva barra de tiempo
    if(solicitante==0)    tablero(barcos_IA,solicitante);
    if(solicitante==1)    tablero(barcos_J,solicitante);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_SPRING_GREEN);
    Graphics_fillRectangle(&g_sContext,&barra_tiempo);
}

void flota_restante(){
    /*
     * esta funcion pinta arriba del tablero los barcos que nos quedan por colocar
     */

    Graphics_Rectangle portaviones={18,3,66,11};
    Graphics_Rectangle acorazado={69,3,107,11};
    Graphics_Rectangle crucero={23,14,51,22};
    Graphics_Rectangle submarino={54,14,82,22};
    Graphics_Rectangle destructor={85,14,103,22};

    if(aux_barcos==6)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
        Graphics_fillRectangle(&g_sContext,&portaviones);
        Graphics_fillRectangle(&g_sContext,&acorazado);
        Graphics_fillRectangle(&g_sContext,&crucero);
        Graphics_fillRectangle(&g_sContext,&submarino);
        Graphics_fillRectangle(&g_sContext,&destructor);
    }
    else if(aux_barcos==5)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_fillRectangle(&g_sContext,&portaviones);
    }
    else if(aux_barcos==4)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_fillRectangle(&g_sContext,&acorazado);
    }
    else if(aux_barcos==3)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_fillRectangle(&g_sContext,&crucero);
    }
    else if(aux_barcos==2)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_fillRectangle(&g_sContext,&submarino);
    }
    else if(aux_barcos==1)
    {
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_fillRectangle(&g_sContext,&destructor);
    }
}

void posicion_colocar_barco(char direccion){
    /*
     * direccion:
     *                  0:  derecha     (posx++)
     *                  1:  izquierda   (posx--)
     *                  2:  arriba      (posy--)
     *                  3:  abajo       (posy++)
     *
     *                  4:  pintar de nuevo en la posicion actual   (sólo útil para cuando se cambia de barco)
     *                  5:  rotar
     *
     */
    char i;
    char longitud;

    longitud=aux_barcos;
    if(longitud<3)  longitud++;

    Graphics_Rectangle modulo;

    /*
     * Nota: cuando se habla de borrar, realmente se esta leyendo si hay otro barco o no en la casilla
     * a limpiar y pintandola de blanco o azul segun sea afirmativa o negativa esta lectura
     */
    switch(direccion)
    {
    case 0: //derecha. Si horizontal: borramos 1 y pintamos 1. Si vertical: borramos todos y pintamos todos.
        if(orientacion==0)
        {
            /* modulo nuevo a "borrar" */
            modulo.xMin=14+(posx-1)*10;
            modulo.xMax=21+(posx-1)*10;
            modulo.yMin=27+posy*10;
            modulo.yMax=34+posy*10;

            if(leer(barcos_J, 0, posx-1, posy))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            else                                   Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_fillRectangle(&g_sContext,&modulo);

            /* modulo nuevo a pintar */
            modulo.xMin=14+(posx+longitud-1)*10;
            modulo.xMax=21+(posx+longitud-1)*10;
            modulo.yMin=27+posy*10;
            modulo.yMax=34+posy*10;

            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            Graphics_fillRectangle(&g_sContext,&modulo);
        }
        else
        {
            for(i=longitud;i>0;i--)
            {
                /* modulo nuevo a "borrar" */
                modulo.xMin=14+(posx-1)*10;
                modulo.xMax=21+(posx-1)*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                if(leer(barcos_J, 0, posx-1, posy+i-1))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                else                                       Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                Graphics_fillRectangle(&g_sContext,&modulo);

                /* modulo nuevo a pintar */
                modulo.xMin=14+posx*10;
                modulo.xMax=21+posx*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        break;
    case 1: //izquierda. Si horizontal: boorramos 1 y pintamos 1. Si vertical: borramos todos y pintamos todos.
        if(orientacion==0)
        {
            /* modulo nuevo a "borrar" */
            modulo.xMin=14+(posx+longitud)*10;
            modulo.xMax=21+(posx+longitud)*10;
            modulo.yMin=27+posy*10;
            modulo.yMax=34+posy*10;

            if(leer(barcos_J, 0, posx+longitud, posy))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            else                                          Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_fillRectangle(&g_sContext,&modulo);

            /* modulo nuevo a pintar */
            modulo.xMin=14+posx*10;
            modulo.xMax=21+posx*10;
            modulo.yMin=27+posy*10;
            modulo.yMax=34+posy*10;

            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            Graphics_fillRectangle(&g_sContext,&modulo);
        }
        else
        {
            for(i=longitud;i>0;i--)
            {
                /* modulo nuevo a "borrar" */
                modulo.xMin=14+(posx+1)*10;
                modulo.xMax=21+(posx+1)*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                if(leer(barcos_J, 0, posx+1, posy+i-1))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                else                                       Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                Graphics_fillRectangle(&g_sContext,&modulo);

                /* modulo nuevo a pintar */
                modulo.xMin=14+posx*10;
                modulo.xMax=21+posx*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        break;
    case 2: //arriba. Si horizontal:  borramos todos y pintamos todos. Si vertical: boorramos 1 y pintamos 1.
        if(orientacion==1)
        {
            /* modulo nuevo a "borrar" */
            modulo.xMin=14+posx*10;
            modulo.xMax=21+posx*10;
            modulo.yMin=27+(posy+longitud)*10;
            modulo.yMax=34+(posy+longitud)*10;

            if(leer(barcos_J, 0, posx, posy+longitud))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            else                                          Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_fillRectangle(&g_sContext,&modulo);

            /* modulo nuevo a pintar */
            modulo.xMin=14+posx*10;
            modulo.xMax=21+posx*10;
            modulo.yMin=27+posy*10;
            modulo.yMax=34+posy*10;

            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            Graphics_fillRectangle(&g_sContext,&modulo);
        }
        else
        {
            for(i=longitud;i>0;i--)
            {
                /* modulo nuevo a "borrar" */
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+(posy+1)*10;
                modulo.yMax=34+(posy+1)*10;

                if(leer(barcos_J, 0, posx+i-1, posy+1))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                else                                       Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                Graphics_fillRectangle(&g_sContext,&modulo);

                /* modulo nuevo a pintar */
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+posy*10;
                modulo.yMax=34+posy*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        break;
    case 3: //abajo. Si horizontal:  borramos todos y pintamos todos. Si vertical: boorramos 1 y pintamos 1.
        if(orientacion==1)
        {
            /* modulo nuevo a "borrar" */
            modulo.xMin=14+posx*10;
            modulo.xMax=21+posx*10;
            modulo.yMin=27+(posy-1)*10;
            modulo.yMax=34+(posy-1)*10;

            if(leer(barcos_J, 0, posx, posy-1))    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            else                                   Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_fillRectangle(&g_sContext,&modulo);

            /* modulo nuevo a pintar */
            modulo.xMin=14+posx*10;
            modulo.xMax=21+posx*10;
            modulo.yMin=27+(posy+longitud-1)*10;
            modulo.yMax=34+(posy+longitud-1)*10;

            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
            Graphics_fillRectangle(&g_sContext,&modulo);
        }
        else
        {
            for(i=longitud;i>0;i--)
            {
                /* modulo nuevo a "borrar" */
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+(posy-1)*10;
                modulo.yMax=34+(posy-1)*10;

                if(leer(barcos_J, 0, posx+i-1, posy-1))     Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                else                                        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                Graphics_fillRectangle(&g_sContext,&modulo);

                /* modulo nuevo a pintar */
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+posy*10;
                modulo.yMax=34+posy*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        break;
    case 4: //pintamos el barco entero en la orientacion que corresponda
        if(orientacion==0)
        {
            for(i=longitud;i>0;i--)
            {
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+posy*10;
                modulo.yMax=34+posy*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        else
        {
            for(i=longitud;i>0;i--)
            {
                modulo.xMin=14+posx*10;
                modulo.xMax=21+posx*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        break;
    case 5: //rotar barco
        if(orientacion==0)
        {
            for(i=longitud;i>0;i--)
            {
                /* modulo nuevo a "borrar" */
                modulo.xMin=14+posx*10;
                modulo.xMax=21+posx*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                if(leer(barcos_J, 0, posx, posy+i-1))   Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                else                                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                Graphics_fillRectangle(&g_sContext,&modulo);

                /* modulo nuevo a pintar */
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+posy*10;
                modulo.yMax=34+posy*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        else
        {
            for(i=longitud;i>0;i--)
            {
                /* modulo nuevo a "borrar" */
                modulo.xMin=14+(posx+i-1)*10;
                modulo.xMax=21+(posx+i-1)*10;
                modulo.yMin=27+posy*10;
                modulo.yMax=34+posy*10;

                if(leer(barcos_J, 0, posx+i-1, posy))   Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                else                                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                Graphics_fillRectangle(&g_sContext,&modulo);

                /* modulo nuevo a pintar */
                modulo.xMin=14+posx*10;
                modulo.xMax=21+posx*10;
                modulo.yMin=27+(posy+i-1)*10;
                modulo.yMax=34+(posy+i-1)*10;

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_BLUE);
                Graphics_fillRectangle(&g_sContext,&modulo);
            }
        }
        break;
    }
}

void boton_rotar_barco(){
    //si boton 2 pulsado (0)
    if(!(P1IN&BIT2) && aux_boton_2==0)  aux_boton_2=1;  //pulsamo el boton

    //si boton 2 no pulsado (1)
    if((P1IN&BIT2) && aux_boton_2==1)   //cuando lo soltemos:
    {
        aux_boton_2=0;
        if((orientacion==1 && ((posx+aux_barcos<11 && aux_barcos>2) || (posx+aux_barcos<10))) ||
                (orientacion==0 && ((posy+aux_barcos<11 && aux_barcos>2) || (posy+aux_barcos<10))))
            //si no se va a salir de la pantalla al rotar
        {
            if(aux_barcos!=0 && aux_barcos!=6)  //si es un barco en lo que estamos
                {
                    orientacion=!orientacion;   //rotamos
                    posicion_colocar_barco(5);    //pintamos el barco en la nueva orientacion
                }
        }
        else indice_partitura=sonido_agua;   //sonido para indicar que no se puede rotar aqui
    }
}

void boton_poner_barco(unsigned int barcos[6][10], char solicitante){
    /*
     * solicitante:
     *                  0: el jugador llama a la funcion
     *                  1: la IA llama a la funcion
     */
    char hay_barco;     //para saber si podemos pintar encima o no
    char j;

    //si boton 1 pulsado (0)
    if((!(P1IN&BIT1) && aux_boton_1==0) && solicitante==0)  aux_boton_1=1; //para que solo entre una vez al pulsar el boton

    //si boton 1 no pulsado (1)
    if((((P1IN&BIT1) && aux_boton_1==1) && solicitante==0) || solicitante==1)
    {
        aux_boton_1=0;        //volvemos a poner la variable auxiliar a 0 para poder volver a pulsar luego
        hay_barco=0;          //hay hueco hasta que se demuestre lo contrario

        if(aux_barcos<5)      //para el primer barco siempre hay hueco
        {
            escribir(barcos, aux_barcos);   //rellenamos la capa actual para poder comparar
            for(j=10;j>0;j--)   //recorremos toda la "matriz de bits" para las "capas" 0 y actual
            {
                if(barcos[aux_barcos][j-1] & barcos[0][j-1])    //si solapan
                {
                    hay_barco=1; //si el and da en cualquier comparacion, un 1 es que hay barco.
                }
                barcos[aux_barcos][j-1]=0; //borramos la linea que acabamos de comparar porque no sabemos si sera la posicion definitiva
            }
        }
        if(!(hay_barco))      //si no hay barco => hay hueco
        {
            if(aux_barcos!=6){
                escribir(barcos, aux_barcos);   //escribimos el barco en su capa
                for(j=10;j>0;j--)
                {
                    barcos[0][j-1] |= barcos[aux_barcos][j-1];  //lo añadimos a la capa 0 para comprobar si el siguiente solapa
                }
            }
            aux_barcos--;   //decrementamos la variable para pasar al siguiente barco
            if(solicitante==0)
            {
                indice_partitura=sonido_tocado; //sonido de colocacion correcta
                if(aux_barcos!=0)   posicion_colocar_barco(4);      //cada vez que cambiemos de barco, pintamos
            }
        }
        else if(solicitante==0) indice_partitura=sonido_agua; //sonido de colocacion incorrecta
        if(aux_barcos==0)   //si ya hemos puesto todos los barcos
        {
            aux_barcos=6;   //reseteamos el indice de las capas
            siguiente=1;    //le toca el siguiente
        }
    }
}

void boton_disparar(unsigned int barcos[6][10], char solicitante){
    /*
     * solicitante:
     *                  0: el jugador llama a la funcion
     *                  1: la IA llama a la funcion
     */
    char i,j,x;
    Graphics_Rectangle modulo;

    //si boton 1 pulsado (0)
    if((!(P1IN&BIT1) && aux_boton_1==0) && solicitante==0)  aux_boton_1=1; //para que solo entre una vez al pulsar el boton

    //si boton 1 no pulsado (1)
    if((((P1IN&BIT1) && aux_boton_1==1) && solicitante==0) || solicitante==1)
    {
        aux_boton_1=0;        //volvemos a poner la variable auxiliar a 0 para poder volver a pulsar luego


        /******************** DISPAROS Y PINTAR EL DISPARO ************************/
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_SKY_BLUE); //pintamos del color del agua hasta que se demuestre lo contrario
        siguiente=1;           //siguiente hasta que se demuestre lo contrario
        indice_partitura=sonido_agua;   //sonara agua a menos que luego se decida lo contrario
        escribir(barcos, 0);    //registramos el disparo, lo escribimos en la capa de los disparos

        for(i=5;i>0;i--)    //comprobamos los 5 barcos
        {
            if(leer(barcos, i, posx, posy)) //si hay alguno de los barcos ahi:
            {
                siguiente=0;                //mantenemos el turno
                indice_partitura=sonido_tocado;     //suena tocado a menos que lo hayamos hundido
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED); //pintamos en rojo para indicar barco tocado

                if(barco_hundido(barcos, i))
                {
                    indice_partitura=sonido_hundido;    //sonara el sonido que indica que hemos hundido un barco
                }
            }
        }
        Graphics_fillCircle(&g_sContext,17+posx*10,30+posy*10,2); //circulo de haber disparado, agua o tocado
        /***********************************************************************************/


        /******************** HUNDIDO Y PINTAR SI ESTA HUNDIDO ***************************/
        for(i=5;i>0;i--)    //comprobamos los 5 barcos
        {
             //comprobamos si el barco actual esta hundido
            if(barco_hundido(barcos, i))          //si el barco esta hundido lo pintamos de rojo en el tablero
            {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
                for(j=10;j>0;j--)
                {
                    for(x=10;x>0;x--)
                    {
                        if(leer(barcos, i, x-1, j-1))
                        {
                            modulo.xMin=14+(x-1)*10;
                            modulo.xMax=21+(x-1)*10;
                            modulo.yMin=27+(j-1)*10;
                            modulo.yMax=34+(j-1)*10;
                            Graphics_fillRectangle(&g_sContext,&modulo);
                        }
                    }
                }
            }
        }
        /***********************************************************************************/


        /*************************** VICTORIAS ****************************/
        //si (la suma de todos los barcos) es igual a [(la suma) por (los disparos)] se acaba la partida
        victoria=1;             //victoria a menos que abajo se diga lo contrario
        for(j=10;j>0;j--)
        {
            if((barcos[1][j-1] | barcos[2][j-1] | barcos[3][j-1] | barcos[4][j-1] | barcos[5][j-1]) !=
                    ((barcos[1][j-1] | barcos[2][j-1] | barcos[3][j-1] | barcos[4][j-1] | barcos[5][j-1]) & barcos[0][j-1]))
            {
                victoria=0; //si falta alguno por hundir no se gana
            }
        }
        /***********************************************************************************/
    }
}

void boton_pasar_turno(){
    /*
     * si ya le toca al siguiente (hemos disparado al agua), y no queremos
     * esperar a que se agote el tiempo, podemos pasarlo con el boton 2
     *
     * si queremos agotar parte del tiempo decidiendo nuestro proximo movimiento, estamos
     * en nuestro derecho de no pulsar el boton
     *
     * tambien se usara para cuando la ia haya disparado y errado, pasar a nuestro turno
     *  sin esperar al tiempo
     */
    //si boton 2 pulsado (0)
    if(!(P1IN&BIT2) && aux_boton_2==0)  aux_boton_2=1;

    //si boton 2 no pulsado (1)
    if((P1IN&BIT2) && aux_boton_2==1)
    {
        aux_boton_2=0;
        if(siguiente==1 && time_out==0)     time_out=1;
    }
}

void barra(){
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_drawLine(&g_sContext,11+tiempo,3,11+tiempo,21);
}

void control_cursor_colocar(){
    Graphics_Rectangle cursor;
    ejex=lee_ch(0);
    ejey=lee_ch(3);

    cursor.xMin=13+posx*10;  //pintamos el valor de posicion anterior de negro
    cursor.xMax=22+posx*10;
    cursor.yMin=26+posy*10;
    cursor.yMax=35+posy*10;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawRectangle(&g_sContext,&cursor);

    //posiciones del cursor, mover joystick, vemos los limites de la cuadrícula
    if(aux_barcos!=6)
    {
        //leemos los valores del joistick y dejamos mover el barco si no se sale de los limites de la pantalla
        //solo movemos ua vez por cada movimiento de joistick
        if(ejex>800 && auxx==0)
        {
            auxx=1;
            if(orientacion==0 && aux_barcos<6)
            {
                if(aux_barcos>2)
                {
                    if(posx<9+1-aux_barcos)
                    {
                        posx++;
                        posicion_colocar_barco(0);
                    }
                }
                else if(posx<9-aux_barcos)
                {
                    posx++;
                    posicion_colocar_barco(0);
                }
            }
            else if(posx<9)
            {
                posx++;
                posicion_colocar_barco(0);
            }
        }
        if(ejex<200 && auxx==0)
        {
            auxx=1;
            if(posx>0)
            {
                posx--;
                posicion_colocar_barco(1);
            }
        }
        if(ejey>800 && auxy==0)
        {
            auxy=1;
            if(posy>0)
            {
                posy--;
                posicion_colocar_barco(2);
            }
        }
        if(ejey<200 && auxy==0)
        {
            auxy=1;
            if(orientacion==1 && aux_barcos<6)
            {
                if(aux_barcos>2)
                {
                    if(posy<9+1-aux_barcos)
                    {
                        posy++;
                        posicion_colocar_barco(3);
                    }
                }
                else if(posy<9-aux_barcos)
                {
                    posy++;
                    posicion_colocar_barco(3);
                }
            }
            else if(posy<9)
            {
                posy++;
                posicion_colocar_barco(3);
            }
        }
    }

    //si dejamos de mover el joystick
    if(ejex<560 && ejex>480 && ejey<560 && ejey>480)
    {
        auxy=0;
        auxx=0;
    }

    /* Pintamos el cursor */
    cursor.xMin=13+posx*10;
    cursor.xMax=22+posx*10;
    cursor.yMin=26+posy*10;
    cursor.yMax=35+posy*10;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_CYAN);
    Graphics_drawRectangle(&g_sContext,&cursor);
}

void control_cursor_apuntar(){
    Graphics_Rectangle cursor;
    ejex=lee_ch(0);
    ejey=lee_ch(3);

    cursor.xMin=13+posx*10;  //pintamos el valor de posicion anterior de negro
    cursor.xMax=22+posx*10;
    cursor.yMin=26+posy*10;
    cursor.yMax=35+posy*10;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawRectangle(&g_sContext,&cursor);

    //posiciones del cursor, mover joystick, vemos los limites de la cuadrícula
    if(ejex>800 && auxx==0)
    {
        auxx=1;
        if(posx<9)posx++;
    }
    if(ejex<200 && auxx==0)
    {
        auxx=1;
        if(posx>0) posx--;
    }
    if(ejey>800 && auxy==0)
    {
        auxy=1;
        if(posy>0) posy--;
    }
    if(ejey<200 && auxy==0)
    {
        auxy=1;
        if(posy<9)posy++;
    }

    //si dejamos de mover el joystick
    if(ejex<560 && ejex>480 && ejey<560 && ejey>480)
    {
        auxy=0;
        auxx=0;
    }

    cursor.xMin=13+posx*10;      //pintamos el cursor en si
    cursor.xMax=22+posx*10;
    cursor.yMin=26+posy*10;
    cursor.yMax=35+posy*10;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_CYAN);
    Graphics_drawRectangle(&g_sContext,&cursor);
}

void colocacion_J(){
    control_cursor_colocar();       //mueve el barco por la pantalla mientras lo colocamos en funcion del joystick
    flota_restante();               //pinta arriba los barcos que te quedan por colocar
    boton_rotar_barco();            //rota el barco de ser posible cuando pulsas el boton 2
    boton_poner_barco(barcos_J, 0); //coloca el barco de ser posible cuando pulsas boton 1

    /*
     * La ultima funcion pondra a 1 la variable fin_colocacion cuando
     * el jugador haya terminado de poner los barcos, y se usara para cambiar
     * de estado en la maquina de estados.
     */
}

void colocacion_IA(){
    char longitud;
    longitud=aux_barcos;
    if(aux_barcos<3)      longitud=aux_barcos+1;

    /*
     * 1. generar aleatoriamente posx, posy y orientacion
     *
     * 2. comprobar si vale para el barco actual
     *
     * 3.1. en caso afirmativo guardarlo, pasar al siguiente barco
     * 3.2. en caso negativo, volver al paso 1
     *
     * 4. Repetir hasta acabar con todos los barcos
     */

    /* PASO 1 */
    orientacion=numero_aleatorio(2,0);  //generamos numero aleatorio de 0 a 1
    if(orientacion==0)
    {
        posx=numero_aleatorio(11-longitud,0);        //generamos numero aleatorio de 0 a (11 - longitud del barco actual)
        posy=numero_aleatorio(10,0);        //generamos numero aleatorio de 0 a 9
    }
    if(orientacion==1)
    {
        posx=numero_aleatorio(10,0);        //generamos numero aleatorio de 0 a 9
        posy=numero_aleatorio(11-longitud,0);        //generamos numero aleatorio de 0 a (11 - longitud del barco actual)
    }

    /* PASO 2   PASO 3  Y   PASO 4 */
    flota_restante();               //pinta arriba los barcos que te quedan por colocar
    boton_poner_barco(barcos_IA, 1);
    /*
     * La ultima funcion pondra a 1 la variable fin_colocacion cuando
     * la IA haya terminado de poner los barcos, y se usara para cambiar
     * de estado en la maquina de estados
     */
}

void borrar_barco(unsigned int barcos[6][10], char i){
    char j;
    for(j=10;j>0;j--)   barcos[i][j-1]=0;
}

void control_cursor_IA(char posx_referencia, char posy_referencia){
    Graphics_Rectangle cursor;

    cursor.xMin=13+posx_cursor*10;  //pintamos el valor de posicion anterior de negro
    cursor.xMax=22+posx_cursor*10;
    cursor.yMin=26+posy_cursor*10;
    cursor.yMax=35+posy_cursor*10;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawRectangle(&g_sContext,&cursor);

    //esta funcion movera una cuadricula cada 40 ms hasta llegar a la posicion a la que la IA ha decidido disparar
    if(siguiente_casilla_cursor_IA==1)
    {
        if(posx_cursor<posx_referencia)
        {
            t_cursor_IA=17;      //para que espere 40 ms para volverse a mover
            siguiente_casilla_cursor_IA=0;
            posx_cursor++;
        }
        if(posy_cursor<posy_referencia)
        {
            t_cursor_IA=17;      //para que espere 40 ms para volverse a mover
            siguiente_casilla_cursor_IA=0;
            posy_cursor++;
        }
        if(posx_cursor>posx_referencia)
        {
            t_cursor_IA=17;      //para que espere 40 ms para volverse a mover
            siguiente_casilla_cursor_IA=0;
            posx_cursor--;
        }
        if(posy_cursor>posy_referencia)
        {
            t_cursor_IA=17;      //para que espere 40 ms para volverse a mover
            siguiente_casilla_cursor_IA=0;
            posy_cursor--;
        }
    }
    cursor.xMin=13+posx_cursor*10;      //pintamos el cursor en si
    cursor.xMax=22+posx_cursor*10;
    cursor.yMin=26+posy_cursor*10;
    cursor.yMax=35+posy_cursor*10;

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_CYAN);
    Graphics_drawRectangle(&g_sContext,&cursor);
}

void turno_J(){
    barra();  //actualizamos la barra
    boton_pasar_turno();    //para pasar de turno si ya hemos disparado y le toca a la IA
    control_cursor_apuntar();   //para mover el cursor
    if(siguiente==0)    boton_disparar(barcos_IA, 0);
}

void turno_IA(){
    /************* PSEUDICODIGO ****************/
    /*
     * 1. generar aleatoria pero inteligentente posx, posy
     *      disparando solo a casillas que tengan ambas coordenadas o par o impar
     *      se consigue reducir a la mitad la cantidad de casillas a inspeccionar
     *      para encontrar todos los barcos, ya que el mas pequeño es de 2.
     *
     * 2. comprobar si ya se ha disparado ahi
     *
     * 3.1. en caso negativo, disparar
     * 3.2. en caso afirmativo, volver al paso 1
     *
     * 4. si acertamos en un barco, seguimos disparando pero ahora apuntamos a ese barco
     *          tener en cuenta que la primera vez que acertemos un barco, para apuntar otra
     *          vez a él, es aleatoriamente arriba, abajo, izq o derecha, siempre y cuando no
     *          haya ya un disparo en alguno de ellos
     *
     *          cuando ya tengamos mas de un modulo del barco, hay que apuntar aleatoriamente pero en
     *          la direccion de ese barco, por ejemplo, si esta vertical, pues arriba o debajo del
     *          modulo de arriba o de el de abajo respectivamente. siempre y cuando no se haya disparado
     *          ahi ya.
     *
     *          si en la direccion del barco llegamos a dos aguas, una por cada extremo, son dos barcos en realidad.
     *          Disparo a cada uno de ellos por separado en la otra direccion hasta hundirlos
     *
     *          hay que almacenar de alguna forma que se falla un disparo, al siguiente turno siga por el
     *          barco este que habia encontrado
     *
     * 5. cuando hundamos el barco, se activara una variable barco_hundido a partir de la cual volveremos al paso 1
     * pero comprobando antes si me he dejado alguno tocado atras y seguir por el
     */
    char i,j,x;

    barra();  //actualizamos la barra del tiempo
    boton_pasar_turno();    //para pasar de turno si ya ha disparado y nos toca

    switch(estado_IA)
    {
    case 0: /* comprobamos si hay algun barco tocado */
        for(i=5;i>0;i--)    //para cada barco
        {
            if(!barco_hundido(barcos_J, i))  //si no esta hundido, ver si esta tocado
            {
                for(j=10;j>0;j--)   //para cada fila (j-1) (en realidad es un entero no una fila)
                {
                    if((barcos_J[i][j-1] & barcos_J[0][j-1])) //si esto no da cero, es que hay algun barco tocao aqui
                    {
                        estado_IA=5;    //pasamos a hundir ese barco que esta tocado
                        for(x=10;x>0;x--)   //recorremos todos los bits de la fila que nos indica que hay un barco tocado
                        {
                            if(((barcos_J[i][j-1] & barcos_J[0][j-1]) & BIT(x-1)))  //buscamos la coordenada del modulo tocado
                            {
                                objetivo++;     //marcamos que hay un objetivo
                                if(objetivo!=1) //si tenemos mas de un modulo tocado
                                {
                                    if((x-1)==posx_IA)  orientacion=1;  //si coindice la x, parece que esta vertical
                                    if((j-1)==posy_IA)  orientacion=0;  //si coindice la y, parece que esta Horizontal
                                }
                                posx_IA=x-1;    //guardamos la posicion del barco tocado para centrarnos en el
                                posy_IA=j-1;
                            }
                        }
                    }
                }
            }
        }
        if(estado_IA!=5)    estado_IA=1;
        break;
    case 1: /* Generamos coordenada aleatoria para disparar, con x e y ambos par o impar */
        orientacion=numero_aleatorio(2,0);           //ya no se usa para nada, asi que la usaremos para decidir si par o impar
        posx_IA=(numero_aleatorio(5,0)<<1)+orientacion; //(numero aleatorio de 0 a 4)*2 y dependiendo de orientacion sumamos 1 o no
        posy_IA=(numero_aleatorio(5,0)<<1)+orientacion; //(numero aleatorio de 0 a 4)*2 y dependiendo de orientacion sumamos 1 o no

        estado_IA=2;    //pasamos al siguiente estado para ver si esta casilla es una buena candidata
        break;
    case 2: /* comprobamos si se habia disparado ya ahi o no */
        if(objetivo==0) //no tenemos objetivo, venimos del estado 1
        {
            if(!(leer(barcos_J, 0, posx_IA, posy_IA)))    //si no habiamos disparado ahí aun
            {
                estado_IA=3;                        //pasamos al siguiente estado para disparar
                t_cursor_IA=75;         //esperamos segundo y medio antes de mover el cursor
                siguiente_casilla_cursor_IA=0;
            }
            else    estado_IA=1;    //si ya habiamos disparado, necesitamos otra coordenada, volvemos al estado 1
        }
        if(objetivo!=0) //tenemos objetivo, venimos del estado 5
        {
            if(!(leer(barcos_J, 0, posx_IA_2, posy_IA_2)))    //si no habiamos disparado ahí aun
            {
                estado_IA=6; //pasamos al siguiente estado para disparar
                t_cursor_IA=37;         //esperamos 3/4 de segundo antes de mover el cursor
                siguiente_casilla_cursor_IA=0;
            }
            else if(objetivo>1) //si no podemos disparar ahi y venimos del estado 5, vemos si hay un barco tocado o un agua previa
            {
                agua++; //decimos que es agua previa a menos que ahora digamos lo contrario
                for(i=5;i>0;i--)    //para cada barco (si es un barco y no agua, solo puede haber 1)
                {
                    if(leer(barcos_J, i, posx_IA_2, posy_IA_2))   //si no podemos disparar porque ya se ha disparado ahi, y hay un barco
                    {
                        if(!barco_hundido(barcos_J, i))//si la casilla tocada a la que hemos llegado corresponde a un barco hundido, lo tratamos como agua
                        {
                            agua--; //deshacemos lo que acabamos de sumar
                            //guardamos las coordenadas para seguir disparando en esa direccion hasta topar con agua
                            posx_IA=posx_IA_2;
                            posy_IA=posy_IA_2;
                        }
                    }
                }
                estado_IA=5;    //si ya habiamos disparado, necesitamos otra coordenada, volvemos al estado 5
            }
            else    estado_IA=5;    //(objetivo==1) si ya habiamos disparado, necesitamos otra coordenada, volvemos al estado 5
        }
        break;
    case 3: /* movemos el cursor hacia la casilla seleccionada para disparar (no tenemos objetivo fijo)*/
        if(siguiente==0)    //solo nos movemos si tenemos el turno, si no, esperamos
        {
            control_cursor_IA(posx_IA, posy_IA); //solo se mueve si se posee el turno para el siguiente turno
            //guardamos las coordenadas por si acertamos
            posx_IA_2=posx_cursor;
            posy_IA_2=posy_cursor;
        }
        if((posx_IA_2==posx_IA) && (posy_IA_2==posy_IA))
        {
            estado_IA=4;        //cuando se llegue a la casilla seleccionada, cambiamos de estado y disparamos
        }
        break;
    case 6: /* movemos el cursor hacia la casilla seleccionada para disparar (tenemos objetivo fijo)*/
        if(siguiente==0)    //solo nos movemos si tenemos el turno, si no, esperamos
        {
            control_cursor_IA(posx_IA_2, posy_IA_2); //solo se mueve si se posee el turno para el siguiente turno
        }
        if((posx_IA_2==posx_cursor) && (posy_IA_2==posy_cursor))
        {
            estado_IA=4;        //cuando se llegue a la casilla seleccionada, cambiamos de estado y disparamos
        }
        break;
    case 4: /* DISPARAMOS */
        /*
         * Disparamos, y si acertamos, siguiente se pone a 0 porque mantenemos el turno.
         *
         * En funcion del nuevo siguiente, segun el valor que se le de en boton_disparar, o si estabamos
         * ya centrados en un barco concreto, pasamos al estado 0 o al 5
         * en otras palabras:
         *          si acertamos:
         *                  si lo hemos hundido, ponemos objetivo a 0, pasamos al estado 0, toca ver si hay alguno tocado o buscar uno nuevo
         *                  si aun no esta hundido, ponemos objetivo a 1, pasamos el estado 5, toca seguir disparandole hasta hundirlo
         *
         *                          para saber en que barco estamos, y asi tener una capa de la matriz con la que comprar los disparos
         *                          y ver si el barco esta hundido, primero leemos la casilla a la que acabamos de disparar para
         *                          todos los barcos, y el que de 1, con esa capa comparamos.
         *          si fallamos:
         *                  si objetivo==0, pasamos al 0, toca ver si hay alguno tocado o buscar uno nuevo
         *                  si objetivo!=1, pasamos al 5
         */
        if(siguiente==0)    //solo disparamos si tenemos el turno, si no, esperamos
        {
            //dispara la variable posx asecas, luego le pasamos el valor decidido
            posx=posx_IA_2;
            posy=posy_IA_2;
            boton_disparar(barcos_J,1); //solo disparamos si al siguiente que nos toca es a nosotros: if(siguiente==0)
            if(siguiente==0)    //despues de diparar, en funcion del nuevo siguiente, pasamos al estado 0 o al 5
            {
                //guardamos las coordenadas porque si venimos del estado 5 no tienen por que ser iguales
                posx_IA=posx_IA_2;
                posy_IA=posy_IA_2;

                objetivo++;     //lo marcamos como un objetivo nuevo, y si ahora resulta que lo hemos hundido, lo desmarcamos
                for(i=5;i>0;i--)    //para cada barco
                {
                    if((barcos_J[i][posy_IA] & BIT(posx_IA))) //buscamos la capa del barco que hemos tocado
                    {
                        if(barco_hundido(barcos_J, i))
                        {
                            objetivo=0;    //si acabamos de hundir a nuestro ojetivo, pasamos al estado 0 para buscar otro
                            //reseteamos las aguas
                            agua=0;
                            aux_agua=0;
                        }
                    }
                }
            }
            else if(objetivo>1)     agua++; //si teniamos una orientacion, hemos alcanzado un extremo

            if(objetivo!=0) estado_IA=5;    //si tenemos un objetivo tocado, pasamos al estado 5 para tratar de hundirlo
            else            estado_IA=0;    //si no tenemos objetivo, pasamos al estado 0 para buscar uno
        }
        break;
    case 5: /* OBJETIVO MARCADO */
        /*
         * Se ha detectado un barco, toca apuntar al barco encontrado y tratar de hundirlo.
         *
         * Miramos si el barco mas pequeño que quede por detectar cabe en vertical y horizontal, porque si no
         * cabe, hay que disparar solo con la orientacion para la que si cabe (solo es relevante si el de dos
         * no se encuentra el ultimo, ademas es dificil que queden barcos con tableros tan llenos de disparos como
         * para que en una de las dos orientaciones no quepa ningun barco restante)
         * (Esto no se va a implementar de momento)
         *
         * Si es la primera vez que lo tocamos (objetivo==1), apuntamos aleatoriamente arriba, abajo, izq o derecha (a menos que se de
         * el caso explicado arriba)
         *
         * Si no es la primera (objetivo!=1), guardar esa orientacion hasta hundirlo o encontrar aguas en los extremos:
         * serian dos barcos, y sus orientaciones la contraria a la actual (agua==2)(orientacion^=1)
         * apuntamos aleatoriamente a uno de sus extremos en consecuencia a su orientacion
         *
         * comprobamos que no se haya disparado ya a la casilla seleccionada
         *          si ya se habia disparado, necesitamos otra coordenada nueva, repetimos el estado 5 (es decir, no hacemos nada
         *          y dejamos que vuelva a entrar)
         *          si no se habia disparado aun, pasamos al estado 3 para comenzar el proceso de disparar
         */
        if(objetivo==1) //no tenemos ninguna pista de hacia donde va el barco
        {
            alrededor=numero_aleatorio(4,0);    //variable con valor aleatorio para disparar alrederor del objetivo
            orientacion=numero_aleatorio(2,0);  //variable para cuando objetivo>1, para comenzar aleatoriamente
            switch(alrededor)
            {
            case 0: //arriba
                if(posy_IA>0)
                {
                    posx_IA_2=posx_IA;
                    posy_IA_2=posy_IA-1;
                    estado_IA=2;    //pasamos a comprobar si ahi hemos disparado ya
                }
                break;
            case 1: //abajo
                if(posy_IA<9)
                {
                    posx_IA_2=posx_IA;
                    posy_IA_2=posy_IA+1;
                    estado_IA=2;    //pasamos a comprobar si ahi hemos disparado ya
                }
                break;
            case 2: //izquierda
                if(posx_IA>0)
                {
                    posx_IA_2=posx_IA-1;
                    posy_IA_2=posy_IA;
                    estado_IA=2;    //pasamos a comprobar si ahi hemos disparado ya
                }
                break;
            case 3: //derecha
                if(posx_IA<9)
                {
                    posx_IA_2=posx_IA+1;
                    posy_IA_2=posy_IA;
                    estado_IA=2;    //pasamos a comprobar si ahi hemos disparado ya
                }
                break;
            }
        }
        else    //(objetivo>1) tenemos mas de un modulo tocado, puede ser una pista de la orientacion del barco
        {
            switch(agua)
            {
            case 1: //nos hemos topado con agua a uno de los extremos del "barco"

                if(aux_agua==0)
                    {
                        orientacion^=1; //disparamos en esa orientacion, en sentido contrario
                        aux_agua=1;
                    }
                break;
            case 2: //resulta que eran dos barcos distintos, cambiamos de orientacion.
                orientacion=numero_aleatorio(2,0);  //volvemos a decidir aleatoriamente el sentido
                agua=0;     //reseteamos las aguas
                aux_agua=0;
                if(alrededor<2) alrededor=2;
                else            alrededor=0;
                break;
            }

            switch(alrededor)
            {
            case 0: case 1: //antes hemos disparado arriba o abajo, el barco parce estar en vertical
                posx_IA_2=posx_IA;
                if(orientacion)
                {
                    if(posy_IA>0)
                    {
                        posy_IA_2=posy_IA-1;  //disparamos arriba
                        estado_IA=2;    //pasamos a comprobar la casilla seleccionada
                    }
                    //si por un extremo ya hemos tocado agua, y por el otro llegamos a la pared, cambiamos de orientacion
                    else if(agua==1)
                    {
                        alrededor=2;
                        agua=0;
                        aux_agua=0;
                    }
                    //si llegamos a la pared antes que al agua, disparamos en la otra direccion
                    else    orientacion=0;  //si no se puede disparar arriba, disparamos debajo
                }
                else
                {
                    if(posy_IA<9)
                    {
                        posy_IA_2=posy_IA+1;  //disparamos abajo
                        estado_IA=2;    //pasamos a comprobar la casilla seleccionada
                    }
                    //si por un extremo ya hemos tocado agua, y por el otro llegamos a la pared, cambiamos de orientacion
                    else if(agua==1)
                    {
                        alrededor=2;
                        agua=0;
                        aux_agua=0;
                    }
                    //si llegamos a la pared antes que al agua, disparamos en la otra direccion
                    else    orientacion=1;  //si no se puede disparar debajo, disparamos arriba
                }
                break;
            case 2: case 3: //antes hemos disparado izquierda o derecha, el barco parce estar en horizontal
                posy_IA_2=posy_IA;
                if(orientacion)
                {
                    if(posx_IA>0)
                    {
                        posx_IA_2=posx_IA-1;  //disparamos izquierda
                        estado_IA=2;    //pasamos a comprobar la casilla seleccionada
                    }
                    //si por un extremo ya hemos tocado agua, y por el otro llegamos a la pared, cambiamos de orientacion
                    else if(agua==1)
                    {
                        alrededor=0;
                        agua=0;
                        aux_agua=0;
                    }
                    //si llegamos a la pared antes que al agua, disparamos en la otra direccion
                    else    orientacion=0;  //si no se puede disparar izquierda, disparamos derecha
                }
                else
                {
                    if(posx_IA<9)
                    {
                        posx_IA_2=posx_IA+1;  //disparamos derecha
                        estado_IA=2;    //pasamos a comprobar la casilla seleccionada
                    }
                    //si por un extremo ya hemos tocado agua, y por el otro llegamos a la pared, cambiamos de orientacion
                    else if(agua==1)
                    {
                        alrededor=0;
                        agua=0;
                        aux_agua=0;
                    }
                    //si llegamos a la pared antes que al agua, disparamos en la otra direccion
                    else    orientacion=1;  //si no se puede disparar derecha, disparamos izquierda
                }
                break;
            }
        }
        break;
    }
}

void pitido(unsigned int nota,unsigned int duracion)//nota musical, con palabaras (define), duracion en ms
{

    //Configuramos el pin del timer que esta a PWM para los pitidos
    if(nota!=silencio)
    {
        P2SEL |= BIT6; //ponemos el pin 2.6 como pwm
        P2SEL2 = 0;   //tenemos que asegurarnos que el resto esta a 0
        P2DIR |= BIT6; // (p2.6 salida) aqui se encuentra el buzzer
    }

    TA0CCR0=nota;       //la frecuencia define la nota
    TA0CCR1=nota>>1;    //el duty cycle debe estar a la mitad para generar una onda cuadrada

    siguiente_nota=0;    //no dejamos que se pase a la siguiente nota

    t_pitido=0;     //ponemos a cero el tiempo del pitido para controlar la duracion exacta
    numero_a_contar_pitido=duracion/20;    //dividimos la duracion en periodos de timer
}

void titanic()
{
    if(siguiente_nota==1)
    {
        switch(indice_partitura)
        {
        case 0: /*case 16:*/ pitido(FA,750); break;
        case 1: case 3: case 5: case 9: /*case 17: case 19: case 21: case 25:*/ pitido(silencio,20);break;
        case 2: /*case 18:*/ pitido(FA,250); break;
        //case 3: pitido(silencio,20);break;
        case 4: case 6: case 10: /*case 20: case 22: case 26:*/  pitido(FA,500); break;
        //case 5: pitido(silencio,20);break;
        //case 6: pitido(FA,500); break;
        case 7: case 11: /*case 23:*/ pitido(MI,500); break;
        case 8: case 12: /*case 24:*/ pitido(FA,1000); break;
        //case 9: pitido(silencio,20);break;
        //case 10: pitido(FA,500); break;
        //case 11: pitido(MI,500); break;
        //case 12: pitido(FA,1000); break;
        case 13: pitido(SOL,500); break;
        case 14: pitido(LA,1000); break;
        case 15: pitido(SOL,1000); break;
        //case 16: pitido(FA,750); break;
        //case 17: pitido(silencio,20);break;
        //case 18: pitido(FA,250); break;
        //case 19: pitido(silencio,20);break;
        //case 20: pitido(FA,500); break;
        //case 21: pitido(silencio,20);break;
        //case 22: pitido(FA,500); break;
        //case 23: pitido(MI,500); break;
        //case 24: pitido(FA,1000); break;
        //case 25: pitido(silencio,20);break;
        //case 26: pitido(FA,500); break;
        /////case 27: pitido(DO,1000); break;
        //case 28: pitido(silencio,5000); break;
        }

    }
}

void megalovania()
{
    if(siguiente_nota==1)
    {
        switch(indice_partitura)
        {
        case 0: case 2: case 9: /*case 21: case 33: case 45:*/ pitido(RE,120); break;
        case 1: /*case 13: case 25: case 37:*/ pitido(silencio,20);break;
        //case 2: pitido(RE,120); break;
        case 3: /*case 15: case 27: case 39:*/ pitido(REH,220); break;
        case 4: /*case 16: case 28: case 40:*/ pitido(LA,320); break;
        case 5: /*case 17: case 29: case 41:*/ pitido(SOLS,120); break;
        case 6: /*case 18: case 30: case 42:*/ pitido(silencio,120); break;
        case 7: /*case 19: case 31: case 43:*/ pitido(SOL,220); break;
        case 8: /*case 20: case 32: case 44:*/ pitido(FA,220); break;
        //case 9: pitido(RE,120); break;
        case 10: /*case 22: case 34: case 46:*/ pitido(FA,120); break;
        case 11: /*case 23: case 35: case 47:*/ pitido(SOL,120); break;

        /***************** primer compas *********************/
        //case 12: case 14: pitido(DO,120); break;
        //case 13: pitido(silencio,20);break;
        //case 14: pitido(DO,120); break;
        //case 15: pitido(REH,220); break;
        //case 16: pitido(LA,320); break;
        //case 17: pitido(SOLS,120); break;
        //case 18: pitido(silencio,120); break;
        //case 19: pitido(SOL,220); break;
        //case 20: pitido(FA,220); break;
        //case 21: pitido(RE,120); break;
        //case 22: pitido(FA,120); break;
        //case 23: pitido(SOL,120); break;

        /***************** segundo compas ***********************/
        //case 24: case 26: pitido(SIB,120); break;
        //case 25: pitido(silencio,20);break;
        //case 26: pitido(SIB,120); break;
        //case 27: pitido(REH,220); break;
        //case 28: pitido(LA,320); break;
        //case 29: pitido(SOLS,120); break;
        //case 30: pitido(silencio,120); break;
        //case 31: pitido(SOL,220); break;
        //case 32: pitido(FA,220); break;
        //case 33: pitido(RE,120); break;
        //case 34: pitido(FA,120); break;
        //case 35: pitido(SOL,120); break;

        /**************  tercer compas ********************/
        //case 36: case 38: pitido(LASB,120); break;
        //case 37: pitido(silencio,20);break;
        //case 38: pitido(LASB,120); break;
        //case 39: pitido(REH,220); break;
        //case 40: pitido(LA,320); break;
        //case 41: pitido(SOLS,120); break;
        //case 42: pitido(silencio,120); break;
        //case 43: pitido(SOL,220); break;
        //case 44: pitido(FA,220); break;
        //case 45: pitido(RE,120); break;
        //case 46: pitido(FA,120); break;
        //case 47: pitido(SOL,120); break;

        /**************** cuarto compas *********************/
        //case 48: pitido(silencio,750); break;
        //case 49: indice_partitura=0; break; //repetimos
        }

    }
}

void sonidos()
{
    if(siguiente_nota==1)
    {
        switch(indice_partitura)
        {
        /* Sonido agua */
        case 0: pitido(SOL,100);break;
        case 1: pitido(RE,300);break;

        /* Sonido tocado */
        case 3: pitido(SOL,100);break;
        case 4: pitido(REH,300);break;

        /* Sonido hundido */
        case 6: pitido(SOL,200);break;
        case 7: pitido(silencio,50);break;
        case 8: pitido(SOL,100);break;
        case 9: pitido(REH,300);break;
        }

    }
}

void escribir_record()
{
    conf_reloj(1);

    FCTL1 = FWKEY + ERASE;          //vamos a borrar la celda de memoria
    FCTL3 = FWKEY;                  //metemos la llave para que nos deje, quitamos LOCK y LOCKA
    *record_J = 0;                  //borramos la celda
    FCTL1 = FWKEY + WRT;            //activamos el modo write, borramos el modo erase
    *record_J = record_aux_1;       //escribimos el record J
    *record_IA = record_aux_2;      //escribimos el record IA
    FCTL1 = FWKEY;                  //borramos el modo write
    FCTL3 = FWKEY + LOCK + LOCKA;   //bloqueamos la ram, restauramos LOCK y LOCKA

    conf_reloj(16);
}

int main(void) {
    //pulsadores del boosterpack
    P1REN|=BIT1+BIT2;   //habilita la resistencia pull-up/pull-down
    P1OUT|=BIT1+BIT2;   //resistencia de pull-up

    //boton del joystick
    P2REN|=BIT5; //habilitamos resistencia
    P2OUT|=BIT5; //resistencia de pull up

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    conf_reloj(16);             //configuramos el reloj a 16 MHz

    /* Configuracion de las interrupciones por timer */
    TA1CCTL0=CCIE;          //CCIE=1
    TA1CTL=TASSEL_2| ID_3 | MC_1;  //SMCLK, DIV=8, UP, 2MH
    TA1CCR0=39999;            //periodo 20ms

    /* Configuramos el PWM que modulara las notas musicales */
    TA0CTL = TASSEL_2|ID_3| MC_1; //SMCLK, DIV:8, UP    2MHz
    TA0CCTL1 = OUTMOD_7;  //modo PWN act a nivel alto
    TA0CCR0 = 25999;
    TA0CCR1 = 3999;

    inicia_ADC(BIT0+BIT3);

    __bis_SR_register(GIE);         //activamos las interrupciones

    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_clearDisplay(&g_sContext);

    /* Inicializaciones previas a la maquina de estados */
    //estado=0;
    //estado_IA=0;
    aux_barcos=6;
    //victoria=0;
    //siguiente=0;
    //objetivo=0;
    //agua=0;
    //aux_agua=0;
    siguiente_nota=1;
    indice_partitura=100;


    //leemos de flash las victorias
    record_aux_1 = *record_J;
    record_aux_2 = *record_IA;

    tablero(barcos_J,0);
    while(1){
        LPM0;
        switch(estado)
        {
        case 0:     //el jugador coloca sus barcos
            colocacion_J();
            sonidos();
            if(siguiente==1)
            {
                borrar_barco(barcos_J, 0);
                aux_barcos=6;
                siguiente=0;
                tablero(barcos_IA, 0);
                estado=1;
            }
            break;
        case 1:     //la IA coloca sus barcos
            colocacion_IA();
            if(siguiente==1)
            {
                borrar_barco(barcos_IA, 0);
                siguiente=0;
                aux_barcos=6;
                nuevo_turno(0);
                estado=2;
            }
            break;
        case 2:     //turno jugador
            turno_J();
            sonidos();
            if(victoria==1)     //hemos ganado
            {
                /*
                 * faltaria añadir algunas acciones como incrementar el contador de victorias
                 *
                 * pintar el fondo verdecito como para hacer ver que hemos ganado
                 *
                 * si se implementa, hacer sonido de victoria
                 */
                Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_LIGHT_GREEN);
                tablero(barcos_IA,0);
                indice_partitura=0;
                estado=4;
                record_aux_1++;

            }
            else if(time_out==1)
            {
                nuevo_turno(1);
                estado=3;
            }
            break;
        case 3:     //turno IA
            turno_IA();
            sonidos();
            if(victoria==1)     //ha ganado la ia
            {
                /*
                 * faltaria añadir algunas acciones como incrementar el contador de victorias,
                 * mostrar por pantalla los barcos que nos faltaban por encontar
                 */
                Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_INDIAN_RED);
                tablero(barcos_IA,0);
                indice_partitura=0;
                estado=5;
                record_aux_2++;
            }
            else if(time_out==1)
            {
                nuevo_turno(0);
                estado=2;
            }
            break;
        case 4:     //hemos ganado
            if(aux_final==0)    //registramos la victoria
            {
                aux_final=1;
                escribir_record();
                tablero(barcos_IA,0);
            }
            if((!(P2IN&BIT5)) && aux_boton_final==0) //si el boton esta pulsado
            {
                aux_boton_final=1;
            }
            if((P2IN&BIT5) && aux_boton_final==1) //cuando soltemos el boton
            {
                aux_boton_final=0;
                record_aux_1=0;
                record_aux_2=0;
                escribir_record(); //reseteamos los record en flash
                tablero(barcos_IA,0);

            }
            //titanic();
            megalovania();
            break;
        case 5:     //ha ganado la IA
            if(aux_final==0)    //registramos la victoria
            {
                aux_final=1;
                escribir_record();
                tablero(barcos_IA,0);
            }
            if((!(P2IN&BIT5)) && aux_boton_final==0) //si el boton esta pulsado
            {
                aux_boton_final=1;
            }
            if((P2IN&BIT5) && aux_boton_final==1) //cuando soltemos el boton
            {
                aux_boton_final=0;
                record_aux_1=0;
                record_aux_2=0;
                escribir_record(); //reseteamos los record en flash
                tablero(barcos_IA,0);
           }
            titanic();
            break;
        }
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ConvertidorAD(void)
{
    LPM0_EXIT;  //Despierta al micro al final de la conversión
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR_HOOK(void)
{
    t_pitido++;
    if(t_cursor_IA>0)   t_cursor_IA--;
    if(time_out==0)         //si aun no se te ha acabado el tiempo del turno
        {
            if(t==0)            //contamos hacia atras porque es más óptimo
            {
                t=14;            //7 es mas o menos lo que hay que contar para que la barra se agote en ~30s
                tiempo++;       //aumentamos la coordenada de la barra
                if(tiempo==103) //si la barra se ha agotado
                {
                    tiempo=0;
                    time_out=1; //indicamos que se te ha acabado el tiempo
                }
            }
            t--;                //decrementamos t
        }
    if(t_pitido==numero_a_contar_pitido)
    {
        /* Apagamos el buzzer */
        P2SEL &= ~BIT6; //desactivamos el pin 6 como pwm
        P2SEL2 = 0;   //tenemos que asegurarnos que el resto esta a 0
        P2DIR &= ~BIT6; // (p2.6 salida) aqui se encuentra el buzzer

        indice_partitura++;
        siguiente_nota=1;

        /* volvemos a dejar el contador a esa frecuencia hasta la siguiente nota para usarlo mientras en numeros_aleatorios */
        TA0CCR0 = 25999;
        TA0CCR1 = 3999;
    }
    if(t_cursor_IA==0)
    {
        siguiente_casilla_cursor_IA=1;
    }
    LPM0_EXIT;              //salimos del modo interrupcion
}
