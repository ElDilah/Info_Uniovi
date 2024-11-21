#include <stdarg.h>
#include <stddef.h>
#include <funciones_h.h>
#include <stdio.h>
#include "UserLibSimulator.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
//__________________________________DEFINE_____________________________________
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
//______________________________________________________________________________
int sw_5[2]={0,0};
int sw_0[3]={0,0,0};
extern float *uk_pos,*uk_vel,*ek_pos,*ek_vel;
extern int estado_control;
extern int modo_control;
extern int modo_ref;
extern float _POS,_VEL;
extern int _cont_100ms;
extern int _cont_500ms;
int t_ant[2]={0,0};
int Valor_DO=0;
extern int _rele;
extern int _sleep_mode;
extern float  _Lazo_Abierto;
extern int e_ant;
float enc_val[2]={0,0};
float posk_deg[2]={0,0};
extern  float _refk_deg,_pos_ACT,_vel_act,_u_ext;
extern struct RZ _rz_pos,_rz_vel;
void MiFnControl(){
    //reserva de memoria en caso de introducir un valor de Rz distinto al inicial ;
    int ad0,ad1,ad2,ENC0;
    int tiempo500,tiempo3s;
    int DI1,DI7,DI2,DI3,DI6,DI5,DI0;
    float errk_deg=0,errk_vel=0,u_motor=0,salto_k,vel_emc = 0.0;
    //_______________________________Lectura de variables_________________________
    DI7=mask(Simulator_ReadDI(),7);
    DI6=mask(Simulator_ReadDI(),6);
    DI5=mask(Simulator_ReadDI(),5);
    DI2=mask(Simulator_ReadDI(),2);
    DI3=mask(Simulator_ReadDI(),3);
    DI1=mask(Simulator_ReadDI(),1);
    DI0=mask(Simulator_ReadDI(),0);
    sw_5[1]=sw_5[0];
    sw_5[0]=DI5;
    sw_0[1]=sw_0[0];
    sw_0[0]=DI0;
    ad0=Simulator_ReadAD(0);
    ad1=Simulator_ReadAD(1);
    ad2=Simulator_ReadAD(2);
    ENC0=Simulator_ReadCounter(0);
    _vel_act=Leervel(ad0);

    tiempo3s=valT(3000,_cont_100ms,t_ant,1);
    //________________________________Adaptacion_________________________________
    if (DI3 ==1){
    //________________________________ encoder __________________________________
        posk_deg[1]=posk_deg[0];
        posk_deg[0]=leePosEnc(ENC0);

    }else{
        posk_deg[1]=posk_deg[0];
        posk_deg[0]=LeerPotRef_deg(ad1);
    }

        _vel_act=leeVelEnc(posk_deg,100);

    //___________________________________________________________________________
    _refk_deg=LeerPotRef_deg(ad2); // lectura de la consigna

    _pos_ACT=posk_deg[0];

    if (modo_ref==MODO_REF_POTENCIOM && estado_control==ESTADO_CONTROL_POS){
        errk_deg=_refk_deg- posk_deg[0];
    }else if ((modo_ref==MODO_REF_TECLADO && estado_control==ESTADO_CONTROL_POS)||(estado_control==ESTADO_CONTROL_BARRIDO)){
        errk_deg=_POS-posk_deg[0];
    }
    if (modo_ref==MODO_REF_POTENCIOM && estado_control==ESTADO_CONTROL_VEL){
        errk_vel=relacion_lin(_refk_deg,-180,180,-40,40)-_vel_act;

    }else if (modo_ref==MODO_REF_TECLADO && estado_control==ESTADO_CONTROL_VEL){
        errk_vel=_VEL-_vel_act;
    }
    if (ek_vel!=NULL){
        ek_vel[0]=errk_vel;
        mem_lifo(ek_vel,_rz_vel.items_b);
    }
    if (ek_pos!=NULL){
        ek_pos[0]=errk_deg;
        mem_lifo(ek_pos,_rz_pos.items_b);
    }
    //_______________________________Modos y control___________________________
    if (sw_0[1]==1 && sw_0[0]==1 ){
        if (tiempo3s == 0){
            e_ant=estado_control;
            estado_control=ESTADO_CONTROL_BARRIDO;
            _POS=70;
        }
    }
    if ((tiempo3s!= 0 )&& (estado_control==ESTADO_CONTROL_BARRIDO)){
        if (sw_0[1]==1 && sw_0[0]==0 ){
            sw_0[2]+=1;
            if (sw_0[2]==2)
                sw_0[2]=0;
            if (sw_0[2]==1)
                estado_control=e_ant;
        }else if ((tiempo3s== 0 )&& (estado_control==ESTADO_CONTROL_BARRIDO))
            t_ant[1]=0;
    }
    if (DI6==0){
        if ((sw_5[0] ==1) && (sw_5[1]==0))
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
    if (DI7)
        modo_control=CONTROL_PARADO;
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
    case ESTADO_CONTROL_OPEN:
        u_motor=_Lazo_Abierto;
        break;
    case ESTADO_CONTROL_BARRIDO:
        if (errk_deg<0 && _POS==70)
            _POS=-70;
        else if (errk_deg>0 && _POS==-70)
            _POS=70;

        u_motor=ControlTodoNada(errk_deg,2);
        break;
    case ESTADO_CONTROL_VEL:
        switch (modo_control) {
        case CONTROL_PARADO:
            u_motor=0;
            break;
        case CONTROL_TN:
            u_motor=ControlTNVel(errk_vel,uk_vel[1],0.5);
            break;
        case CONTROL_P:
            u_motor=uk_vel[1]+0.05*errk_vel;
            break;
        case CONTROL_RZ:
            u_motor=CalculaRZ(_rz_vel.a,_rz_vel.b,uk_vel,ek_vel,_rz_vel.n,_rz_vel.m);
            break;
        }
        break;

    case ESTADO_CONTROL_POS:
        switch (modo_control){
        case CONTROL_PARADO:
            u_motor=0;
            break;
        case CONTROL_TN:
            u_motor=ControlTodoNada(errk_deg,2);
            break;
        case CONTROL_P:
            u_motor=ControlProporcional(errk_deg,0.05);
            break;
        case CONTROL_RZ:
            u_motor=CalculaRZ(_rz_pos.a,_rz_pos.b,uk_pos,ek_pos,_rz_pos.n,_rz_pos.m);
            break;
        }
        break;


    }
    Aplica(u_motor);
    _u_ext=u_motor;
    if (uk_pos!=NULL){
        uk_pos[0]=u_motor;
        mem_lifo(uk_pos,_rz_pos.items_a);
    }
    if (uk_vel!=NULL){
        uk_vel[0]=u_motor;
        mem_lifo(uk_vel,_rz_vel.items_a);
    }

    if (_sleep_mode ==OFF)
        Valor_DO=(Valor_DO & 0<<1);
    else if (_sleep_mode ==ON)
        Valor_DO=Valor_DO |1<<1 ;
    if (_rele ==OFF)
        Valor_DO=(Valor_DO & 0<<0);
    else if (_rele ==ON)
        Valor_DO=Valor_DO |1<<0 ;

    if (_vel_act<-1)
        Valor_DO =Valor_DO | 1<<7;
    else if (_vel_act>1)
        Valor_DO=Valor_DO |1<<6;
    else

    Simulator_LCD_gotoxy(1,1);
    Simulator_LCD_printf("");
    Simulator_LCD_gotoxy(1,2);
    Simulator_LCD_printf("");

    //limpiar pantalla
    Simulator_LCD_gotoxy(1,1); // Posiciona cursor en display
    Simulator_LCD_printf("U_m : %.3f ",u_motor);
    Simulator_LCD_gotoxy(1,2);
    Simulator_LCD_printf("VEL : %.3f ",_vel_act);
    _cont_100ms=_cont_100ms+1;
        tiempo500=valT(500,_cont_100ms,t_ant,0);
        if (tiempo500 == 0){
        if (estado_control==ESTADO_CONTROL_POS ){
            Valor_DO =Valor_DO ^ 1<<4;
        }else if (estado_control==ESTADO_CONTROL_VEL){
            Valor_DO= Valor_DO ^ 1<<3;
        }
    }
    Simulator_WriteDO(Valor_DO);
}
