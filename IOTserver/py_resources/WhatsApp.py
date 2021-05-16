import pywhatkit as py
import datetime as date

def mandar(phone, msg):

	if date.datetime.now().minute == 59 :
		py.sendwhatmsg("+34" + phone, msg, date.datetime.now().hour+1, 0, 20, True)
	else :
		py.sendwhatmsg("+34" + phone, msg, date.datetime.now().hour, date.datetime.now().minute+1, 20, True)

