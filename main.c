#include <stdarg.h>
#include <stddef.h>
#include <curses.h>
#include <winsock2.h>
#include <stdio.h>
#include <time.h>
#include "UserLibSimulator.h""
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "funciones_h.h"
#define ESTADO_CONTROL_POS 1
#define ESTADO_CONTROL_VEL 2
#define ESTADO_CONTROL_OPEN 3
#define ESTADO_CONTROL_BARRIDO 4
#define CONTROL_PARADO 3
#define CONTROL_TN 1
#define CONTROL_P 2
#define CONTROL_RZ 0
#define MODO_REF_POTENCIOM 1 // Referencia por potenciómetro (POS/VEL=POT)
#define MODO_REF_TECLADO 2 // Referencia por teclado (POS/VEL=valor)
#define Rz_Man 1
#define Rz_default 0
#define ON 1
#define OFF 0
//________________________________Variables globales_______________________________

float *uk_pos,*uk_vel,*ek_pos,*ek_vel,_refk_deg,_pos_ACT;
int estado_control=ESTADO_CONTROL_POS;
int modo_control=CONTROL_TN;
int modo_ref=MODO_REF_POTENCIOM;
float _POS=0,_VEL=0,_vel_act=0;
int _cont_100ms=0;
int _cont_500ms=0;
int _Modo_RZ_pos=Rz_default;
int _Modo_RZ_vel=Rz_default;
int _rele=ON;
int _sleep_mode=OFF;
float  _Lazo_Abierto=0;
int e_ant;
float _u_ext;
int s_code=0;
char server_reply[80];
struct RZ _rz_pos,_rz_vel;
//_____________________________PANTALLA Y SOCKET _______________________________________
void ControlPantalla(){
    int rsocket,recv_size;
    char *message;
    //CONTROL DINAMICO DE LA 3 VENTANA
    WINDOW* wnd3;
    WINDOW* wnd4;
    wnd3=newwin(10,80,11,5);
    wnd4=newwin(6,80,21,5);
    //__
    init_pair(3, COLOR_WHITE, COLOR_RED);
    wattron(wnd3,COLOR_PAIR(3));
    wbkgd(wnd3,COLOR_PAIR(3) | ' ');
    box(wnd3,ACS_VLINE,ACS_HLINE);
    //__
    init_pair(4, COLOR_WHITE, COLOR_GREEN);
    wattron(wnd4,COLOR_PAIR(4));
    wbkgd(wnd4,COLOR_PAIR(4) | ' ');
    box(wnd4,ACS_VLINE,ACS_HLINE);
    //__
    wmove(wnd3,1,3);
    wprintw(wnd3,"============================== INFORMACION ==============================");
    wmove(wnd3,4,3);
    if (modo_control==CONTROL_TN){
        wprintw(wnd3,"CONTROL: Todo o nada");
    }
    if (modo_control==CONTROL_P){
        wprintw(wnd3,"CONTROL: Proporcional");
    }
    if (modo_control==CONTROL_RZ){
        wprintw(wnd3,"CONTROL: Integral");
    }
    wmove(wnd3,5,3);
    if (modo_ref==MODO_REF_POTENCIOM && estado_control==ESTADO_CONTROL_POS){
        wprintw(wnd3,"Posicion consigna = %.3f , Posicion actual = %.3f ",_refk_deg,_pos_ACT);
        wmove(wnd3,6,3);
        wprintw(wnd3,"VELOCIDAD: %.3f",_vel_act);
    }else if ((modo_ref==MODO_REF_TECLADO && estado_control==ESTADO_CONTROL_POS)||(estado_control==ESTADO_CONTROL_BARRIDO)){
        wprintw(wnd3,"Posicion consigna = %.3f , Posicion actual = %.3f ",_POS,_pos_ACT);
        wmove(wnd3,6,3);
        wprintw(wnd3,"VELOCIDAD: %.3f",_vel_act);
    }
    if (modo_ref==MODO_REF_POTENCIOM && estado_control==ESTADO_CONTROL_VEL){
        wprintw(wnd3,"Velocidad consigna = %.3f , Velocidad actual = %.3f ",relacion_lin(_refk_deg,-180,180,-40,40),_vel_act);
        wmove(wnd3,6,3);
        wprintw(wnd3,"POSICION: %.3f",_pos_ACT);

    }else if (modo_ref==MODO_REF_TECLADO && estado_control==ESTADO_CONTROL_VEL){
        wprintw(wnd3,"Velocidad consigna = %.3f , Velocidad actual = %.3f ",_VEL,_vel_act);
        wmove(wnd3,6,3);
        wprintw(wnd3,"POSICION: %.3f",_pos_ACT);
}
    wmove(wnd3,8,3);
wprintw(wnd3,"Tension aplicada: %.3f",_u_ext);
    wrefresh(wnd3);
wmove(wnd4,1,3);
wprintw(wnd4,"============================ COMUNICACIONES ============================");
wmove(wnd4,2,3);
switch (s_code){
case(0):
        wprintw(wnd4,"Inicializando...");
        break;
case(1):
        wprintw(wnd4,"No se pudo inicializar comunicaciones");
        break;
case(2):
        wprintw(wnd4,"No se encotro servidor");
        break;
case(3):
        wprintw(wnd4,"Conexion Establecida");
        break;
case(4):
        wprintw(wnd4,"Fallo de enlace \n");
        wmove(wnd4,4,3);
        winsstr(wnd4,server_reply);
case(5):
        wprintw(wnd4,"Mensaje recibido \n");
        wmove(wnd4,4,3);
        winsstr(wnd4,server_reply);
}
    wrefresh(wnd4);

/* Esta parte no funciona ya que se queda colgado el programa despues de recibir el primer comando y no consegui dar con el error que lo causa
 * descomentar en caso de revisar

WSADATA WsaData;
WORD wVersionRequerida = MAKEWORD (2, 2);
SOCKET s;
WSAStartup (wVersionRequerida, &WsaData);
struct sockaddr_in server;
server.sin_addr.s_addr = inet_addr("127.0.0.1");
server.sin_family = AF_INET;
server.sin_port = htons( 55455 );
if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
{
        s_code=1;
}

if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
{
        s_code=2;

}else {
        s_code=3;
}
listen(s , 3);
if((recv_size = recv(s , server_reply ,80 , 0)) != SOCKET_ERROR)
{
        s_code=5;
        server_reply[recv_size] = '\0';
        rsocket=strcmp(server_reply,"POS");
        if (rsocket==0){
        // envio de cadena
        }
        rsocket=strcmp(server_reply,"VEL");
        if (rsocket==0){
        // envio de cadena
        }
        rsocket=strcmp(server_reply,"UMOTOR");
        if (rsocket==0){
        // envio de cadena
        }
        rsocket=strcmp(server_reply,"POT");
        if (rsocket==0){
        // envio de cadena
        }

}*/
}

