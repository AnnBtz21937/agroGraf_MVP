import random

def get_sensor_data():
    return {
        "agua": [random.randint(200, 800) for _ in range(10)],
        "solo_umidade": [random.uniform(20, 80) for _ in range(10)],
        "energia": [random.uniform(1.2, 4.5) for _ in range(10)],
        "pragas": [random.randint(0, 3) for _ in range(10)],
        "temperatura": [random.uniform(18, 35) for _ in range(10)],
        "umidade": [random.uniform(30, 80) for _ in range(10)],
    }
