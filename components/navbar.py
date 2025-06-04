from dash import html, dcc
from dash_iconify import DashIconify
import dash_bootstrap_components as dbc

def render_navbar():
    return dbc.Navbar(
        dbc.Container([
            html.Div(""),
            dbc.Input(type="search", placeholder="Buscar...", className="me-2", style={"width": "200px"}),

            dbc.Nav([
                dbc.Button(DashIconify(icon="carbon:accessibility", width=24), id="btn-acess", color="light", className="me-2"),
                dbc.Button(DashIconify(icon="carbon:notification", width=24), id="btn-notificacoes", color="light", className="me-2"),
                dbc.Button(DashIconify(icon="carbon:user-avatar", width=24), id="btn-user", color="light"),
            ], className="ms-auto", navbar=True),

            # Modal Acessibilidade 
            dbc.Modal([
                dbc.ModalHeader("Acessibilidade"),
                dbc.ModalBody([
                    html.Button("Contraste Alto", id="acess-contraste", style={"background": "none", "border": "none", "cursor": "pointer", "marginBottom": "10px", "display": "block"}),
                    html.Button("Aumentar Fonte", id="acess-fonte", style={"background": "none", "border": "none", "cursor": "pointer", "marginBottom": "10px", "display": "block"}),
                    html.Button("Leitura em Voz Alta", id="acess-voz", style={"background": "none", "border": "none", "cursor": "pointer", "display": "block"})
                ])
            ], id="modal-acess", is_open=False),

            # Modal Notificações 
            dbc.Modal([
                dbc.ModalHeader("Notificações"),
                dbc.ModalBody([
                    html.P("💧 Umidade baixa na plantação de milho."),
                    html.P("🌾 Colheita agendada para amanhã."),
                    html.P("📡 Sensor da estufa desconectado.")
                ])
            ], id="modal-notificacoes", is_open=False),

            # Modal Usuário 
            dbc.Modal([
                dbc.ModalHeader("Perfil do Usuário"),
                dbc.ModalBody([
                    html.Button("Ver Perfil", id="user-ver", style={"background": "none", "border": "none", "cursor": "pointer", "marginBottom": "10px", "display": "block"}),
                    html.Button("Editar Dados", id="user-editar", style={"background": "none", "border": "none", "cursor": "pointer", "marginBottom": "10px", "display": "block"}),
                    html.Button("Sair", id="user-sair", style={"background": "none", "border": "none", "cursor": "pointer", "display": "block"})
                ])
            ], id="modal-user", is_open=False),
        ]),
        color="light", dark=False, className="mb-4", style={"height": "60px"}
    )
