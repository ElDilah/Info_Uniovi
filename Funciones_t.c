#include <string.h>
#include "UserLibSimulator.h"
#include "MisFunciones.h"
const char* Saltaespacios(char* pt){
    while (*pt==' '){
        pt++;
    }
    return pt;
}
const char* GetValorComando(const char* comando,const char* busca){
    char* pt=comando;
    int len,i;
    len= strlen(busca);
    for (i=0;i<len;i++){
        if (pt[i] != busca[i])
            return NULL;
    }
    pt+=len;
    pt=strstr(pt, "=");
    pt+=1;
    if (pt!=NULL){
        return pt=Saltaespacios(pt);
   }
}
float relacion_lin(float x,float x0,float x1,float y0, float y1 ){
    float y;
    y=((y1-y0)/(x1-x0))*(x-x0)+y0;
    return y;
}
float Leervel(int velo){
    float Utaco, velocidad;
    Utaco=relacion_lin(velo,0,1024,0,5);
    velocidad=relacion_lin(Utaco,0,5,-10,10)/0.2;

    return velocidad;
}
int mask(int val,int pos){
    int lectura;
    lectura=val&(1<<pos);
    lectura= (lectura>>pos);
    return lectura;

}
float LeerPotRef_deg(int val){
    float val_fin,val_in;
    val_in=relacion_lin(val,0,1024,0,5)*2;
    val_fin=relacion_lin(val_in,0,10,-180,180);

    return val_fin;

}
void Aplica( float ref){
    float umotor;
    umotor=relacion_lin(ref,-12,12,0,1000) ;
    Simulator_SetPWMvalue(0,umotor);
}
float ControlTodoNada(float valor, float vel){
    float tension;
    tension=vel;
    if (valor<0)
        tension=-vel;
    else if(valor>0)
        tension=vel;
    return tension;
}
float ControlProporcional(float valor,float K){
    float tension;
    tension=valor*K;

    return tension;
}
void mem_lifo(float tabla[],int valores){
    int i,d;
    for (i=valores-1;i>0;i--){
        d=i-1;
        tabla[i]=tabla[d];
    }
}
