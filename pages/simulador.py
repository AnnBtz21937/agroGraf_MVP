# simular.py
from dash import Dash, dcc, html, Input, Output, State, ctx
from dash import callback_context as ctx
import dash_bootstrap_components as dbc

# Layout principal com abas
def get_layout():
    return html.Div([
        html.H3("Simulador Inteligente (IA)"),

        dcc.Tabs(id="simulador-tabs", value='tab-simulacao', children=[
            dcc.Tab(label='Simulação', value='tab-simulacao'),
            dcc.Tab(label='Análise de Sustentabilidade', value='tab-sustentabilidade'),
            dcc.Tab(label='Assistente IA', value='tab-ia')
        ]),

        html.Div(id='conteudo-aba-simulador')
    ])

# Conteúdos de cada aba
conteudo_simulacao = html.Div([
    html.Label("Escolha a cultura:"),
    dcc.Dropdown(id="cultura-dropdown", options=[
        {"label": "Milho", "value": "milho"},
        {"label": "Soja", "value": "soja"},
        {"label": "Cana-de-açúcar", "value": "cana"},
    ], value="milho", style={"marginBottom": "15px"}),

    html.Label("Fase da Lavoura:"),
    dcc.Dropdown(id="fase-dropdown", options=[
        {"label": "Plantio", "value": "plantio"},
        {"label": "Crescimento", "value": "crescimento"},
        {"label": "Colheita", "value": "colheita"}
    ], value="plantio", style={"marginBottom": "15px"}),

    html.Label("Período Sazonal:"),
dcc.Dropdown(id="sazonal-dropdown", options=[
    {"label": "Estação chuvosa (Jan - Mar)", "value": "chuva"},
    {"label": "Estação seca (Jul - Set)", "value": "seca"},
    {"label": "Transição (Abr - Jun / Out - Dez)", "value": "transicao"}
], placeholder="Selecione o período sazonal"),

    html.Label("Condição Climática (momentânea):"),
    dcc.RadioItems(id="clima-radio", options=[
        {"label": "Seco", "value": "seco"},
        {"label": "Chuva forte", "value": "chuva"},
        {"label": "Temperatura alta", "value": "calor"}
    ], value="seco", labelStyle={"display": "inline-block", "marginRight": "15px"}),

    dcc.Dropdown(id="solo-dropdown", options=[
    {"label": "Arenoso", "value": "arenoso"},
    {"label": "Argiloso", "value": "argiloso"},
    {"label": "Misto", "value": "misto"}
], placeholder="Tipo de solo"),

    html.Br(),
    html.Label("Aplicar práticas sustentáveis:"),
    dcc.Checklist(
        options=[
            {"label": "Irrigação Inteligente", "value": "irrigacao"},
            {"label": "Cobertura vegetal", "value": "cobertura"},
            {"label": "Rotação de culturas", "value": "rotacao"}
        ],
        value=["irrigacao"],
        id="sustentavel-checklist",
        labelStyle={"display": "block"}
    ),

    html.Br(),
    dbc.Button("Simular", id="simular-btn", color="success", className="me-2"),
    html.Div(
    dbc.Button("Gerar Relatório", id="relatorio-btn", color="info"),
    id="div-relatorio-btn",
    style={"display": "none"}
),


    html.Div([
    html.Button("Nova Simulação", id="nova-simulacao", n_clicks=0, style={"display": "none"}),
    html.Div(id="resultado-simulacao", style={"marginTop": "30px"})
])
])

conteudo_sustentabilidade = html.Div([
    html.H4("Análise de Sustentabilidade"),

    html.Div([
        html.P("Sua lavoura foi analisada com base nas práticas adotadas e nas condições climáticas simuladas."),

        html.H5("Pontuação de Sustentabilidade"),
        dcc.Graph(
            id="grafico-sustentabilidade",
            figure={
                "data": [{
                    "x": ["Ambiental", "Hídrico", "Solo", "Geral"],
                    "y": [85, 70, 90, 82],
                    "type": "bar",
                    "name": "Pontuação"
                }],
                "layout": {"height": 300, "title": "Indicadores de Sustentabilidade"}
            }
        ),

        html.H5("Impacto Ambiental Estimado"),
        html.Ul([
            html.Li("Economia estimada de água: 1.200 L/ha"),
            html.Li("Redução potencial de emissões: 5,2 kg CO₂/ha"),
            html.Li("Uso positivo de cobertura vegetal: melhora da biodiversidade do solo")
        ]),

        html.H5("Recomendações Inteligentes da IA"),
        html.Div([
            html.P("➡️ Para elevar sua sustentabilidade geral, considere adotar irrigação por gotejamento."),
            html.P("➡️ Avalie rotação de culturas com leguminosas para reduzir impacto no solo."),
            html.P("➡️ A substituição parcial de adubos químicos por orgânicos pode melhorar seu score."),
        ], style={"marginTop": "10px", "backgroundColor": "#f1f1f1", "padding": "10px", "borderRadius": "8px"}),

        html.H5("Comparação com outras simulações"),
        html.P("Você está entre os 10% de lavouras mais sustentáveis simuladas nesta plataforma hoje.",
               style={"color": "green", "fontWeight": "bold"})
    ])
])

conteudo_ia = html.Div([
    html.H4("Assistente IA Agrícola - Chat"),

    html.Div(id="chat-container", style={
        "height": "300px",
        "overflowY": "auto",
        "backgroundColor": "#f9f9f9",
        "padding": "10px",
        "border": "1px solid #ccc",
        "borderRadius": "8px",
        "marginBottom": "10px"
    }),

    dcc.Textarea(
        id="input-ia",
        placeholder="Digite sua dúvida aqui...",
        style={"width": "100%", "height": 80, "marginBottom": "10px"}
    ),

    dbc.Button("Enviar", id="btn-enviar-ia", color="success"),

    dcc.Store(id="store-mensagens", data=[])
])

def registrar_callbacks(app):
    @app.callback(
        Output('conteudo-aba-simulador', 'children'),
        Input('simulador-tabs', 'value')
    )
    def atualizar_conteudo_aba(tab):
        if tab == 'tab-simulacao':
            return conteudo_simulacao
        elif tab == 'tab-sustentabilidade':
            return conteudo_sustentabilidade
        elif tab == 'tab-ia':
            return conteudo_ia
        return html.Div("Selecione uma aba.")

    @app.callback(
        Output("cultura-dropdown", "value"),
        Output("fase-dropdown", "value"),
        Output("clima-radio", "value"),
        Output("solo-dropdown", "value"),
        Output("sazonal-dropdown", "value"),
        Output("sustentavel-checklist", "value"),
        Input("nova-simulacao", "n_clicks"),
        prevent_initial_call=True
    )
    def resetar_formulario(n_clicks):
        return "milho", "plantio", "seco", None, None, ["irrigacao"]