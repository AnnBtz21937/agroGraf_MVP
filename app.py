from dash import Dash, dcc, html, Input, Output, State, ctx
import dash_bootstrap_components as dbc
from pages import dashboard, simulador, relatorios, configuracoes, alarmes, identificacao
from pages.simulador import get_layout, registrar_callbacks
from components.sidebar import render_sidebar
from components.navbar import render_navbar

app = Dash(__name__, suppress_callback_exceptions=True, external_stylesheets=[dbc.themes.BOOTSTRAP])

app.title = "AgroGraf MVP"
server = app.server

app.layout = html.Div([  
    dcc.Location(id="url"),

    html.Div(render_navbar(), style={
        "position": "fixed",
        "top": 0,
        "left": 0,
        "right": 0,
        "zIndex": 999,
        "height": "60px"
    }),

    html.Div(render_sidebar(), style={
        "position": "fixed",
        "top": "60px",
        "left": 0,
        "bottom": 0,
        "width": "15rem",
        "padding": "1rem",
        "backgroundColor": "#003366",
        "overflowY": "auto"
    }),

    html.Div(id="page-content", style={
        "margin-left": "15rem",
        "margin-top": "60px",
        "padding": "2rem",
        "height": "calc(100vh - 60px)",
        "overflow-y": "auto"
    })
])

registrar_callbacks(app)



@app.callback(Output("page-content", "children"), [Input("url", "pathname")])
def display_page(pathname):
    if pathname == "/simulador":
        return get_layout()
    elif pathname == "/relatorios":
        return relatorios.layout
    elif pathname == "/configuracoes":
        return configuracoes.layout
    elif pathname == "/alarmes":
        return alarmes.layout
    elif pathname == "/identificacao":
        return identificacao.layout
    return dashboard.layout



@app.callback(Output("modal-acess", "is_open"),
              [Input("btn-acess", "n_clicks")],
              [State("modal-acess", "is_open")])
def toggle_modal_a(n1, is_open):
    return not is_open if n1 else is_open


@app.callback(
    Output("modal-notificacoes", "is_open"),
    [Input("btn-notificacoes", "n_clicks")],
    [State("modal-notificacoes", "is_open")]
)
def toggle_modal_notificacoes(n1, is_open):
    return not is_open if n1 else is_open

@app.callback(Output("modal-user", "is_open"),
              [Input("btn-user", "n_clicks")],
              [State("modal-user", "is_open")])
def toggle_modal_b(n1, is_open):
    return not is_open if n1 else is_open

@app.callback(
    Output('output-image-upload', 'children'),
    Input('upload-image', 'contents'),
    State('upload-image', 'filename'),
    State('upload-image', 'last_modified')
)
def update_output(content, filename, date):
    if content is not None:
        return html.Div([
            html.H5(f"Imagem enviada: {filename}"),
            html.Img(src=content, style={'width': '50%', 'marginTop': '20px'})
        ])
    return ""

conteudo_simulacao = html.Div([
    html.Label("Cultura:"),
    dcc.Dropdown(id="cultura-dropdown", options=[
        {"label": "Milho", "value": "milho"},
        {"label": "Soja", "value": "soja"},
        {"label": "Cana-de-açúcar", "value": "cana"},
    ], placeholder="Escolha a cultura"),

    html.Label("Fase da Lavoura:"),
    dcc.Dropdown(id="fase-dropdown", options=[
        {"label": "Plantio", "value": "plantio"},
        {"label": "Crescimento", "value": "crescimento"},
        {"label": "Colheita", "value": "colheita"}
    ], placeholder="Selecione uma fase da lavoura"),
    
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
    html.Label("Práticas Sustentáveis:"),
    dcc.Checklist(
        id="sustentavel-checklist",
        options=[
            {"label": "Irrigação Inteligente", "value": "irrigacao"},
            {"label": "Cobertura vegetal", "value": "cobertura"},
            {"label": "Rotação de culturas", "value": "rotacao"}
        ],
        value=["irrigacao"],
        labelStyle={"display": "block"}
    ),

    html.Br(),
    dbc.Button("Simular", id="simular-btn", color="success", className="me-2"),

    html.Div(id="resultado-simulacao", style={"marginTop": "30px"})
])

