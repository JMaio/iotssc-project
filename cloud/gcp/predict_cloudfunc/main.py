# deploy with 256 MiB fails, also 1 GiB
import base64
import json
import os
# import warnings
import logging

# https://riptutorial.com/tensorflow/example/31875/run-tensorflow-on-cpu-only---using-the--cuda-visible-devices--environment-variable-
# https://stackoverflow.com/a/59087453/9184658
# force "no gpus"
os.environ["CUDA_VISIBLE_DEVICES"]="-1"    

import numpy as np
from google.cloud import pubsub_v1, storage
# from tensorflow.keras.layers import Dense, Flatten, Conv2D
# ignore tf GPU warnings
# try:
#     with warnings.catch_warnings():
#         warnings.simplefilter("ignore")
# deploy failure could be timeout from loading tf

import tensorflow as tf
# from tensorflow.keras.layers import Flatten, Dense, Softmax
# from tensorflow.keras.models import Sequential
# models, layers
# except Exception as e:
#     logging.info(e)

# https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/examples/python/label_image.py
# https://www.tensorflow.org/lite/guide/inference#load_and_run_a_model_in_python
# Load the TFLite model and allocate tensors.

# def hello_pubsub(event, context):
#     """Triggered from a message on a Cloud Pub/Sub topic.
#     Args:
#          event (dict): Event payload.
#          context (google.cloud.functions.Context): Metadata for the event.
#     """
#     pubsub_message = base64.b64decode(event['data']).decode('utf-8')
#     print(pubsub_message)

# simpler tflite runtime (only runtime: https://www.tensorflow.org/lite/guide/python)
# !pip3 install --extra-index-url https://google-coral.github.io/py-repo/ tflite_runtime

project_id = "iotssc-307920"
topic_name = "model-pred"
model_name = "model-simple-21-03-21--23-14"
CLS_NAMES = ["Walking", "Running", "Resting"]

publisher = None
# The `topic_path` method creates a fully qualified identifier
# in the form `projects/{project_id}/topics/{topic_name}`
topic_path = None


# https://cloud.google.com/blog/products/ai-machine-learning/how-to-serve-deep-learning-models-using-tensorflow-2-0-with-cloud-functions
# We keep model as global variable so we don't have to reload it in case of warm invocations
model = None


def download_blob(bucket_name, source_blob_name, destination_file_name):
    """Downloads a blob from the bucket."""
    storage_client = storage.Client()
    bucket = storage_client.get_bucket(bucket_name)
    blob = bucket.blob(source_blob_name)

    blob.download_to_filename(destination_file_name)

    print('Blob {} downloaded to {}.'.format(
        source_blob_name,
        destination_file_name))


# {"data": [1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4]}

def get_pred(event, context):
    # deploy failure could be timeout from loading tf
    # import tensorflow as tf

    global model, publisher, topic_path

    # print(len(data), data)
    # data = base64.b64decode(event['data']) #.decode('utf-8')
    # input_data = np.array(json.loads(
    #     data), dtype=np.float32)[np.newaxis, :, np.newaxis]
    # TypeError: argument should be a bytes-like object or ASCII string, not 'list'
    window_size = 16 # fix later?
    try:
        # input_data = np.array(event['data'], dtype=np.float32)[np.newaxis, :, np.newaxis]
        data = base64.b64decode(event['data']) #.decode('utf-8')
        input_data = np.array(
            json.loads(data)[-window_size:], # make sure it's actually just the window size
            dtype=np.float32
        )[np.newaxis, :, np.newaxis]
        # input_data = np.array(base64.b64decode(event['data']), dtype=np.float32)[np.newaxis, :, np.newaxis]
    except Exception as e:
        logging.info(e)
        return

    if publisher is None:
        publisher = pubsub_v1.PublisherClient()
        topic_path = publisher.topic_path(project_id, topic_name)

    if model is None:
        download_blob(
            "iotssc-307920",
            f"model_simple_210321_2314/{model_name}.index",
            f'/tmp/{model_name}.index'
        )
        download_blob(
            "iotssc-307920",
            f"model_simple_210321_2314/{model_name}.data-00000-of-00001",
            f'/tmp/{model_name}.data-00000-of-00001'
        )
        model = tf.keras.models.Sequential([
            tf.keras.layers.Flatten(input_shape=(window_size, 1)),
            tf.keras.layers.Dense(128, activation='relu'),
            # tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(3),
            tf.keras.layers.Softmax(),
        ])
        model.load_weights(f'/tmp/{model_name}')

    # The function `get_tensor()` returns a copy of the tensor data.
    # Use `tensor()` in order to get a pointer to the tensor.
    output_data = model.predict(input_data)
    pred = output_data.argmax()
    out = {
        'label': CLS_NAMES[pred],
        'class_i': pred.item(),
        # https://stackoverflow.com/a/11389998/9184658
        # Use val.item() to convert most NumPy values to a native Python type:
        'confidence': output_data.max().item(),
    }
    print(out)
    # conf = output_data.max() * 100
    # print(f"{conf:.1f}% conf")
    publisher.publish(
        topic_path,
        # Data must be a bytestring
        json.dumps(out).encode("utf-8")
        # base64.b64decode(payload)
        # b'',
    )

    # return json.dumps(out) # (CLS_NAMES[output_data.argmax()], conf)
