from dash import html, dcc, dash_table
import dash_bootstrap_components as dbc

relatorios_salvos = [
    {"id": 1, "tipo": "Diário", "data": "2025-05-27"},
    {"id": 2, "tipo": "Semanal", "data": "2025-05-20"},
    {"id": 3, "tipo": "Mensal", "data": "2025-05-01"},
    {"id": 4, "tipo": "Diário", "data": "2025-05-15"},
    {"id": 5, "tipo": "Mensal", "data": "2025-04-01"},
]

layout = html.Div([
    html.H3("Relatórios"),

    dbc.Row([
        dbc.Col([
            html.Label("Tipo de Relatório:"),
            dcc.Dropdown(
                id='tipo-relatorio',
                options=[
                    {'label': 'Diário', 'value': 'diario'},
                    {'label': 'Semanal', 'value': 'semanal'},
                    {'label': 'Mensal', 'value': 'mensal'}
                ],
                placeholder="Selecione um tipo",
                value='diario',
                clearable=False
            )
        ], md=6),

        dbc.Col([
            html.Label("Data de Referência:"),
            dcc.DatePickerSingle(
                id='data-relatorio',
                placeholder='Selecione uma data',
                date=None
            )
        ], md=6)
    ], className="mb-4"),

    dbc.Row([
        dbc.Col([
            dbc.Button("Gerar Relatório", id="btn-gerar", color="primary", className="me-2"),
        ])
    ]),

    html.Hr(),

    html.Div(id="conteudo-relatorio", style={"marginTop": "2rem"}),

    html.H4("Relatórios Salvos", className="mt-4"),

    dash_table.DataTable(
        id='tabela-relatorios',
        columns=[
            {"name": "ID", "id": "id"},
            {"name": "Tipo", "id": "tipo"},
            {"name": "Data", "id": "data"},
            {"name": "Editar", "id": "editar", "presentation": "markdown"},
            {"name": "Excluir", "id": "excluir", "presentation": "markdown"}
        ],
        data=[
            {
                **relatorio,
                "editar": f"[ Editar](#)",
                "excluir": f"[ Excluir](#)"
            }
            for relatorio in relatorios_salvos
        ],
        style_cell={'textAlign': 'center'},
        style_header={'fontWeight': 'bold'},
        style_table={'overflowX': 'auto'},
        markdown_options={"link_target": "_self"}
    )
])
