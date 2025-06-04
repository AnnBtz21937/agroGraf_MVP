from dash import html, dcc
import dash_bootstrap_components as dbc

layout = html.Div([
    html.H3("Configurações"),

    # Modo de exibição
    html.Label("Modo de exibição:"),
    dcc.RadioItems(
        options=[
            {"label": "Claro", "value": "claro"},
            {"label": "Escuro", "value": "escuro"}
        ],
        value="claro",
        id="modo-exibicao",
        labelStyle={"display": "inline-block", "marginRight": "15px"}
    ),

    html.Br(),

    # Preferências de Notificações
    html.Label("Notificações:"),
    dcc.Checklist(
        options=[
            {"label": "Email", "value": "email"},
            {"label": "Som", "value": "som"},
            {"label": "IA", "value": "ia"}
        ],
        value=["email", "ia"],
        id="notificacoes-checklist",
        labelStyle={"display": "block"}
    ),

    html.Br(),

    # Idioma da interface
    html.Label("Idioma:"),
    dcc.Dropdown(
        options=[
            {"label": "Português", "value": "pt"},
            {"label": "Inglês", "value": "en"},
            {"label": "Espanhol", "value": "es"}
        ],
        value="pt",
        id="idioma-dropdown",
        style={"width": "50%"}
    ),

    html.Br(),

    # Atualizações automáticas
    html.Label("Atualizações automáticas de dados:"),
    dcc.RadioItems(
        options=[
            {"label": "Ativado", "value": "sim"},
            {"label": "Desativado", "value": "nao"}
        ],
        value="sim",
        id="atualizacao-radio",
        labelStyle={"display": "inline-block", "marginRight": "15px"}
    ),

    html.Hr(),
# Recursos Offline
html.H4("Recursos Offline"),
html.P("Essas opções funcionam mesmo sem conexão com a internet."),

dcc.Checklist(
    id="offline-opcoes-checklist",
    options=[
        {"label": "Visualizar últimas simulações salvas no navegador", "value": "ver_simulacoes"},
        {"label": "Exportar dados simulados em PDF", "value": "exportar_pdf"},
        {"label": "Acessar guia rápido de boas práticas agrícolas", "value": "guia_boas_praticas"}
    ],
    value=["ver_simulacoes", "guia_boas_praticas"],
    labelStyle={"display": "block", "marginBottom": "10px"}
),

dcc.Checklist(
    id="modo-offline-checklist",
    options=[
        {"label": "Ativar modo offline automaticamente sem internet", "value": "auto_offline"}
    ],
    value=[],
    labelStyle={"display": "block"}
),

dbc.Button("Baixar Guia de Boas Práticas Agrícolas", id="btn-download-guia", color="secondary", className="mb-3"),
html.Div(id="guia-feedback", style={"marginTop": "10px"}),

    html.Hr(),

    dbc.Button("Salvar configurações", id="salvar-config", color="success", className="mt-2")
])