int main()
{

    char cmd[80];
    char idf[40];
    char *line;

    char* pt_value;
    char *pt_start;

    int i=0;
    float bol;
    float sleeptime;
    int err=Simulator_ConnectWss( "Feedback","alumno","ISAUNIOVI","127.0.0.1",8080);
    if (err!=CONNECT_IS_OK)
        return -1;
    Simulator_ConfigPWM(0,1000,500,1);
    Simulator_LCD_init();




//_________________________________________________________________________________
    initscr(); /* Inicializa */
    cbreak(); /* Evita control-break (si se desea) */
    noecho(); /* No hace eco automático de los caracteres
 pulsados, para que no aparezcan donde esté
 el cursor (si se desea) */
    refresh(); /* Actualiza la pantalla */
    start_color(); /* Permite colores */
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);


//______________________________Inicializacion _____________________________________
    ek_vel=(float*)malloc(3*sizeof(float));
    Inicializa(ek_vel,3);
    uk_vel=(float*)malloc(3*sizeof(float));
    Inicializa(uk_vel,3);
    ek_pos=(float*)malloc(2*sizeof(float));
    Inicializa(ek_pos,2);
    uk_pos=(float*)malloc(2*sizeof(float));
    Inicializa(uk_pos,2);
    _rz_pos.b=(float*)malloc(2*sizeof(float));
    _rz_pos.b[0]=0.15;
    _rz_pos.b[1]=-0.11;
    _rz_pos.m=1;
    _rz_pos.items_b=2;
    _rz_pos.a=(float*)malloc(sizeof(float));
    _rz_pos.a[0]=-0.43;
    _rz_pos.n=1;
    _rz_pos.items_a=1;
    _rz_vel.b=(float*)malloc(3*sizeof(float));
    _rz_vel.b[0]=6.0;
    _rz_vel.b[1]=-9.3;
    _rz_vel.b[2]=3.48;
    _rz_vel.m=2;
    _rz_vel.items_b=3;
    _rz_vel.a=(float*)malloc(3*sizeof(float));
    _rz_vel.a[0]=-0.3;
    _rz_vel.a[1]=-0.78;
    _rz_vel.a[2]=0.08;
    _rz_vel.n=3;
    _rz_vel.items_a=3;
