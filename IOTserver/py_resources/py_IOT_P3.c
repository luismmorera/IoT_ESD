#include <py_IOT_P3.h>

#define PY_MAX_MSG 1024

void mandarMsgWA(char* phone, char* msg){

	char msg_final[PY_MAX_MSG + 50];

	strcat(strcpy(msg_final, "mandar(\""), phone);
	strcat(msg_final, "\", \"");
	strcat(msg_final, msg);
	strcat(msg_final, "\")\n");

	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("import os");
	PyRun_SimpleString("os.chdir('../py_resources')");
	PyRun_SimpleString("if not(os.getcwd() in sys.path):\n\tsys.path.append(os.getcwd())");
	PyRun_SimpleString("from WhatsApp import mandar");
	PyRun_SimpleString(msg_final);
	Py_Finalize();

}

void mandarMsgTG(char* chat_id, char* msg){

	char msg_final[PY_MAX_MSG + 50];

		strcat(strcpy(msg_final, "send_message(\""), chat_id);
		strcat(msg_final, "\", \"");
		strcat(msg_final, msg);
		strcat(msg_final, "\")\n");

		Py_Initialize();
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("import os");
		PyRun_SimpleString("os.chdir('../py_resources')");
		PyRun_SimpleString("if not(os.getcwd() in sys.path):\n\tsys.path.append(os.getcwd())");
		PyRun_SimpleString("from IoT_esd_bot import send_message");
		PyRun_SimpleString(msg_final);
		Py_Finalize();
}

void mandarEmail(char* email, char* asunto, char* msg){

	char msg_final[PY_MAX_MSG + 100];

		strcat(strcpy(msg_final, "send_email(\""), email);
		strcat(msg_final, "\", \"");
		strcat(msg_final, asunto);
		strcat(msg_final, "\", \"");
		strcat(msg_final, msg);
		strcat(msg_final, "\")\n");

		Py_Initialize();
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("import os");
		PyRun_SimpleString("os.chdir('../py_resources')");
		PyRun_SimpleString("if not(os.getcwd() in sys.path):\n\tsys.path.append(os.getcwd())");
		PyRun_SimpleString("from send_email_main import send_email");
		PyRun_SimpleString(msg_final);
		Py_Finalize();
}
