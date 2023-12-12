#include <stdio.h>
#include "UserLibSimulator.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "MisFunciones.h"
#define ESTADO_PARADO 0
#define ESTADO_CONTROL_POS 1
#define ESTADO_CONTROL_VEL 2
#define CONTROL_TN 1
#define CONTROL_P 2
#define CONTROL_RZ 0
#define MODO_REF_POTENCIOM 1 // Referencia por potenciómetro (POS/VEL=POT)
#define MODO_REF_TECLADO 2 // Referencia por teclado (POS/VEL=valor)
#define MODO_REF_OPENLOOP 3

//________________________________Variables globales
int sw_in[2]={0,0};
float u_ant[3]={1,0,0},e_ant[3]={1,0,0};
int estado_control=ESTADO_PARADO;
int modo_control=CONTROL_TN;
int modo_ref=MODO_REF_POTENCIOM;
float _POS,_VEL;
//_________________________________________________________________________________
float ControlTNVel(float velo, float ref){
    float u_f;
    if (velo>0){
        return u_f=u_ant[1]+ref;
    }else if (velo<0){
        return u_f=u_ant[1]-ref;
    }
}

void MiFnControl(){
    int ad0,ad1,ad2;
    int Valor_DO,DI1,DI7,DI2,DI6,DI5;
    float errk_deg,refk_deg,vel,u_motor,posk_deg[2]={0,0},salto_k,errk_vel[3]={0,0,0};
    //_______________________________Lectura de variables_________________________
    DI7=mask(Simulator_ReadDI(),7);
    DI6=mask(Simulator_ReadDI(),6);
    DI5=mask(Simulator_ReadDI(),5);
    DI2=mask(Simulator_ReadDI(),2);
    DI1=mask(Simulator_ReadDI(),1);
    sw_in[1]=sw_in[0];
    sw_in[0]=DI5;
    ad0=Simulator_ReadAD(0);
    ad1=Simulator_ReadAD(1);
    ad2=Simulator_ReadAD(2);
    vel=Leervel(ad0);
    //________________________________Adaptacion_________________________________
    posk_deg[0]=LeerPotRef_deg(ad1); //conversion
    mem_lifo(posk_deg,2);
    refk_deg=LeerPotRef_deg(ad2); // lectura de la consigna
    if (modo_ref==MODO_REF_POTENCIOM && estado_control==ESTADO_CONTROL_POS){
    errk_deg=refk_deg- posk_deg[0];
    }else if (modo_ref==MODO_REF_TECLADO && estado_control==ESTADO_CONTROL_POS){
    errk_deg=_POS-posk_deg[0];
    }
    if (estado_control==ESTADO_CONTROL_VEL){
    errk_vel[0]=relacion_lin(refk_deg,-180,180,-40,40)-vel;
    mem_lifo(errk_vel,3);
    }
    e_ant[0]=errk_deg;
    mem_lifo(e_ant,3);
    //_______________________________Modos y  control___________________________
    if (DI6==1){
        if ((sw_in[0] ==1) && (sw_in[1]==0))
            if (modo_control!=CONTROL_TN)
            {
                modo_control=CONTROL_TN;
            }
            else
            {
                modo_control=CONTROL_P;
            }
    }
    else{
        modo_control=CONTROL_RZ;
    }
    if (DI1 ==1){
        salto_k=posk_deg[0]-posk_deg[1];
        if (salto_k < -180)
            posk_deg[0]+=360;
        else if (salto_k > 180)
            posk_deg[0] = posk_deg[0]- 360;
    }
    if (DI2 ==1){
        if (errk_deg <-180)
            errk_deg+=360;
        else if (errk_deg>180)
            errk_deg-=360;

    }
    switch (estado_control){
    case ESTADO_PARADO:
        u_motor=0;
        break;
    case ESTADO_CONTROL_VEL:
            switch (modo_control) {
            case CONTROL_TN:
                u_motor=ControlTNVel(errk_vel[0],0.5);
                break;
            case CONTROL_P:
                u_motor=u_ant[1]+0.05*errk_vel[0];
                break;
            case CONTROL_RZ:
                u_motor=errk_vel[0]*4.25+errk_vel[2]*-8.075+errk_vel[2]*3.866-(u_ant[0]*-1.91+u_ant[1]*1.16+u_ant[2]*-0.25);
            break;
            }


        break;

    case ESTADO_CONTROL_POS:
        switch (modo_control){
        case CONTROL_TN:
            u_motor=ControlTodoNada(errk_deg,2);
            break;
        case CONTROL_P:
            u_motor=ControlProporcional(errk_deg,0.05);
            break;
        case CONTROL_RZ:
            u_motor=(0.15*e_ant[0]+(-0.11*e_ant[1]))-(0.43*u_ant[1]);
            break;
        }
    break;

    }
    Aplica(u_motor);
    u_ant[0]=u_motor;
    mem_lifo(u_ant,3);
    if (DI7 ==1)
        Valor_DO=0;
    else
        Valor_DO=1 ;

    if (vel<-1)
        Simulator_WriteDO(Valor_DO |1<<7);
    else if (vel>1)
        Simulator_WriteDO(Valor_DO |1<<6);
    else
        Simulator_WriteDO(Valor_DO);
    Simulator_LCD_gotoxy(1,1); // Posiciona cursor en display
    Simulator_LCD_printf("U_m : %f ",u_motor);
    Simulator_LCD_gotoxy(1,2);
    Simulator_LCD_printf("VEL : %f ",vel);

}

int main()
{
    int err=Simulator_ConnectWss( "Feedback","alumno","ISAUNIOVI","127.0.0.1",8080);
    if (err!=CONNECT_IS_OK)
        return -1;
    Simulator_ConfigPWM(0,1000,500,1);
    Simulator_LCD_init();
    Simulator_SetTimerInterrupt(0,100,MiFnControl);

    float valor_ref_teclado;
    float tension_cte_teclado=2.0f;
    char cmd[80];
    char* pt_value;
    char* puntero;
    float bool;
    while (1)
    {
        printf("cmd: ");
        gets(cmd);
        puntero =cmd;
        pt_value=GetValorComando(cmd,"POS");
        if (pt_value!=NULL){
            estado_control=ESTADO_CONTROL_POS;
            bool=strcmp(pt_value,"POT");
             if (bool==0){
                    modo_ref=MODO_REF_POTENCIOM;
                }
                else{
                modo_ref=MODO_REF_TECLADO;
                _POS=atof(pt_value);
                }
                //lectura y ajuste de posicion
        }
        pt_value=GetValorComando(cmd,"VEL");
        if (pt_value!=NULL){
            estado_control=ESTADO_CONTROL_VEL;
                bool=strcmp(pt_value,"VEL");
            if (bool==0){
                modo_ref=MODO_REF_POTENCIOM;
            }else{
            _VEL=atof(pt_value);
                //lectura y ajuste de velocidad
            }
        }
        pt_value=GetValorComando(cmd,"TENSION");
        if (pt_value!=NULL)
        {
            //u_motor=lo que sea;
        }
    }
    return 0;
}
