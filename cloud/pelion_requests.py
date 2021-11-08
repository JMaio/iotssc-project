import requests
import time
from pprint import pprint

# Pelion stuff
device_id = "0"
baseurl = "https://api.us-east-1.mbedcloud.com"
access_key = "foo"
# async_id = "abc"
async_id = ""

# webhook
wh_url = "https://webhook.site/928143ad-5d11-454b-b7a6-923b8d9a639e"

# device_url = f"{baseurl}/v2/device-requests/{device_id}"
headers = {
    "authorization": f"bearer {access_key}",
}

subscription_url = f"/v2/subscriptions/{device_id}"


def subscribe(resourcePath):
    return requests.put(f"{baseurl}{subscription_url}/{resourcePath}", headers=headers)

def check_subscription(resourcePath):
    return requests.get(f"{baseurl}{subscription_url}/{resourcePath}", headers=headers)

def delete_subscription(resourcePath):
    return requests.delete(f"{baseurl}{subscription_url}/{resourcePath}", headers=headers)


def register_callback():
    # subscription_url = "/v2/subscriptions/{device-id}/{resourcePath}"
    notification_url = f"/v2/notification/callback"
    body = {
        "url": wh_url,
        # "uri": resourcePath,
        # "accept": "application/octet-stream",
        # "accept": "application/json",
        # "accept": "application/vnd.oma.lwm2m+tlv",
        # "content-type": "text/plain",
        # "payload-b64": "dmFsdWUxCg=="
    }
    return requests.put(f"{baseurl}{notification_url}", headers=headers, json=body)

def delete_callback():
    notification_url = f"/v2/notification/callback"
    return requests.delete(f"{baseurl}{notification_url}", headers=headers)

def check_callback():
    notification_url = f"/v2/notification/callback"
    return requests.get(f"{baseurl}{notification_url}", headers=headers)

def getresource(resourcePath):
    # subscription_url = "/v2/subscriptions/{device-id}/{resourcePath}"
    url = f"/v2/device-requests/{device_id}?async-id={async_id}"
    
    body = {
        "method": "GET",
        "uri": resourcePath,
        # "accept": "application/octet-stream",
        # "accept": "application/json",
        # "accept": "application/vnd.oma.lwm2m+tlv",
        # "content-type": "text/plain",
        # "payload-b64": "dmFsdWUxCg=="
    }
    return requests.post(f"{baseurl}{url}", headers=headers, json=body)

def getecho():
    url = f"/v3/devices/{device_id}/echo"
    return requests.get(f"{baseurl}{url}", headers=headers)


if __name__ == '__main__':
    # workflow for subscribing to pelion resources
    # 1. subscribe to a device resource. an "async id" will be returned for use with this app
    # 2. register a notification callback (with webhook url) to receive data
    # 
    # for dev in range(1, 4):
    #     r = subscribe(f"/3313/0/570{dev}") # accelerometer, 0, all values
    #     print(r)
    #     print(r.text)
    r = subscribe(f"/3313/0/5701") # accelerometer, 0, all values
    print(r)
    print(r.text)

    r = register_callback() # accelerometer, 0, all values
    print(r) # no json response

    # r = getresource("/3313/1/5700") # accelerometer, 1, all values
    # print(r)

    # r = delete_subscription("/3313/1/5700") # accelerometer, 0, all values
    # print(r, r.text)



    # r = check_callback()
    # print(r)
    # pprint(r.json())

    # r = delete_callback()
    # print(r)
    # pprint(r.json())
    
    # r = getecho()
    # print(r)
    # pprint(r.json())

    # r = getresource("/3313/1/5700") # accelerometer, 1, all values
    # while True:
    #     r = getresource("/3313/1/5700") # accelerometer, 1, all values
    #     # r = getresource("/10255/0/4") # accelerometer, 1, all values
    #     # r = getresource(device_id, "/3/0/0")
    #     print(r)
    #     print(r.headers)
    #     print(len(r.text))
    #     print(r.text)
    #     # print(r.json())
    #     time.sleep(5)

    