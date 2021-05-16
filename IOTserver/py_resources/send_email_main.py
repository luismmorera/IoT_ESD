from __future__ import print_function
from googleapiclient.discovery import build
import auth
import SendGmail

SCOPES = ['https://mail.google.com/']
CLIENT_SECRET_FILE = 'credentials.json'

def send_email_photo(email, asunto, text, photo):
	"""
	Send an email from a Gmail account
	"""
	#Creation of an auth instance
	authInst = auth.auth(SCOPES, CLIENT_SECRET_FILE)
	service = build('gmail', 'v1', credentials=authInst.get_credentials())

	#Creation of a sendGmail instance
	sendInst = SendGmail.sendGmail(service)
	message = sendInst.create_message_with_attachment(email, asunto, text, photo)
	sendInst.send_message('me', message)


def send_email(email, asunto, text):
	"""
	Send an email from a Gmail account
    	"""
	#Creation of an auth instance
	authInst = auth.auth(SCOPES, CLIENT_SECRET_FILE)
	service = build('gmail', 'v1', credentials=authInst.get_credentials())

	#Creation of a sendGmail instance
	sendInst = SendGmail.sendGmail(service)
	message = sendInst.create_message(email, asunto, text)
	sendInst.send_message('me', message)