@app.callback(
    Output("resultado-simulacao", "children"),
    Input("simular-btn", "n_clicks"),
    [State("cultura-dropdown", "value"),
     State("fase-dropdown", "value"),
     State("clima-radio", "value"),
     State("solo-dropdown", "value"),
     State("sustentavel-checklist", "value")],
    prevent_initial_call=True
)
def simular(n_simular, cultura, fase, clima, solo, praticas):
    if not n_simular:
        return ""

    riscos = {
        "seco": "⚠️ Risco de estresse hídrico devido à condição seca.",
        "chuva": "⚠️ Risco de encharcamento e proliferação de fungos.",
        "calor": "⚠️ Temperaturas elevadas podem afetar a fotossíntese."
    }

    pontuacao = 50
    if "irrigacao" in praticas:
        pontuacao += 20
    if "cobertura" in praticas:
        pontuacao += 15
    if "rotacao" in praticas:
        pontuacao += 15

    sugestoes = [
        "💡 Evite irrigar entre 12h e 15h em dias muito quentes.",
        "💡 Utilize sensores de solo para adaptar a irrigação.",
        "💡 Considere adubação orgânica na fase de crescimento."
    ]

    return html.Div([
        html.H5("📌 Mensagem de Risco"),
        html.P(riscos.get(clima, "Condições climáticas normais."), style={"color": "red"}),

        html.H5("📊 Comparação de Cenários"),
        dcc.Graph(
            figure={
                "data": [
                    {"x": ["Sem Sustentabilidade", "Com Sustentabilidade"], "y": [50, pontuacao], "type": "bar"}
                ],
                "layout": {"height": 300}
            }
        ),

        html.H5("📋 Sugestões da IA"),
        html.Ul([html.Li(s) for s in sugestoes]),

        html.H5("🏆 Pontuação de Sustentabilidade"),
        html.Div(f"{pontuacao}/100 pontos", style={"fontSize": "20px", "color": "green"}),

        html.H5("📄 Relatório Resumido"),
        html.Ul([
            html.Li(f"Cultura: {cultura}"),
            html.Li(f"Fase: {fase}"),
            html.Li(f"Clima: {clima}"),
            html.Li(f"Solo: {solo}"),
            html.Li(f"Práticas adotadas: {', '.join(praticas)}"),
            html.Li(f"Pontuação final: {pontuacao}/100")
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
    Output("store-mensagens", "data"),
    Input("btn-enviar-ia", "n_clicks"),
    State("input-ia", "value"),
    State("store-mensagens", "data"),
    prevent_initial_call=True
)
def enviar_mensagem_ia(n, mensagem, mensagens):
    if mensagem:
        mensagens.append({"autor": "Você", "texto": mensagem})
        mensagens.append({"autor": "IA", "texto": f"Simulação de resposta para: '{mensagem}'"})
    return mensagens

@app.callback(
    Output("chat-container", "children"),
    Input("store-mensagens", "data")
)
def atualizar_chat_ia(mensagens):
    chat = []
    for msg in mensagens:
        estilo = {
            "padding": "8px",
            "margin": "5px 0",
            "borderRadius": "8px",
            "maxWidth": "80%"
        }
        if msg["autor"] == "Você":
            estilo.update({"backgroundColor": "#d1e7dd", "alignSelf": "flex-end", "textAlign": "right"})
        else:
            estilo.update({"backgroundColor": "#f8d7da", "alignSelf": "flex-start", "textAlign": "left"})

        chat.append(html.Div([
            html.Small(f"{msg['autor']}:", style={"fontWeight": "bold"}),
            html.Div(msg["texto"])
        ], style=estilo))
    return html.Div(chat, style={"display": "flex", "flexDirection": "column"})

if __name__ == "__main__":
    app.run(debug=True)
