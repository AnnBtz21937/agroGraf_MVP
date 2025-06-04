# main.py da função HTTP no Google Cloud Functions (Python 3.10)

import functions_framework

@functions_framework.http
def receber_bitdog(request):
    request_json = request.get_json(silent=True)
    if not request_json:
        return "Dados inválidos", 400

    # salvar em banco ou enviar para Pub/Sub
    print("Dados recebidos:", request_json)

    return {"status": "ok", "recebido": request_json}
