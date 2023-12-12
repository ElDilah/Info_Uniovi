#ifndef MISFUNCIONES_H
#define MISFUNCIONES_H

#endif // MISFUNCIONES_H
const char* GetValorComando(const char* comando,const char* busca);
float relacion_lin(float x,float x0,float x1,float y0, float y1 );
float Leervel(int velo);
int mask(int val,int pos);
float LeerPotRef_deg(int val);
void Aplica( float ref);
float ControlTodoNada(float valor, float vel);
float ControlProporcional(float valor,float K);
void mem_lifo(float tabla[],int valores);
