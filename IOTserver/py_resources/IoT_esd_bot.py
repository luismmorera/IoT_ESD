import requests
import json

BOT_TOKEN='token given by the Father Bot'
LONG_POLLING_TIMEOUT = 20
REQUEST_TIMEOUT = LONG_POLLING_TIMEOUT * 2

def send_message(chat_id, text):
    response = api_request('sendMessage', {
        "chat_id": chat_id,
        "text": text,
        "parse_mode": 'HTML'
    })

def api_request(method, parameters={}, files=None):
    url = "https://api.telegram.org/bot{token}/{method}".format(token=BOT_TOKEN, method=method)

    http_method = 'get'

    try:
        response = requests.request(http_method, url, timeout=REQUEST_TIMEOUT, params=parameters, files=files)
    except requests.RequestException as e:
        raise Exception(str(e))

    try:
        result = response.json()
    except ValueError:  # typo on the url, no json to decode
        raise Exception('Error: Invalid URL', response.status_code)

    if not (response.status_code is requests.codes.ok):
        raise Exception(result['description'], response.status_code)  # Server reported error

    if not result["ok"]:
        raise Exception("Telegram API sent a non OK response")  # Telegram API reported error

    return result["result"]
