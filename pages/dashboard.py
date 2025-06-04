from dash import html, dcc
import plotly.express as px
import services.bitdog_data as bitdog

data = bitdog.get_sensor_data()

layout = html.Div([
    html.H3("Dashboard de Monitoramento Agrícola"),
    html.Div([
        dcc.Graph(figure=px.line(x=list(range(10)), y=data["agua"], title="Consumo de Água (litros)"), style={"width": "48%", "display": "inline-block"}),
        dcc.Graph(figure=px.line(x=list(range(10)), y=data["energia"], title="Consumo de Energia (kWh)"), style={"width": "48%", "display": "inline-block"}),
    ]),
    html.Div([
        dcc.Graph(figure=px.bar(x=list(range(10)), y=data["solo_umidade"], title="Umidade do Solo (%)"), style={"width": "48%", "display": "inline-block"}),
        dcc.Graph(figure=px.scatter(x=data["temperatura"], y=data["umidade"], title="Clima: Temp vs Umidade"), style={"width": "48%", "display": "inline-block"}),
    ]),
    dcc.Graph(figure=px.bar(x=list(range(10)), y=data["pragas"], title="Detecção de Pragas (eventos)"))
])
