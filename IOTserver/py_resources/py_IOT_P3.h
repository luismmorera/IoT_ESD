#ifndef PY_IOT_P3_H_
#define PY_IOT_P3_H_

#include <stdio.h>
#include <Python.h>

void mandarMsgWA(char* phone, char* msg);

void mandarMsgTG(char* chat_id, char* msg);

void mandarEmail(char* email, char* asunto, char* msg);

#endif
