from dash import html, dcc
import dash_bootstrap_components as dbc
from dash_iconify import DashIconify

layout = html.Div([
    html.H3("Identificação de Pragas e Doenças"),

    html.P("Envie uma imagem para tentar identificar pragas nas lavouras."),

    dcc.Upload(
        id='upload-image',
        children=html.Div([
            'Arraste ou clique para fazer upload de uma imagem da lavoura'
        ]),
        style={
            'width': '100%',
            'height': '120px',
            'lineHeight': '120px',
            'borderWidth': '2px',
            'borderStyle': 'dashed',
            'borderRadius': '10px',
            'textAlign': 'center',
            'margin': '20px'
        },
        multiple=False
    ),

    html.Div(id='output-image-upload'),

    html.Br(),

    html.H5("Confiança da IA na identificação:"),
    html.P("Alta", style={"color": "green"}),

    html.H5("Sugestões de Ação:"),
    html.Ul([
        html.Li("Aplicar inseticida natural nas próximas 48 horas."),
        html.Li("Monitorar novas áreas próximas."),
        html.Li("Evitar irrigação nas áreas afetadas temporariamente.")
    ]),

    html.Br(),

    html.H5("Comparação Visual:"),
    html.Div([
        html.Div([
            html.P("Imagem Enviada", style={"textAlign": "center"}),
            html.Div([
                DashIconify(icon="carbon:image", width=64)
            ], style={
                "width": "100%",
                "height": "180px",
                "border": "1px dashed #ccc",
                "display": "flex",
                "alignItems": "center",
                "justifyContent": "center",
                "backgroundColor": "#f9f9f9"
            })
        ], style={"width": "45%", "display": "inline-block"}),

        html.Div([
            html.P("Imagem de Referência", style={"textAlign": "center"}),
            html.Div([
                DashIconify(icon="carbon:image", width=64)
            ], style={
                "width": "100%",
                "height": "180px",
                "border": "1px dashed #ccc",
                "display": "flex",
                "alignItems": "center",
                "justifyContent": "center",
                "backgroundColor": "#f9f9f9"
            })
        ], style={"width": "45%", "display": "inline-block", "marginLeft": "5%"})
    ]),

    html.Br(),

    html.H5("Histórico de Diagnósticos:"),
    html.Table([
        html.Thead(html.Tr([
            html.Th("Data"), html.Th("Praga Detectada"), html.Th("Ações")
        ])),
        html.Tbody([
            html.Tr([html.Td("27/05/2025"), html.Td("Lagarta-do-cartucho"), html.Td("Editar | Excluir")]),
            html.Tr([html.Td("26/05/2025"), html.Td("Pulgão-verde"), html.Td("Editar | Excluir")]),
            html.Tr([html.Td("25/05/2025"), html.Td("Mosca branca"), html.Td("Editar | Excluir")])
        ])
    ], style={"width": "100%", "marginTop": "10px", "borderCollapse": "collapse"}),

    html.Br(),

    html.Div([
        dbc.Button("Enviar para análise humana", id="btn-analise-especializada", color="secondary", className="me-2"),
        dbc.Button("Salvar Diagnóstico", id="btn-salvar", color="success")
    ])
])
