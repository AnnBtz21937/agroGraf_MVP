from dash import html, dcc
import dash_bootstrap_components as dbc

layout = html.Div([
    html.H3("Alarmes de Desastres Naturais"),
    html.P("Sistema de detecção e alerta para incêndios, alagamentos e outros desastres."),

    # Formulário para adicionar novo alarme
    html.Div([
        html.H5("Adicionar Novo Alarme"),
        dcc.Input(id="novo-texto-alarme", type="text", placeholder="Descrição do alarme", style={"width": "60%", "marginRight": "10px"}),
        dcc.Dropdown(
            id="novo-tipo-alarme",
            options=[
                {"label": "Incêndio", "value": "danger"},
                {"label": "Alagamento", "value": "primary"},
                {"label": "Vento Forte", "value": "warning"},
                {"label": "Outro", "value": "secondary"}
            ],
            placeholder="Tipo de alarme",
            style={"width": "30%", "display": "inline-block", "marginRight": "10px"}
        ),
        dbc.Button("Adicionar", id="btn-adicionar-alarme", color="success", className="mt-2"),
    ], style={"marginBottom": "30px"}),

    # Lista de alarmes simulados
    html.Div([

        dbc.Alert([
            html.Div("🔥 Incêndio detectado na zona norte!", style={"flex": "1"}),
            html.Div([
                dbc.Button("Editar", color="warning", size="sm", className="me-2"),
                dbc.Button("Excluir", color="danger", size="sm"),
            ])
        ], color="danger", className="d-flex justify-content-between align-items-center"),

        dbc.Alert([
            html.Div("🌊 Risco de alagamento na zona sul!", style={"flex": "1"}),
            html.Div([
                dbc.Button("Editar", color="warning", size="sm", className="me-2"),
                dbc.Button("Excluir", color="danger", size="sm"),
            ])
        ], color="primary", className="d-flex justify-content-between align-items-center"),

        dbc.Alert([
            html.Div("🌀 Vento forte em direção às plantações!", style={"flex": "1"}),
            html.Div([
                dbc.Button("Editar", color="warning", size="sm", className="me-2"),
                dbc.Button("Excluir", color="danger", size="sm"),
            ])
        ], color="warning", className="d-flex justify-content-between align-items-center"),

        dbc.Alert([
            html.Div("⚠️ Sensores inativos na área oeste.", style={"flex": "1"}),
            html.Div([
                dbc.Button("Editar", color="warning", size="sm", className="me-2"),
                dbc.Button("Excluir", color="danger", size="sm"),
            ])
        ], color="secondary", className="d-flex justify-content-between align-items-center"),

        dbc.Alert([
            html.Div("🌪️ Possível formação de ciclone na região central.", style={"flex": "1"}),
            html.Div([
                dbc.Button("Editar", color="warning", size="sm", className="me-2"),
                dbc.Button("Excluir", color="danger", size="sm"),
            ])
        ], color="danger", className="d-flex justify-content-between align-items-center"),
    ])
])
