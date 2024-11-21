#ifndef FUNCIONES_H_H
#define FUNCIONES_H_H
//_________________________funcion de control y tablas________________________
struct RZ{
    float *a,*b;
    int n,m,items_a,items_b;
};
void MiFnControl();
//_________________________________Funciones__________________________________
const char* GetValorComando(const char* comando,const char* busca);
float relacion_lin(float x,float x0,float x1,float y0, float y1 );
float Leervel(int velo);
int mask(int val,int pos);
float LeerPotRef_deg(int val);
void Aplica( float ref);
float ControlTodoNada(float valor, float vel);
float ControlProporcional(float valor,float K);
void mem_lifo(float tabla[],int valores);
int LeerNumValCad(char *pt);
void RellenarTabla(char *pt,float val[],int i);
float ControlTNVel(float velo,float val_anterior, float ref);
float CalculaRZ(const float a[],const float b[],const float uk[],const float ek[],int n,int m);
void Inicializa(float val[],int k);
int valT(int time,int time_act,int time_ant[],int pos);
float leePosEnc(int val);
float leeVelEnc(const float val[],float time);

#endif // FUNCIONES_H_H
