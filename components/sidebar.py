from dash import html, dcc

def render_sidebar():
    return html.Div([
        html.H2("AgroGraf", style={"color": "white", "textAlign": "center", "marginBottom": "30px"}),
        dcc.Link("Dashboard", href="/", style={"display": "block", "margin": "10px 0", "color": "white", "textDecoration": "none"}),
        dcc.Link("Simulador IA", href="/simulador", style={"display": "block", "margin": "10px 0", "color": "white", "textDecoration": "none"}),
        dcc.Link("Identificação de Pragas", href="/identificacao", style={"display": "block", "margin": "10px 0", "color": "white", "textDecoration": "none"}),
        dcc.Link("Alarmes", href="/alarmes", style={"display": "block", "margin": "10px 0", "color": "white", "textDecoration": "none"}),
        dcc.Link("Relatórios", href="/relatorios", style={"display": "block", "margin": "10px 0", "color": "white", "textDecoration": "none"}),
        dcc.Link("Configurações", href="/configuracoes", style={"display": "block", "margin": "10px 0", "color": "white", "textDecoration": "none"}),
    ], style={
        "position": "fixed", "top": "60px", "left": 0, "bottom": 0,
        "width": "15rem", "padding": "1rem", "backgroundColor": "#234F1E"
    })

