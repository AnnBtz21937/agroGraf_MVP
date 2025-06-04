      
# CMakeLists.txt para o projeto AgroGraf
# Desenvolvido por: Ricardo Cristiano da Silva
# Target: Raspberry Pi Pico W (BitDogLab)

# Versão mínima do CMake requerida
cmake_minimum_required(VERSION 3.13)

# Define os padrões C e C++ a serem usados
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
# Gera o arquivo compile_commands.json, útil para algumas IDEs e linters
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == NÃO EDITE AS LINHAS A SEGUIR para que a Extensão VS Code do Raspberry Pi Pico funcione ==
# Define o diretório home do usuário de forma portável
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
# Versões das ferramentas (gerenciadas pela extensão VSCode)
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
# Inclui configurações específicas do VSCode para o Pico, se existir
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Define a placa alvo como pico_w (Raspberry Pi Pico W)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Inclui o SDK do Raspberry Pi Pico (deve vir antes da definição do projeto)
include(pico_sdk_import.cmake)

# ===== DEFINIÇÃO DO NOME DO PROJETO =====
# Define o nome do projeto e as linguagens utilizadas (C, C++, Assembly)
project(agrograf C CXX ASM) # Nome do projeto: agrograf
# ======================================

# Inicializa o SDK do Raspberry Pi Pico para o projeto atual
pico_sdk_init()

# ===== DEFINIÇÃO DO EXECUTÁVEL E ARQUIVOS FONTE =====
# Adiciona um executável ao projeto
add_executable(agrograf # Nome do executável (geralmente igual ao nome do projeto)
    agrograf.c          # Arquivo fonte principal C
    inc/ssd1306_i2c.c   # Arquivo fonte para a biblioteca do display OLED (comunicação I2C)
)
# =======================================================

# pico_set_program_name(agrograf "agrograf") # Redundante se o nome do projeto já é "agrograf"
# pico_set_program_version(agrograf "0.1")   # Opcional: define a versão do programa

# Gera o arquivo de cabeçalho (.h) a partir do arquivo .pio para o WS2812B
# O executável 'agrograf' precisa deste header gerado.
pico_generate_pio_header(agrograf ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

# Modifique as linhas abaixo para habilitar/desabilitar saída via UART/USB
# Habilita a saída padrão (stdio) via UART0 (pinos GP0/GP1 por padrão) - 0 desabilita
pico_enable_stdio_uart(agrograf 0)
# Habilita a saída padrão (stdio) via USB CDC - 1 habilita
pico_enable_stdio_usb(agrograf 1)

# Adiciona as bibliotecas padrão do SDK do Pico necessárias para o projeto
target_link_libraries(agrograf
    pico_stdlib                               # Biblioteca padrão do Pico (timers, gpio básico, etc.)
    hardware_pio                              # Suporte para Programmable I/O (usado pelo WS2812B)
    hardware_clocks                           # Suporte para gerenciamento de clocks (usado pelo PIO)
    hardware_adc                              # Suporte para Conversor Analógico-Digital (joystick, temp)
    hardware_i2c                              # Suporte para comunicação I2C (display OLED)
    hardware_pwm                              # Suporte para Pulse Width Modulation (buzzer)
    pico_cyw43_arch_lwip_threadsafe_background # Suporte para Wi-Fi (CYW43) com lwIP em background e thread-safe
)

# Adiciona os diretórios de include ao projeto
target_include_directories(agrograf PRIVATE
    ${CMAKE_CURRENT_LIST_DIR} # Diretório atual do projeto (onde está o CMakeLists.txt)
    inc                       # Diretório 'inc' para cabeçalhos customizados (ex: ssd1306.h)
)

# ===== GERAÇÃO DE ARQUIVOS DE SAÍDA ADICIONAIS =====
# Adiciona a geração de arquivos de saída extras como .uf2, .hex, .bin, etc.
# para o executável 'agrograf'.
pico_add_extra_outputs(agrograf)
# ===============================================

    