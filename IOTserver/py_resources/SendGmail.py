from __future__ import print_function
import base64
import os.path
from email.mime.text import MIMEText
from email.mime.audio import MIMEAudio
from email.mime.image import MIMEImage
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from googleapiclient.errors import HttpError
import mimetypes

class sendGmail:
    def __init__(self, service):
        self.service = service

    def create_message(self, to, subject, message_text):
      """Create a message for an email.
    
      Args:
        to: Email address of the receiver.
        subject: The subject of the email message.
        message_text: The text of the email message.
    
      Returns:
        An object containing a base64url encoded email object.
      """
      message = MIMEText(message_text)
      message['To'] = to
      message['From'] = self.service.users().getProfile(userId = 'me').execute()['emailAddress']
      message['Subject'] = subject
      return {'raw': base64.urlsafe_b64encode(message.as_bytes()).decode("utf-8")}

    def create_message_with_attachment(self, to, subject, message_text, file):
      """Create a message for an email.
    
      Args:
        to: Email address of the receiver.
        subject: The subject of the email message.
        message_text: The text of the email message.
        file: The path to the file to be attached.
    
      Returns:
        An object containing a base64url encoded email object.
      """
      message = MIMEMultipart()
      message['To'] = to
      message['From'] = self.service.users().getProfile(userId = 'me').execute()['emailAddress']
      message['Subject'] = subject
    
      msg = MIMEText(message_text)
      message.attach(msg)
    
      content_type, encoding = mimetypes.guess_type(file)
    
      if content_type is None or encoding is not None:
        content_type = 'application/octet-stream'
      main_type, sub_type = content_type.split('/', 1)
      if main_type == 'text':
        fp = open(file, 'rb')
        msg = MIMEText(fp.read(), _subtype=sub_type)
        fp.close()
      elif main_type == 'image':
        fp = open(file, 'rb')
        msg = MIMEImage(fp.read(), _subtype=sub_type)
        fp.close()
      elif main_type == 'audio':
        fp = open(file, 'rb')
        msg = MIMEAudio(fp.read(), _subtype=sub_type)
        fp.close()
      else:
        fp = open(file, 'rb')
        msg = MIMEBase(main_type, sub_type)
        msg.set_payload(fp.read())
        fp.close()
      filename = os.path.basename(file)
      msg.add_header('Content-Disposition', 'attachment', filename=filename)
      message.attach(msg)
    
      return {'raw': base64.urlsafe_b64encode(message.as_bytes()).decode()}

    def send_message(self, user_id, message):
      """Send an email message.
    
      Args:
        service: Authorized Gmail API service instance.
        user_id: User's email address. The special value "me"
        can be used to indicate the authenticated user.
        message: Message to be sent.
    
      Returns:
        Sent Message.
      """
      try:
        message = (self.service.users().messages().send(userId=user_id, body=message)
                   .execute())
        print('Message Id: %s' % message['id'])
        return message
      except HttpError as error:
        print('An error occurred: %s' % error)


