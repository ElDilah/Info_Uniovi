#include <stdarg.h>
#include <stddef.h>
#include <funciones_h.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "UserLibSimulator.h"

float leeVelEnc(const float val[],float time){
    float ref,ref2,res,v1,v2;
    if (val[0]<0)
        v1=val[0]+360;
    else
        v1=val[0];
    if (val[1]<0)
        v2=val[1]+360;
    else
        v2=val[1];
    ref=((v1-v2)/time);
    return res=ref*(1000.f*60.f/360.f);
}
float leePosEnc(int val){
    float res,gra;
    int ref,ref2; // para truncar la posicion
    ref=val/2048;
    ref2=2048*(1+ref);
    gra=relacion_lin(val,2048*ref,ref2,0,360);
    if (gra>180)
        res=gra-360;
    else if (gra<-180)
        res=gra+360;
    else
        res=gra;
    return res;
}

int valT(int time,int time_act,int time_ant[],int pos){
    int resultado=0;
    if (time_ant[pos] ==0)
        time_ant[pos]=time_act;

    resultado=(time-100*(time_act-time_ant[pos]));
    if (resultado == 0){
        time_ant[pos]=0;
    }
    return resultado;
}
const char* Saltaespaciosycomas(char* pt){
    while (*pt==' ' || *pt==','){
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
        return pt=Saltaespaciosycomas(pt);
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
int LeerNumValCad(char *pt){
    int i=0;
    char* pt_end;
    pt=strstr(pt,"[");
    if (pt==NULL){
        return -1;
    }
    pt++;
    strtod(pt,&pt_end);
    while (pt!=pt_end){
        i++;
        pt=pt_end;
        pt=Saltaespaciosycomas(pt);
        strtod(pt,&pt_end);
    }
    pt=strstr(pt,"]");
    if (pt==NULL){
        return -1;
    }
    pt++;
    return i;
}
void RellenarTabla(char *pt,float val[],int i){
    int j=0;
    char *pt_end;
    pt=strstr(pt,"[");
    pt++;
    strtod(pt,&pt_end);
    for (j=0;j<i;j++){
        val[j]=atof(pt);
        pt=pt_end;
        pt=Saltaespaciosycomas(pt);
        strtod(pt,&pt_end);
    }
    pt=strstr(pt,"]");
    pt++;
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
float CalculaRZ(const float a[],const float b[],const float uk[],const float ek[],int n,int m){
    float result1=0,result2=0,resultfin=0;
    int i;
    for (i=0;i<m;i++){
        result1=result1+(b[i]*ek[m-i]);
    }
    for (i=0;i<n;i++){
        result2=result2+(a[i]*uk[n-i]);
    }
    return resultfin=result1-result2;
}
float ControlTNVel(float velo,float val_anterior, float ref){
    float u_f;
    if (velo>0){
        return u_f=val_anterior+ref;
    }else if (velo<0){
        return u_f=val_anterior-ref;
    }
}
void Inicializa(float val[],int k){
    int i ;
    for (i=0;i<k;i++)
        val[i]=0;
}
