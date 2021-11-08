# https://flask.palletsprojects.com/en/1.1.x/quickstart/#a-minimal-application
# import flask
# import json
import logging

from flask import Flask, Response, request
from flask_cors import CORS

logging.getLogger().setLevel(logging.INFO)

PROJECT = "iotssc-307920"
# TOPIC = "model-pred"
SUBSCRIPTION = "model-pred-sub"  # default
# SUBSCRIPTION = "windowed-data-sub"  # default


# Wrap the subscriber in a 'with' block to automatically call close() to
# close the underlying gRPC channel when done.
# with subscriber:
#     subscription = subscriber.create_subscription(
#         request={"name": subscription_path, "topic": topic_path}
#     )

app = Flask(__name__)
CORS(app)


def subscription_generator():
    """Pulling messages synchronously."""
    from google.api_core import retry
    from google.cloud import pubsub

    # https://github.com/googleapis/python-pubsub/blob/master/samples/snippets/subscriber.py#L481
    # publisher = pubsub.PublisherClient()
    subscriber = pubsub.SubscriberClient()
    # topic_path = subscriber.topic_path(PROJECT, TOPIC)
    subscription_path = subscriber.subscription_path(PROJECT, SUBSCRIPTION)

    with subscriber:
        response = subscriber.pull(
            request={
                "subscription": subscription_path,
                "max_messages": 300,
            },
            retry=retry.Retry(deadline=20),
        )

        for msg in response.received_messages:
            logging.info('message => %s', msg)
            yield "\n".join([
                "event: prediction",
                # https://googleapis.dev/python/pubsub/latest/subscriber/index.html#pulling-a-subscription-synchronously
                # data: "{\"label\": \"Walking\", \"class_i\": 0, \"confidence\": 0.9881201982498169}"  (bytes)
                "data: " + msg.message.data.decode("utf-8")
            ])+"\n\n"  # "the response must be closed by a double empty line"

        # ack_ids = [msg.ack_id for msg in response.received_messages]

        # msg.ack()
        # subscriber.acknowledge(
        #     request={
        #         "subscription": subscription_path,
        #         "ack_ids": ack_ids,
        #     }
        # )

# similar to pelion, subscribe to the events emitted by pubsub


@app.route('/stream')
def stream():
    return Response(subscription_generator(), mimetype="text/event-stream")


@app.route('/')
def index():
    return "Index"


if __name__ == '__main__':

    # You do not need to use gunicorn to run the example app.
    # Just make sure to use threading when running the app,
    # because otherwise the SSE connection will block your development server:
    app.debug = True
    app.run(threaded=True)