//_______________________________ consola ______________________________________________
    Simulator_SetTimerInterrupt(0,100,MiFnControl);
    Simulator_SetTimerInterrupt(0,200,ControlPantalla);
    FILE* fi2d=fopen("cmdlog.txt","w");
    fprintf(fi2d,"Inicializacion... \n");
    fclose(fi2d);
//_______________________________   VENTANAS ____________________________________________
    WINDOW* wnd1; /* Variable para manejar una ventana */
    wnd1=newwin(7,80,1,5); /* Crea nueva ventana */
    WINDOW* wnd2;
    wnd2=newwin(3,80,8,5);

//___________________________________BUCLE CONTINUO ______________________________________
    while (1)
    {FILE* fi2d=fopen("cmdlog.txt","a");
//_____________________________ DIBUJADO DE PANTALLAS ____________________________________
        wattron(wnd1,COLOR_PAIR(1));
        wbkgd(wnd1,COLOR_PAIR(1) | ' ');
        box(wnd1,ACS_VLINE,ACS_HLINE); /* Dibuja borde */
        wmove(wnd1,1,3);
        wprintw(wnd1,"=========================== COMANDO RECIENCTE ===========================");
        wrefresh(wnd1);

        wmove(wnd2,1,3); /* Colocar el cursor en fila 1, col 3*/
        wattron(wnd2,COLOR_PAIR(2));
        wprintw(wnd2,"CMD:  ");
        wbkgd(wnd2,COLOR_PAIR(2) | ' ');
        box(wnd2,ACS_VLINE,ACS_HLINE);
        wrefresh(wnd2);
        wmove(wnd2,1,9);
        echo();
        wgetnstr(wnd2,cmd,80); /* Espera la introducción de texto por teclado */
        noecho();
        wclear(wnd2);
        wclear(wnd1);
//_____________________________________________________________________________________
        //______________________________________________
        pt_value=GetValorComando(cmd,"POS");
        if (pt_value!=NULL){
            estado_control=ESTADO_CONTROL_POS;
            bol=strcmp(pt_value,"POT");
            if (bol==0){
                modo_ref=MODO_REF_POTENCIOM;
                wmove(wnd1,4,3);
                wprintw(wnd1,"POSICION POR POTENCIOMETRO");
                wrefresh(wnd1);
            }
            else{
                modo_ref=MODO_REF_TECLADO;
                _POS=atof(pt_value);
                wmove(wnd1,4,3);
                wprintw(wnd1,"POSICION POR TECLADO");
                wrefresh(wnd1);
            }
            //lectura y ajuste de posicion
        }
        pt_value=GetValorComando(cmd,"VEL");
        if (pt_value!=NULL){
            estado_control=ESTADO_CONTROL_VEL;
            bol=strcmp(pt_value,"POT");
            if (bol==0){
                modo_ref=MODO_REF_POTENCIOM;
                wmove(wnd1,4,3);
                wprintw(wnd1,"VELOCIDAD POR POTENCIOMETRO");
                wrefresh(wnd1);

            }else{
                modo_ref=MODO_REF_TECLADO;
                _VEL=atof(pt_value);
                wmove(wnd1,4,3);
                wprintw(wnd1,"VELOCIDAD  POR TECLADO");
                wrefresh(wnd1);
                    //lectura y ajuste de velocidad
            }
        }
        pt_value=GetValorComando(cmd,"TENSION");
        if (pt_value!=NULL)
        {   estado_control=ESTADO_CONTROL_OPEN;
            _Lazo_Abierto=atof(pt_value);
            wmove(wnd1,4,3);
            wprintw(wnd1,"TENSION FIJA");
            wrefresh(wnd1);
        }

        pt_value=GetValorComando(cmd,"RZ");
        if (pt_value!=NULL){
            if (estado_control==ESTADO_CONTROL_POS){
                wmove(wnd1,4,3);
                wprintw(wnd1,"CAMBIO DE RZ PARA POSICION");
                wrefresh(wnd1);
                pt_start=pt_value; //guardar el valor de  pt_value
                _rz_pos.items_b=LeerNumValCad(pt_value);
                pt_value=strstr(pt_value,"/");
                pt_value++;
                _rz_pos.items_a=LeerNumValCad(pt_value);
                free(_rz_pos.b);
                _rz_pos.b=(float*)malloc(_rz_pos.items_b*sizeof(float));
                free(ek_pos);
                ek_pos=(float*)malloc(_rz_pos.items_b*sizeof(float));
                Inicializa(ek_pos,_rz_pos.items_b);
                _rz_pos.m=_rz_pos.items_b-1;
                free(_rz_pos.a);
                _rz_pos.a=(float*)malloc(_rz_pos.items_a*sizeof(float));
                free(uk_pos);
                uk_pos=(float*)malloc(_rz_pos.items_a*sizeof(float));
                Inicializa(uk_pos,_rz_pos.items_a);
                if (_rz_pos.a!=NULL && _rz_pos.b!=NULL){
                    //se hizo la reserva de memoria en a y b
                    RellenarTabla(pt_start,_rz_pos.b,_rz_pos.items_b);
                    pt_value=strstr(pt_start,"/");
                    pt_value++;
                    RellenarTabla(pt_value,_rz_pos.a,_rz_pos.items_a);
                    //ya tenemos los valores del nuevo Rz

                }
            }
            if (estado_control==ESTADO_CONTROL_VEL){
                wmove(wnd1,4,3);
                wprintw(wnd1,"CAMBIO CONTROL RZ PARA VELOCIDAD");
                wrefresh(wnd1);
                pt_start=pt_value; //guardar el valor de  pt_value
                _rz_vel.items_b=LeerNumValCad(pt_value);
                pt_value=strstr(pt_value,"/");
                pt_value++;
                _rz_vel.items_a=LeerNumValCad(pt_value);
                free(_rz_vel.b);
                _rz_vel.b=(float*)malloc(_rz_vel.items_b*sizeof(float));
                free(ek_vel);
                ek_vel=(float*)malloc(_rz_vel.items_b*sizeof(float));
                Inicializa(ek_vel,_rz_vel.items_b);
                _rz_vel.m=_rz_vel.items_b-1;
                free(_rz_pos.a);
                _rz_pos.a=(float*)malloc(_rz_vel.items_a*sizeof(float));
                free(uk_vel);
                uk_vel=(float*)malloc(_rz_vel.items_a*sizeof(float));
                Inicializa(uk_vel,_rz_vel.items_a);
                _rz_vel.n=_rz_vel.items_a;
                if (_rz_vel.a!=NULL && _rz_vel.b!=NULL){
                    //se hizo la reserva de memoria en a y b

                    RellenarTabla(pt_start,_rz_vel.a,_rz_vel.items_a);
                    pt_value=strstr(pt_start,"/");
                    pt_value++;
                    RellenarTabla(pt_value,_rz_vel.b,_rz_vel.items_b);
                    //ya tenemos los valores del nuevo Rz

                }

            }
        }

        pt_value=GetValorComando(cmd,"SLEEP");
        if (pt_value!=NULL){
            wmove(wnd1,4,3);
            wprintw(wnd1,"MODO BAJO CONSUMO");
            wrefresh(wnd1);
            sleeptime=atof(pt_value);
            _sleep_mode=1;
            Simulator_Delay(sleeptime);
            _sleep_mode=0;
        }
        pt_value=GetValorComando(cmd,"RELAY");
        if (pt_value!=NULL){
            //comprobar estado rele
            wmove(wnd1,4,3);
            wprintw(wnd1,"CAMBIO DEL ESTADO DEL RELE");
            wrefresh(wnd1);
            bol=strcmp(pt_value,"OFF");
            if (bol==0)
                _rele=OFF;
            bol=strcmp(pt_value,"ON");
            if (bol==0)
                _rele=ON;

        }
        pt_value=GetValorComando(cmd,"FILE");
        if (pt_value!=NULL){
            wmove(wnd1,4,3);
            wprintw(wnd1,"INICIALIZACION POR FICHERO");
            wrefresh(wnd1);
            strcpy(idf,pt_value);
            FILE* fid;
            fid=fopen(idf,"r");
            if (fid!=NULL){
                while (!feof(fid)){
                    fgets(cmd,80,fid);
                    pt_value=GetValorComando(cmd,"POS");
                    if (pt_value!=NULL){
                        bol=strcmp(pt_value,"POT");
                        if (bol==0){
                            modo_ref=MODO_REF_POTENCIOM;
                        }
                        else{
                            modo_ref=MODO_REF_TECLADO;
                            _POS=atof(pt_value);
                        }
                        //comprobacion de cadena POS
                    }
                    pt_value=GetValorComando(cmd,"VEL");
                    if (pt_value!=NULL){
                        estado_control=ESTADO_CONTROL_VEL;
                        bol=strcmp(pt_value,"POT");
                        if (bol==0){
                            modo_ref=MODO_REF_POTENCIOM;

                        }else{
                            modo_ref=MODO_REF_TECLADO;
                            _VEL=atof(pt_value);
                         //lectura y ajuste de velocidad
                        }
                    }
                    pt_value=GetValorComando(cmd,"RZ");
                    if (pt_value!=NULL){
                        if (estado_control==ESTADO_CONTROL_POS){
                            pt_start=pt_value; //guardar el valor de  pt_value
                            _rz_pos.items_b=LeerNumValCad(pt_value);
                            pt_value=strstr(pt_value,"/");
                            pt_value++;
                            _rz_pos.items_a=LeerNumValCad(pt_value);
                            free(_rz_pos.b);
                            _rz_pos.b=(float*)malloc(_rz_pos.items_b*sizeof(float));
                            free(ek_pos);
                            ek_pos=(float*)malloc(_rz_pos.items_b*sizeof(float));
                            Inicializa(ek_pos,_rz_pos.items_b);
                            _rz_pos.m=_rz_pos.items_b-1;
                            free(_rz_pos.a);
                            _rz_pos.a=(float*)malloc(_rz_pos.items_a*sizeof(float));
                            free(uk_pos);
                            uk_pos=(float*)malloc(_rz_pos.items_a*sizeof(float));
                            Inicializa(uk_pos,_rz_pos.items_a);
                            if (_rz_pos.a!=NULL && _rz_pos.b!=NULL){
                                //se hizo la reserva de memoria en a y b

                                RellenarTabla(pt_start,_rz_pos.b,_rz_pos.items_b);
                                pt_value=strstr(pt_start,"/");
                                pt_value++;
                                RellenarTabla(pt_value,_rz_pos.a,_rz_pos.items_a);
                                //ya tenemos los valores del nuevo Rz

                            }
                        }
                        if (estado_control==ESTADO_CONTROL_VEL){
                            pt_start=pt_value; //guardar el valor de  pt_value
                            _rz_vel.items_b=LeerNumValCad(pt_value);
                            pt_value=strstr(pt_value,"/");
                            pt_value++;
                            _rz_vel.items_a=LeerNumValCad(pt_value);
                            free(_rz_vel.b);
                            _rz_vel.b=(float*)malloc(_rz_vel.items_b*sizeof(float));
                            free(ek_vel);
                            ek_vel=(float*)malloc(_rz_vel.items_b*sizeof(float));
                            Inicializa(ek_vel,_rz_vel.items_b);
                            _rz_vel.m=_rz_vel.items_b-1;
                            free(_rz_pos.a);
                            _rz_pos.a=(float*)malloc(_rz_vel.items_a*sizeof(float));
                            free(uk_vel);
                            uk_vel=(float*)malloc(_rz_vel.items_a*sizeof(float));
                            Inicializa(uk_vel,_rz_vel.items_a);
                            _rz_vel.n=_rz_vel.items_a;
                            if (_rz_vel.a!=NULL && _rz_vel.b!=NULL){
                                //se hizo la reserva de memoria en a y b

                                RellenarTabla(pt_start,_rz_vel.a,_rz_vel.items_a);
                                pt_value=strstr(pt_start,"/");
                                pt_value++;
                                RellenarTabla(pt_value,_rz_vel.b,_rz_vel.items_b);
                                //ya tenemos los valores del nuevo Rz

                            }

                        }
                    }
                    pt_value=GetValorComando(cmd,"SLEEP");
                    if (pt_value!=NULL){
                       sleeptime=atof(pt_value);
                        _sleep_mode=1;
                        Simulator_Delay(sleeptime);
                        _sleep_mode=0;
                    }
                    pt_value=GetValorComando(cmd,"RELAY");
                    if (pt_value!=NULL){
                        //comprobar estado rele
                        bol=strcmp(pt_value,"OFF");
                        if (bol==0)
                            _rele=OFF;
                        bol=strcmp(pt_value,"ON");
                        if (bol==0)
                            _rele=ON;
                    }

                }
                fclose(fid);
            }
        }
        wmove(wnd1,2,3); /* Colocar el cursor en fila 1, col 3*/
        winsstr(wnd1,cmd);
        time_t ahora_sec;
        struct tm* ahora;
        ahora_sec=time(NULL);
        ahora=localtime(&ahora_sec);
        line=(char*)malloc(sizeof(char)*(strlen(cmd)+strlen(" \n")));
        strcpy(line,cmd);
        strcat(line,"\n");
        //"[%02d/%02d/%d] -> [%02d:%02d]\n",ahora->tm_mday,ahora->tm_mon,ahora->tm_year+1900,ahora->tm_hour,ahora->tm_min;
        fprintf(fi2d,"[%02d/%02d/%d] -> [%02d:%02d] >>  ",ahora->tm_mday,ahora->tm_mon,ahora->tm_year+1900,ahora->tm_hour,ahora->tm_min);
        fputs(line,fi2d);
        free(line);
        fclose(fi2d);
    }
    return 0;
}

