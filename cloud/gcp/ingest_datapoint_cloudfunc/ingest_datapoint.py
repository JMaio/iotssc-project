# import os
from google.cloud import pubsub_v1
import base64

# https://stackoverflow.com/a/56603636/9184658

project_id = "iotssc-307920"
topic_name = "accel-data"

publisher = pubsub_v1.PublisherClient()
# The `topic_path` method creates a fully qualified identifier
# in the form `projects/{project_id}/topics/{topic_name}`
topic_path = publisher.topic_path(project_id, topic_name)


def run(request):
    """Responds to any HTTP request.
    Args:
        request (flask.Request): HTTP request object.
    Returns:
        The response text or any set of values that can be turned into a
        Response object using
        `make_response <http://flask.pocoo.org/docs/1.0/api/#flask.Flask.make_response>`.
    """

    try:
        # https://medium.com/@chandrapal/creating-a-cloud-function-to-publish-messages-to-pub-sub-154c2f472ca3
        r = request.get_json()
        print(r)

        payload = r['notifications'][0]['payload']
        print(payload)
        # topic_name = "projects/{project_id}/topics/{topic}".format(
        #     project_id=os.getenv('GOOGLE_CLOUD_PROJECT'),
        #     topic='accel-data',
        # )
        publisher.publish(
            topic_path, 
            # Data must be a bytestring
            base64.b64decode(payload)
            # b'', 
        )

        return "OK"
    except Exception as e:
        return f"Error: {e}"
    # if request.args and 'message' in request.args:
    #     return request.args.get('message')
    # elif request_json and 'message' in request_json:
    #     return request_json['message']
    # else:
    #     return f'Hello World!'


# {
#     "notifications": [
#         {
#             "payload": 123.4
#         }
#     ]
# }
