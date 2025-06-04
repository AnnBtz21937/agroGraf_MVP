# IA simulada usando Vertex AI SDK

from google.cloud import aiplatform

def simular_ia(input_dict):
    aiplatform.init(project="SEU_PROJETO", location="us-central1")
    endpoint = aiplatform.Endpoint("projects/SEU_PROJETO/locations/us-central1/endpoints/SEU_ENDPOINT_ID")

    response = endpoint.predict([input_dict])
    return response
