/**
 * @file agrograf.c
 * @brief Sistema de monitoramento AgroGraf para Raspberry Pi Pico W.
 * @details Este sistema utiliza uma matriz de LEDs WS2812B, joystick, botões,
 *          display OLED SSD1306, sensor de temperatura onboard e buzzer para
 *          monitorar e gerenciar setores agrícolas simulados.
 *          Inclui um servidor HTTP para visualização e controle remoto.
 * @author Ricardo Cristiano da Silva
 * @date (03/06/2025)
 */

// Inclui as bibliotecas padrão e específicas do Pico
#include <stdio.h>          // Para entrada/saída padrão (printf, scanf)
#include "pico/stdlib.h"     // Funções utilitárias padrão do SDK do Pico
#include "hardware/adc.h"      // Para o conversor Analógico-Digital (joystick, sensor de temp.)
#include "hardware/pio.h"      // Para o Programmable I/O (controlar WS2812B)
#include "hardware/clocks.h"   // Para gerenciamento de clocks (usado pelo PIO)
#include "ws2818b.pio.h"   // Arquivo de header gerado pelo pioasm para o WS2812B
#include "hardware/gpio.h"     // Para controle de General Purpose Input/Output (botões)

// Inclui as bibliotecas para controlar o display OLED:
#include <string.h>         // Para manipulação de strings (strcpy, strlen)
#include <stdlib.h>         // Funções utilitárias gerais (atoi, etc.) - Menos usado aqui
#include <ctype.h>          // Para manipulação de caracteres (toupper, isdigit) - Menos usado aqui
#include "pico/binary_info.h"// Para adicionar informações ao binário (usado pela lib SSD1306)
#include "inc/ssd1306.h"     // Biblioteca principal para o display OLED SSD1306
#include "hardware/i2c.h"      // Para comunicação I2C (usada pelo OLED)

// **INCLUSÕES DO BUZZER**
#include "hardware/pwm.h"      // Para controle de Pulse Width Modulation (usado pelo buzzer)

// ===== ADIÇÕES PARA WIFI HTTP SERVER =====
#include "pico/cyw43_arch.h" // Para arquitetura específica do chip Wi-Fi CYW43
#include "lwip/tcp.h"          // Para funcionalidades TCP do stack lwIP
#include "lwip/ip4_addr.h"     // Para manipulação de endereços IPv4
// =========================================

// Definições para a matriz de LEDs WS2812B
#define LED_COUNT 25           // Número total de LEDs na matriz (5x5)
#define LED_PIN 7              // Pino GPIO conectado ao DIN da matriz de LEDs

// Definições para o joystick
#define ADC_X_PIN 27           // Pino GPIO para o eixo X do joystick (ADC1)
#define ADC_Y_PIN 26           // Pino GPIO para o eixo Y do joystick (ADC0)
#define JOYSTICK_BUTTON_PIN 22 // Pino GPIO para o botão do joystick

// Definições para o botão A e B (assumindo que são botões externos da BitDogLab)
#define BUTTON_A 5             // Pino GPIO para o Botão A
#define BUTTON_B 6             // Pino GPIO para o Botão B

// Definições para os pinos I2C (usados pelo display OLED)
const uint I2C_SDA = 14;       // Pino GPIO para o SDA do I2C1
const uint I2C_SCL = 15;       // Pino GPIO para o SCL do I2C1

// Definições para a leitura de temperatura
#define TEMPERATURE_UNITS 'C'  // Unidade padrão para temperatura ('C' para Celsius, 'F' para Fahrenheit)
#define ADC_TEMP_PIN 4         // Canal ADC para o sensor de temperatura interno do RP2040

// **DEFINIÇÕES DO BUZZER**
#define BUZZER_PIN 21          // Pino GPIO conectado ao buzzer

// ===== DEFINIÇÕES PARA WIFI HTTP SERVER =====
#define WIFI_SSID "Colocar o nome da sua rede WiFi aqui"      // Nome da rede Wi-Fi (SSID)
#define WIFI_PASS "Colocar a senha da sua rede WiFi aqui"   // Senha da rede Wi-Fi
// ===========================================

// Estruturas de dados

/**
 * @struct pixel_t
 * @brief Estrutura para representar a cor de um pixel RGB.
 */
struct pixel_t {
    uint8_t G, R, B; // Componentes Verde, Vermelho e Azul da cor (ordem G R B para WS2812B)
};
typedef struct pixel_t pixel_t; // Define pixel_t como um tipo
typedef pixel_t npLED_t;       // Define npLED_t como um alias para pixel_t (específico para NeoPixel)

// Array para armazenar o estado de cor de cada LED da matriz
npLED_t leds[LED_COUNT];
// Instância do PIO a ser usada (pio0 ou pio1)
PIO np_pio = pio0;
// State Machine (SM) do PIO a ser usada
uint sm;

// Matriz booleana para rastrear o estado de cadastro de cada LED (true = cadastrado)
// Nota: `setor_cadastrado` é mais diretamente usado para a lógica de cadastro.
// `led_states` parece ser uma redundância ou um resquício.
bool led_states[5][5] = {false};

// Protótipos das funções (declarações antecipadas)

// Funções para controle da matriz de LEDs (Neopixel WS2812B)
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
int getIndex(int x, int y); // Converte coordenadas (x,y) da matriz para índice linear
void clear_led_states();     // Reseta o estado dos LEDs (matriz `led_states`)

// Funções para botões
void init_button(uint pin);
bool read_button(uint pin);

// Funções de interface com o usuário (menus via serial)
void show_menu();
void show_setores_menu();

// Funções de gerenciamento do sistema
void clearSystem();          // Reseta o estado geral do sistema AgroGraf
float read_onboard_temperature(const char unit); // Lê a temperatura do sensor interno
void listar_setores();       // Lista os setores cadastrados e suas temperaturas
void mudar_temperatura_setor(); // Permite alterar a temperatura de um setor
void update_led_colors();    // Atualiza as cores dos LEDs na matriz baseado no estado dos setores
void desligarLedAzul();     // Restaura a cor do LED que estava sob o cursor azul
void acionar_equipamentos_contra_incendio(); // Simula acionamento de equipamentos e reseta temperaturas altas

// Funções para o Buzzer
void pwm_init_buzzer(uint pin); // Inicializa o PWM para o buzzer
void beep(uint pin, uint duration_ms); // Aciona o buzzer (duration_ms não usado para parar)
void stop_tone(uint pin);       // Para o som do buzzer

// Declaração de variáveis globais

#define MAX_SETORES 25         // Número máximo de setores (corresponde ao LED_COUNT)
char nomes_setores[MAX_SETORES][30]; // Array para armazenar nomes dos setores
float temperaturas_setores[MAX_SETORES]; // Array para armazenar temperaturas dos setores
bool setor_cadastrado[MAX_SETORES];    // Array para rastrear se um setor está cadastrado

// Posição atual do cursor na matriz de LEDs (usado no modo de cadastro)
int current_x = 0;
int current_y = 0;

// Definições de cores padrão
uint8_t blue_r = 0, blue_g = 0, blue_b = 128;   // Cor azul para o cursor
uint8_t green_r = 0, green_g = 128, green_b = 0; // Cor verde para setor OK
uint8_t red_r = 128, red_g = 0, red_b = 0;     // Cor vermelha para setor em alerta

// Estado do buzzer
bool buzzer_ativo = false;
// Escolha do usuário no menu principal
int main_menu_choice;

// ===== VARIÁVEIS GLOBAIS PARA WIFI HTTP SERVER =====
char http_response_buffer[1536]; // Buffer para armazenar a resposta HTTP
// ==================================================

/**
 * @brief Reseta o sistema AgroGraf para seu estado inicial.
 * @details Limpa a matriz de LEDs, reseta os estados de cadastro e temperaturas dos setores,
 *          posiciona o cursor no centro e desliga o buzzer se estiver ativo.
 */
void clearSystem() {
    npClear();             // Apaga todos os LEDs da matriz
    clear_led_states();    // Reseta a matriz auxiliar `led_states` (uso questionável)
    current_x = 2;         // Reseta o cursor para a posição central (x=2)
    current_y = 2;         // Reseta o cursor para a posição central (y=2)
    npWrite();             // Envia os dados para a matriz de LEDs (agora apagada)

    // Lê a temperatura ambiente atual do sensor onboard
    float temperatura_ambiente = read_onboard_temperature(TEMPERATURE_UNITS);
    // Itera por todos os setores possíveis
    for (int i = 0; i < MAX_SETORES; i++) {
        setor_cadastrado[i] = false;               // Marca o setor como não cadastrado
        temperaturas_setores[i] = temperatura_ambiente; // Define a temperatura do setor para a ambiente
    }
    // Se o buzzer estiver ativo, desliga-o
    if (buzzer_ativo) {
        stop_tone(BUZZER_PIN);
        buzzer_ativo = false;
    }
    printf("Sistema AgroGraf limpo.\n");
}

/**
 * @brief Constrói a página HTML de status e controle para o servidor HTTP.
 * @details Gera uma página HTML contendo o status dos setores, temperatura,
 *          estado do buzzer e links para ações como resetar alarmes e limpar o sistema.
 *          A página se auto-atualiza a cada 5 segundos.
 */
void build_http_response() {
    char status_info[1024] = ""; // Buffer para informações de status dos setores
    char temp_buf[100];          // Buffer temporário para formatação de strings
    int Sprintf_Num_Local = 0;   // Contador de bytes escritos em status_info (evita overflow)

    // Adiciona cabeçalho para a lista de status dos setores
    Sprintf_Num_Local += sprintf(status_info + Sprintf_Num_Local, "<h2>Status dos Setores:</h2><ul>");
    // Itera por todos os setores
    for (int i = 0; i < MAX_SETORES; i++) {
        // Se o setor estiver cadastrado, adiciona suas informações à lista
        if (setor_cadastrado[i]) {
            // Verifica se há espaço suficiente no buffer status_info
            if (Sprintf_Num_Local < sizeof(status_info) - 100) { // -100 para margem de segurança
                // Formata a string do setor (nome, índice, temperatura, alerta)
                sprintf(temp_buf, "<li>%s (Indice %d): %.2f C %s</li>",
                        nomes_setores[i],
                        i + 1, // Índice para o usuário (1-25)
                        temperaturas_setores[i],
                        (temperaturas_setores[i] > 100.0f) ? "<b>(ALERTA!)</b>" : ""); // Alerta se temp > 100
                // Adiciona a string formatada ao buffer principal de status
                Sprintf_Num_Local += sprintf(status_info + Sprintf_Num_Local, "%s", temp_buf);
            }
        }
    }
    // Fecha a lista HTML e adiciona status do buzzer
    if (Sprintf_Num_Local < sizeof(status_info) - 80) Sprintf_Num_Local += sprintf(status_info + Sprintf_Num_Local, "</ul>");
    if (Sprintf_Num_Local < sizeof(status_info) - 50) Sprintf_Num_Local += sprintf(status_info + Sprintf_Num_Local, "<p>Buzzer: %s</p>", buzzer_ativo ? "ATIVO" : "DESATIVADO");

    // Monta a resposta HTTP completa
    sprintf(http_response_buffer,
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n" // Cabeçalho HTTP
            "<!DOCTYPE html><html><head><title>AgroGraf Control</title>"
            "<meta http-equiv='refresh' content='5'>" // Auto-refresh da página a cada 5s
            "</head><body>"
            "<h1>AgroGraf - Controle Remoto</h1>"
            "%s" // Insere as informações de status dos setores aqui
            "<h2>Acoes:</h2>"
            "<p><a href=\"/reset_alarms\">Acionar Equipamentos (Resetar Alarmes)</a></p>" // Link para resetar alarmes
            "<p><a href=\"/clear_system\">Limpar Sistema</a></p>"                     // Link para limpar o sistema
            "</body></html>\r\n",
            status_info);
}

/**
 * @brief Callback para lidar com requisições HTTP recebidas.
 * @param arg Argumento passado para o callback (não utilizado aqui).
 * @param tpcb Ponteiro para a estrutura de controle do TCP.
 * @param p Ponteiro para o buffer de pacotes (pbuf) contendo os dados recebidos.
 * @param err Código de erro (se houver).
 * @return err_t Código de erro lwIP. ERR_OK se bem sucedido.
 * @details Processa requisições GET para "/reset_alarms" e "/clear_system".
 *          Para qualquer outra requisição GET (ou a raiz "/"), envia a página de status.
 */
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    // Se p for NULL, significa que a conexão foi fechada pelo cliente ou houve um erro grave
    if (p == NULL) {
        tcp_close(tpcb); // Fecha a conexão TCP
        return ERR_OK;
    }
    char *request = (char *)p->payload; // Converte o payload do pacote para string

    // Verifica se a requisição contém "GET /reset_alarms"
    if (strstr(request, "GET /reset_alarms")) {
        acionar_equipamentos_contra_incendio(); // Chama a função para resetar alarmes
    }
    // Verifica se a requisição contém "GET /clear_system"
    else if (strstr(request, "GET /clear_system")) {
        clearSystem(); // Chama a função para limpar o sistema
    }

    // Constrói a resposta HTTP (página de status)
    build_http_response();
    // Envia a resposta HTTP para o cliente
    err_t write_err = tcp_write(tpcb, http_response_buffer, strlen(http_response_buffer), TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK) {
        printf("Erro ao enviar resposta HTTP: %d\n", write_err);
    }
    pbuf_free(p); // Libera o buffer do pacote recebido
    return ERR_OK;
}

/**
 * @brief Callback para aceitar novas conexões TCP.
 * @param arg Argumento passado para o callback (não utilizado aqui).
 * @param newpcb Ponteiro para a nova estrutura de controle TCP da conexão aceita.
 * @param err Código de erro (se houver).
 * @return err_t Código de erro lwIP. ERR_OK se bem sucedido, ERR_MEM se faltar memória.
 */
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    // Se newpcb for NULL, pode ser um erro de falta de memória
    if (newpcb == NULL) {
        return ERR_MEM; // Retorna erro de memória
    }
    // Define a função http_callback para ser chamada quando dados forem recebidos nesta conexão
    tcp_recv(newpcb, http_callback);
    return ERR_OK;
}

/**
 * @brief Inicializa e inicia o servidor HTTP na porta 80.
 * @details Cria um novo PCB TCP, faz o bind para qualquer endereço IP na porta 80,
 *          coloca o servidor em modo de escuta (listen) e define o callback para aceitar conexões.
 */
static void start_http_server(void) {
    // Cria um novo Protocol Control Block (PCB) para TCP, aceitando qualquer tipo de IP (IPv4/IPv6)
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("Erro ao criar PCB HTTP\n");
        return;
    }
    // Associa (bind) o PCB a qualquer endereço IP local (IP_ANY_TYPE) na porta 80
    err_t bind_err = tcp_bind(pcb, IP_ANY_TYPE, 80);
    if (bind_err != ERR_OK) {
        printf("Erro ao ligar o servidor HTTP na porta 80: %d\n", bind_err);
        tcp_abort(pcb); // Aborta o PCB se o bind falhar
        return;
    }
    // Coloca o PCB em modo de escuta (listen) para novas conexões
    pcb = tcp_listen(pcb);
    if (!pcb) {
        printf("Erro ao colocar PCB em modo listen\n");
        return;
    }
    // Define connection_callback para ser chamado quando uma nova conexão for estabelecida
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP AgroGraf rodando na porta 80...\n");
}

/**
 * @brief Limpa a tela do terminal serial usando sequências de escape ANSI.
 */
void clear_screen() {
    printf("\033[H\033[J"); // Move o cursor para Home (topo, esquerda) e limpa a tela
}

/**
 * @brief Função principal do sistema AgroGraf.
 * @return int Código de saída do programa (0 para sucesso).
 * @details Inicializa todos os periféricos (stdio, Wi-Fi, I2C, OLED, ADC, botões, LEDs, buzzer),
 *          exibe o menu principal e processa as escolhas do usuário em um loop infinito.
 *          Gerencia o estado dos setores, temperaturas, alertas e a interface HTTP.
 */
int main() {
    // Inicializa a E/S padrão (USB e/ou UART)
    stdio_init_all();
    sleep_ms(2000); // Pequena pausa para permitir que o terminal serial se conecte
    clear_screen(); // Limpa a tela do terminal

    // Inicializa o chip Wi-Fi CYW43xxx
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi\n");
    } else {
        printf("Wi-Fi inicializado.\n");
        cyw43_arch_enable_sta_mode(); // Habilita o modo Station (cliente Wi-Fi)
        printf("Conectando ao Wi-Fi '%s'...\n", WIFI_SSID);
        // Tenta conectar à rede Wi-Fi com timeout de 20 segundos
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
            printf("Falha ao conectar ao Wi-Fi.\n");
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // Acende o LED onboard do Pico W (indica erro)
        } else {
            printf("Conectado com sucesso ao Wi-Fi '%s'.\n", WIFI_SSID);
            // Verifica se a interface de rede padrão está configurada
            if (netif_default) {
                printf("Endereco IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
            } else {
                printf("Nao foi possivel obter o endereco IP (netif_default is NULL).\n");
            }
            // Pisca o LED onboard para indicar sucesso na conexão Wi-Fi e servidor pronto
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); sleep_ms(250);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); sleep_ms(250);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); sleep_ms(250);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); sleep_ms(250);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // Deixa o LED aceso

            start_http_server(); // Inicia o servidor HTTP
        }
    }

    // Inicializa a comunicação I2C1 na frequência de 400kHz
    i2c_init(i2c1, 400 * 1000);
    // Configura os pinos GPIO para a função I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // Habilita pull-ups internos para os pinos I2C
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Inicializa o display OLED SSD1306
    ssd1306_init();
    // Define a área de renderização para cobrir todo o display
    struct render_area frame_area = {
        .start_column = 0, .end_column = ssd1306_width - 1,
        .start_page = 0, .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area); // Calcula o tamanho do buffer necessário
    uint8_t ssd[ssd1306_buffer_length]; // Aloca o buffer para o display
    memset(ssd, 0, ssd1306_buffer_length); // Limpa o buffer do display (preenche com 0)
    render_on_display(ssd, &frame_area); // Envia o buffer limpo para o display (apaga a tela)
    // Mensagem de boas-vindas no OLED
    char *text[] = {
        "   Bem-vindos   ",
        "   ao AgroGraf  "};
    int y_oled = 0; // Posição Y inicial para o texto no OLED
    for (uint i = 0; i < 2; i++) {
        ssd1306_draw_string(ssd, 5, y_oled, text[i]); // Desenha a string no buffer
        y_oled += 8; // Incrementa a posição Y para a próxima linha
    }
    render_on_display(ssd, &frame_area); // Envia o buffer com o texto para o display

    // Inicializa o ADC
    adc_init();
    // Configura os pinos do joystick como entradas ADC
    adc_gpio_init(ADC_X_PIN);
    adc_gpio_init(ADC_Y_PIN);
    // Inicializa os botões (joystick, A e B)
    init_button(JOYSTICK_BUTTON_PIN);
    init_button(BUTTON_A);
    init_button(BUTTON_B);
    // Inicializa a matriz de LEDs WS2812B
    npInit(LED_PIN);
    // Habilita o sensor de temperatura interno do RP2040
    adc_set_temp_sensor_enabled(true);

    // Variáveis para armazenar o estado anterior dos botões A e B (para detecção de borda)
    bool button_a_last_state = false;
    bool button_b_last_state = false;

    clearSystem(); // Reseta o sistema para o estado inicial
    // Define nomes padrão para os setores
    for (int y_loop = 0; y_loop < 5; y_loop++) {
        for (int x_loop = 0; x_loop < 5; x_loop++) {
            int index = getIndex(x_loop, y_loop);
            sprintf(nomes_setores[index], "Setor (%d,%d)", x_loop + 1, y_loop + 1);
            // A temperatura já foi resetada para a ambiente em clearSystem()
        }
    }
    // Inicializa o PWM para o buzzer
    pwm_init_buzzer(BUZZER_PIN);

    // Loop principal do programa
    while (true) {
        cyw43_arch_poll(); // Realiza o polling necessário para a pilha Wi-Fi e lwIP
        show_menu();       // Exibe o menu principal no terminal serial

        // Lê a escolha do usuário no menu principal
        if (scanf("%d", &main_menu_choice) != 1) {
            main_menu_choice = 5; // Opção inválida padrão
            // Limpa o buffer de entrada em caso de entrada não numérica
            while (getchar() != '\n' && getchar() != EOF);
        }

        // Processa a escolha do usuário
        switch (main_menu_choice) {
            case 1: // Cadastrar/Descadastrar Setores
                clear_screen();
                printf("\nCadastramento de Setores (Joystick). Pressione botao do joystick para sair.\n");
                
                // Início do modo de cadastro:
                // 1. Desenha todos os setores com seu estado atual (verde/vermelho/apagado)
                for (int y_draw = 0; y_draw < 5; y_draw++) {
                    for (int x_draw = 0; x_draw < 5; x_draw++) {
                        int idx = getIndex(x_draw, y_draw);
                        if (setor_cadastrado[idx]) { // Se setor cadastrado
                            if (temperaturas_setores[idx] > 100.0f) npSetLED(idx, red_r, red_g, red_b); // Vermelho se alerta
                            else npSetLED(idx, green_r, green_g, green_b); // Verde se OK
                        } else {
                            npSetLED(idx, 0, 0, 0); // Apagado se não cadastrado
                        }
                    }
                }
                // 2. Desenha o cursor azul por cima, na posição atual
                npSetLED(getIndex(current_x, current_y), blue_r, blue_g, blue_b);
                npWrite(); // Atualiza a matriz física de LEDs

                // Loop do modo de cadastro de setores
                while (true) {
                    cyw43_arch_poll(); // Polling do Wi-Fi
                    // Lê os valores ADC dos eixos X e Y do joystick
                    adc_select_input(1); uint adc_x_raw = adc_read(); // ADC1 é joystick X (definido em ADC_X_PIN)
                    adc_select_input(0); uint adc_y_raw = adc_read(); // ADC0 é joystick Y (definido em ADC_Y_PIN)
                    int new_x = current_x, new_y = current_y; // Posições temporárias para o novo cursor
                    int threshold = 1000; // Limiar para movimento do joystick (centro ~2048)

                    // Lógica de movimento do cursor com base no joystick
                    // Joystick X: < (2048-th) -> esquerda, > (2048+th) -> direita
                    if (adc_x_raw < (2048 - threshold) && current_x > 0) new_x--; // Move para a esquerda
                    if (adc_x_raw > (2048 + threshold) && current_x < 4) new_x++; // Move para a direita
                    // Joystick Y: > (2048+th) -> cima, < (2048-th) -> baixo (invertido devido à montagem/leitura)
                    if (adc_y_raw > (2048 + threshold) && current_y > 0) new_y--; // Move para cima
                    if (adc_y_raw < (2048 - threshold) && current_y < 4) new_y++; // Move para baixo

                    // Verifica se o Botão A foi pressionado (para cadastrar setor)
                    bool button_a_pressed_now = read_button(BUTTON_A);
                    if (button_a_pressed_now && !button_a_last_state) { // Detecção de borda de subida
                        int led_index_cadastro = getIndex(current_x, current_y);
                        led_states[current_x][current_y] = true; // Atualiza matriz `led_states`
                        setor_cadastrado[led_index_cadastro] = true; // Marca setor como cadastrado
                        printf("Setor (%d,%d) cadastrado.\n", current_x + 1, current_y + 1);
                    }
                    button_a_last_state = button_a_pressed_now; // Atualiza estado anterior do botão A

                    // Verifica se o Botão B foi pressionado (para descadastrar setor)
                    bool button_b_pressed_now = read_button(BUTTON_B);
                    if (button_b_pressed_now && !button_b_last_state) { // Detecção de borda de subida
                        int led_index_descadastro = getIndex(current_x, current_y);
                        led_states[current_x][current_y] = false; // Atualiza matriz `led_states`
                        setor_cadastrado[led_index_descadastro] = false; // Marca setor como não cadastrado
                        printf("Setor (%d,%d) descadastrado.\n", current_x + 1, current_y + 1);
                    }
                    button_b_last_state = button_b_pressed_now; // Atualiza estado anterior do botão B

                    // Verifica se o botão do joystick foi pressionado (para sair do modo de cadastro)
                    if (read_button(JOYSTICK_BUTTON_PIN)) {
                        desligarLedAzul(); // Restaura a cor original do LED sob o cursor
                        npWrite();         // Atualiza a matriz física de LEDs
                        break;             // Sai do loop de cadastro
                    }

                    // Se o cursor se moveu
                    if (new_x != current_x || new_y != current_y) {
                        // 1. Restaura a cor da posição antiga do cursor
                        int old_idx = getIndex(current_x, current_y);
                        if (setor_cadastrado[old_idx]) { // Se o setor antigo estava cadastrado
                            if (temperaturas_setores[old_idx] > 100.0f) npSetLED(old_idx, red_r, red_g, red_b); // Vermelho se alerta
                            else npSetLED(old_idx, green_r, green_g, green_b); // Verde se OK
                        } else {
                            npSetLED(old_idx, 0, 0, 0); // Apagado se não cadastrado
                        }
                        current_x = new_x; // Atualiza a posição X do cursor
                        current_y = new_y; // Atualiza a posição Y do cursor
                        // 2. Desenha o cursor azul na nova posição
                        npSetLED(getIndex(current_x, current_y), blue_r, blue_g, blue_b);
                        npWrite(); // Atualiza a matriz física de LEDs
                    }
                    // Se o cursor não moveu, mas o botão A ou B foi pressionado (para atualizar cor imediatamente)
                    else if (button_a_pressed_now || button_b_pressed_now) {
                        // Redesenha o setor sob o cursor com a nova cor (verde ou apagado)
                        int current_idx = getIndex(current_x, current_y);
                        if (setor_cadastrado[current_idx]) { // Se cadastrado (ou acabou de ser)
                             if (temperaturas_setores[current_idx] > 100.0f) npSetLED(current_idx, red_r, red_g, red_b); // Alerta
                             else npSetLED(current_idx, green_r, green_g, green_b); // Verde
                        } else { // Se descadastrado (ou acabou de ser)
                            npSetLED(current_idx, 0, 0, 0); // Apagado
                        }
                        // Redesenha o cursor azul por cima
                        npSetLED(getIndex(current_x, current_y), blue_r, blue_g, blue_b);
                        npWrite(); // Atualiza a matriz física de LEDs
                    }
                    sleep_ms(70); // Pequena pausa para debounce e estabilidade da leitura do joystick
                }
                update_led_colors(); // Garante que o estado dos LEDs reflita o cadastro ao sair
                break;
            case 2: // Limpar Sistema
                clearSystem(); // Chama a função para resetar o sistema
                sleep_ms(1500); // Pausa para o usuário ver a mensagem
                break;
            case 3: // Menu Setores
                show_setores_menu(); // Chama a função que exibe o submenu de setores
                update_led_colors(); // Atualiza cores dos LEDs ao retornar do submenu
                break;
            case 4: // Sair
                printf("Saindo do AgroGraf...\n");
                npClear(); npWrite(); // Apaga todos os LEDs
                // Desinicializa o Wi-Fi se estiver inicializado
                if (cyw43_is_initialized(&cyw43_state)) {
                    cyw43_arch_deinit();
                }
                return 0; // Termina o programa
            case 5: // Opção inválida (digitada ou por erro de scanf)
                printf("Opcao invalida. Por favor, digite um numero valido.\n");
                sleep_ms(1500);
                clear_screen();
                break;
            default: // Caso inesperado
                printf("Erro inesperado! Voltando ao menu.\n");
                sleep_ms(1500);
                clear_screen();
                break;
        }

        // Lógica de controle do buzzer
        bool algum_setor_quente = false;
        // Verifica se algum setor cadastrado está com temperatura alta
        for (int i = 0; i < MAX_SETORES; i++) {
            if (setor_cadastrado[i] && temperaturas_setores[i] > 100.0f) {
                algum_setor_quente = true;
                break; // Encontrou um, não precisa checar os outros
            }
        }
        // Se há setor quente e o buzzer está desligado, liga o buzzer
        if (algum_setor_quente && !buzzer_ativo) {
            beep(BUZZER_PIN, 0); // O '0' em duration_ms significa tom contínuo aqui
            buzzer_ativo = true;
        }
        // Se não há setor quente e o buzzer está ligado, desliga o buzzer
        else if (!algum_setor_quente && buzzer_ativo) {
            stop_tone(BUZZER_PIN);
            buzzer_ativo = false;
        }
        // Atualiza as cores dos LEDs, exceto se estiver no modo de cadastro (case 1)
        // pois o case 1 já gerencia seus próprios LEDs e o cursor.
        if (main_menu_choice != 1) {
            update_led_colors();
        }
    }

    // Desinicializa o Wi-Fi antes de sair (embora o loop seja infinito, é boa prática)
    if (cyw43_is_initialized(&cyw43_state)) {
         cyw43_arch_deinit();
    }
    return 0; // Teoricamente, nunca alcançado devido ao while(true)
}

/**
 * @brief Inicializa o PIO para controlar a matriz de LEDs WS2812B.
 * @param pin O pino GPIO ao qual a matriz de LEDs está conectada.
 * @details Tenta usar pio0. Se não houver State Machine (SM) disponível, tenta pio1.
 *          Carrega o programa PIO ws2818b e configura a SM.
 */
void npInit(uint pin) {
    // Tenta adicionar o programa PIO ao pio0
    uint offset = pio_add_program(np_pio, &ws2818b_program);
    // Tenta requisitar uma SM não utilizada no pio0
    sm = pio_claim_unused_sm(np_pio, true);
    // Se não conseguiu SM no pio0 (sm == -1 ou 0xffffffff)
    if (sm == (uint)-1) {
        np_pio = pio1; // Tenta usar o pio1
        offset = pio_add_program(np_pio, &ws2818b_program);
        sm = pio_claim_unused_sm(np_pio, true);
        // Se também não conseguiu SM no pio1
        if (sm == (uint)-1) {
            printf("ERRO: Nao foi possivel requisitar SM para WS2812B em pio0 ou pio1\n");
            return;
        }
    }
    // Inicializa a SM do PIO com as configurações para WS2812B
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // 800kHz para WS2812B
    npClear(); // Apaga todos os LEDs
    npWrite(); // Envia o estado para a matriz
}

/**
 * @brief Define a cor de um LED específico na matriz.
 * @param index O índice do LED (0 a LED_COUNT-1).
 * @param r Componente Vermelho da cor (0-255).
 * @param g Componente Verde da cor (0-255).
 * @param b Componente Azul da cor (0-255).
 * @details Armazena os valores R, G, B no array `leds`. A ordem G,R,B é importante para o WS2812B.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    if (index < LED_COUNT) { // Verifica se o índice é válido
        // WS2812B espera os dados na ordem G, R, B
        leds[index].R = r;
        leds[index].G = g;
        leds[index].B = b;
    }
}

/**
 * @brief Apaga todos os LEDs da matriz (define a cor deles para preto).
 */
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0); // Define R, G, B como 0
}

/**
 * @brief Envia os dados de cor do array `leds` para a matriz de LEDs física via PIO.
 */
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        // Monta a cor no formato GRB (32 bits, mas apenas 24 são usados)
        // O WS2812B recebe os bits na ordem G7..G0, R7..R0, B7..B0
        uint32_t color_grb = ((uint32_t)leds[i].G << 16) |
                               ((uint32_t)leds[i].R << 8)  |
                                (uint32_t)leds[i].B;
        // Envia a cor para a state machine do PIO.
        // '<< 8u' é necessário porque o programa PIO pode estar esperando dados de 32 bits,
        // e os 24 bits de cor são alinhados à esquerda.
        pio_sm_put_blocking(np_pio, sm, color_grb << 8u);
    }
}

/**
 * @brief Converte coordenadas (x, y) da matriz 5x5 para um índice linear (0-24).
 * @param x Coordenada X (0-4).
 * @param y Coordenada Y (0-4).
 * @return int O índice linear correspondente, considerando um layout serpentina.
 * @details O layout é serpentina:
 *          Linhas pares (0, 2, 4): da direita para a esquerda (4, 3, 2, 1, 0)
 *          Linhas ímpares (1, 3): da esquerda para a direita (0, 1, 2, 3, 4)
 *          Exemplo para uma matriz 5x5:
 *          Y=0: 4  3  2  1  0
 *          Y=1: 5  6  7  8  9
 *          Y=2: 14 13 12 11 10
 *          Y=3: 15 16 17 18 19
 *          Y=4: 24 23 22 21 20
 */
int getIndex(int x, int y) {
    // Se a linha (y) for par
    if (y % 2 == 0) {
        return y * 5 + (4 - x); // Contagem da direita para a esquerda
    }
    // Se a linha (y) for ímpar
    else {
        return y * 5 + x;       // Contagem da esquerda para a direita
    }
}

/**
 * @brief Reseta o estado da matriz auxiliar `led_states` para `false`.
 * @note Esta função manipula `led_states`, que parece ser uma forma alternativa
 *       de rastrear o cadastro, mas `setor_cadastrado` é a principal.
 */
void clear_led_states() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            led_states[i][j] = false;
        }
    }
}

/**
 * @brief Inicializa um pino GPIO como entrada com pull-up interno.
 * @param pin O número do pino GPIO a ser inicializado.
 */
void init_button(uint pin) {
    gpio_init(pin);             // Inicializa o GPIO
    gpio_set_dir(pin, GPIO_IN); // Define a direção como entrada
    gpio_pull_up(pin);          // Habilita o resistor de pull-up interno
}

/**
 * @brief Lê o estado de um pino GPIO configurado como botão.
 * @param pin O número do pino GPIO a ser lido.
 * @return bool `true` se o botão estiver pressionado (nível baixo), `false` caso contrário.
 * @details Assume que o botão conecta o pino ao GND quando pressionado (pull-up habilitado).
 */
bool read_button(uint pin) {
    return !gpio_get(pin); // Retorna true se o pino estiver em nível baixo (botão pressionado)
}

/**
 * @brief Exibe o menu principal do AgroGraf no terminal serial.
 * @details Limpa a tela e mostra as opções disponíveis. Também exibe o IP se conectado ao Wi-Fi.
 */
void show_menu() {
    clear_screen(); // Limpa a tela do terminal
    printf("\n--- Menu Principal AgroGraf ---\n");
    printf("1: Cadastrar/Descadastrar Setores (Joystick)\n");
    printf("2: Limpar Sistema (LEDs e Temperaturas)\n");
    printf("3: Menu Setores\n");
    printf("4: Sair\n");
    // Exibe informações de status do Wi-Fi e IP
    if (netif_default && netif_is_up(netif_default) && !ip4_addr_isany(netif_ip4_addr(netif_default))) {
        // Se a interface de rede padrão está ativa e tem um IP válido
        printf("IP: %s (Acesse via navegador)\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    } else if (cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) < 0 && cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_DOWN) {
        // Se houve um erro no link Wi-Fi (diferente de simplesmente não conectado)
        printf("WiFi: Falha no link ou nao conectado (%d).\n", cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA));
    } else {
        // Outros casos (conectando, sem IP ainda, ou link down)
        printf("WiFi: Conectando ou sem IP...\n");
    }
    printf("Escolha uma opcao (1-4): ");
}

/**
 * @brief Exibe o submenu de gerenciamento de setores do AgroGraf.
 * @details Permite listar setores, mudar temperatura, acionar equipamentos e voltar ao menu principal.
 */
void show_setores_menu() {
    int choice_setor_local; // Variável local para a escolha no submenu de setores
    while (true) { // Loop do submenu de setores
        cyw43_arch_poll(); // Polling do Wi-Fi
        clear_screen();    // Limpa a tela do terminal
        printf("\n--- Menu Setores AgroGraf ---\n");
        printf("1: Listar setores cadastrados\n");
        printf("2: Mudar temperatura do setor cadastrado\n");
        printf("3: Acionar equipamentos contra incendio\n");
        printf("4: Voltar ao Menu Principal\n");
        printf("Escolha uma opcao (1-4): ");

        // Lê a escolha do usuário no submenu
        if (scanf("%d", &choice_setor_local) != 1) {
            choice_setor_local = 5; // Opção inválida padrão
            // Limpa o buffer de entrada
            while (getchar() != '\n' && getchar() != EOF);
        }

        // Processa a escolha do submenu
        switch (choice_setor_local) {
            case 1: listar_setores(); break;
            case 2: mudar_temperatura_setor(); break;
            case 3: acionar_equipamentos_contra_incendio(); break;
            case 4: return; // Volta para o menu principal
            case 5: // Opção inválida (digitada ou por erro de scanf)
                printf("Opcao invalida. Por favor, digite um numero entre 1 e 4.\n");
                sleep_ms(1500);
                break;
            default: // Caso inesperado
                printf("Erro inesperado! Voltando ao menu Setores.\n");
                sleep_ms(1500);
                break;
        }
    }
}

/**
 * @brief Lê a temperatura do sensor interno do RP2040.
 * @param unit Unidade desejada para a temperatura ('C' para Celsius, 'F' para Fahrenheit).
 * @return float A temperatura lida na unidade especificada.
 * @details O sensor de temperatura interno é conectado ao ADC canal 4.
 *          A fórmula de conversão é baseada na documentação do RP2040.
 */
float read_onboard_temperature(const char unit) {
    // Fator de conversão de leitura ADC crua para tensão
    const float conversionFactor = 3.3f / (1 << 12); // (3.3V / 4096 níveis ADC de 12 bits)
    adc_select_input(ADC_TEMP_PIN); // Seleciona o canal ADC do sensor de temperatura (4)
    uint16_t raw = adc_read();     // Lê o valor cru do ADC
    float adc_voltage = (float)raw * conversionFactor; // Converte para tensão
    // Fórmula para converter tensão em temperatura Celsius (do datasheet do RP2040)
    // Temp (°C) = 27 - (ADC_voltage - 0.706) / 0.001721
    float tempC = 27.0f - (adc_voltage - 0.706f) / 0.001721f;

    if (unit == 'C') return tempC; // Retorna em Celsius
    if (unit == 'F') return tempC * 9 / 5 + 32; // Converte para Fahrenheit e retorna
    return tempC; // Padrão é Celsius
}

/**
 * @brief Lista todos os setores cadastrados com seus nomes, índices e temperaturas.
 * @details Exibe um alerta "[ALERTA]" se a temperatura do setor for > 100°C.
 *          Aguarda o usuário pressionar Enter para continuar.
 */
void listar_setores() {
    clear_screen(); // Limpa a tela do terminal
    printf("\n--- Listando Setores Cadastrados (AgroGraf) ---\n");
    int count = 0; // Contador de setores cadastrados
    // Itera pela matriz de LEDs (representando setores)
    for (int y_loop = 0; y_loop < 5; y_loop++) {
        for (int x_loop = 0; x_loop < 5; x_loop++) {
            int index = getIndex(x_loop, y_loop); // Obtém o índice linear do setor
            if (setor_cadastrado[index]) { // Se o setor estiver cadastrado
                printf("%s (Indice %d): Temp: %.2f C %s\n",
                    nomes_setores[index],         // Nome do setor
                    index + 1,                    // Índice (1-25 para o usuário)
                    temperaturas_setores[index],  // Temperatura atual
                    (temperaturas_setores[index] > 100.0f ? "[ALERTA]" : "")); // Alerta se > 100°C
                count++;
            }
        }
    }
    if (count == 0) printf("\nNao existem setores cadastrados.\n");
    printf("\nPressione Enter para continuar...\n");
    int c_in;
    // Consome o newline pendente do scanf anterior, se houver
    while ((c_in = getchar()) != '\n' && c_in != EOF);
    // Aguarda um novo Enter do usuário
    if (c_in != EOF && c_in != '\r') getchar(); // Consome o Enter pressionado
}


/**
 * @brief Permite ao usuário alterar a temperatura de um setor cadastrado.
 * @details Lista os setores cadastrados, solicita o índice do setor e a nova temperatura.
 */
void mudar_temperatura_setor() {
    clear_screen(); // Limpa a tela do terminal
    int setor_idx_escolhido; // Índice do setor escolhido pelo usuário
    float nova_temperatura;  // Nova temperatura a ser definida

    printf("\n--- Mudar Temperatura do Setor (AgroGraf) ---\nSetores Cadastrados:\n");
    int count = 0; // Contador de setores cadastrados
    // Lista os setores cadastrados para o usuário escolher
    for (int y_loop = 0; y_loop < 5; y_loop++) {
        for (int x_loop = 0; x_loop < 5; x_loop++) {
            int index = getIndex(x_loop, y_loop);
            if (setor_cadastrado[index]) {
                printf("%s (Indice %d): Temp: %.2f C\n", nomes_setores[index], index + 1, temperaturas_setores[index]);
                count++;
            }
        }
    }
    if (count == 0) { printf("Nenhum setor cadastrado.\n"); sleep_ms(1500); return; }

    // Solicita o índice do setor ao usuário
    printf("\nDigite o INDICE do setor (1-%d): ", MAX_SETORES);
    if (scanf("%d", &setor_idx_escolhido) != 1) { // Validação da entrada
        printf("Entrada invalida.\n"); while (getchar() != '\n' && getchar() != EOF); sleep_ms(1500); return;
    }
    setor_idx_escolhido--; // Ajusta para índice baseado em 0 (0 a MAX_SETORES-1)

    // Verifica se o índice é válido e se o setor está cadastrado
    if (setor_idx_escolhido < 0 || setor_idx_escolhido >= MAX_SETORES || !setor_cadastrado[setor_idx_escolhido]) {
        printf("Indice de setor invalido ou setor nao cadastrado.\n"); sleep_ms(1500); return;
    }

    // Solicita a nova temperatura
    printf("Digite a nova temperatura para %s: ", nomes_setores[setor_idx_escolhido]);
    if (scanf("%f", &nova_temperatura) != 1) { // Validação da entrada
        printf("Entrada invalida.\n"); while (getchar() != '\n' && getchar() != EOF); sleep_ms(1500); return;
    }
    temperaturas_setores[setor_idx_escolhido] = nova_temperatura; // Atualiza a temperatura
    printf("Temperatura de %s alterada para %.2f C\n", nomes_setores[setor_idx_escolhido], nova_temperatura);
    sleep_ms(1000); // Pausa para o usuário ver a mensagem
}

/**
 * @brief Atualiza as cores dos LEDs na matriz com base no estado atual dos setores.
 * @details LEDs de setores cadastrados ficam verdes (ou vermelhos se >100°C).
 *          LEDs de setores não cadastrados ficam apagados.
 *          Não altera o LED sob o cursor se estiver no modo de cadastro (menu_choice == 1).
 */
void update_led_colors() {
    for (int y_loop = 0; y_loop < 5; y_loop++) {
        for (int x_loop = 0; x_loop < 5; x_loop++) {
            // Se estiver no modo de cadastro (main_menu_choice == 1) E este LED
            // for o que está sob o cursor, não faz nada aqui, pois o cursor azul
            // tem prioridade e é tratado no loop de cadastro.
            if (main_menu_choice == 1 && x_loop == current_x && y_loop == current_y) {
                continue;
            }
            int index = getIndex(x_loop, y_loop); // Obtém o índice linear do LED/setor
            if (setor_cadastrado[index]) { // Se o setor está cadastrado
                if (temperaturas_setores[index] > 100.0f) { // Temperatura alta (alerta)
                    npSetLED(index, red_r, red_g, red_b); // Define cor vermelha
                } else { // Temperatura normal
                    npSetLED(index, green_r, green_g, green_b); // Define cor verde
                }
            } else { // Se o setor não está cadastrado
                npSetLED(index, 0, 0, 0); // Apaga o LED
            }
        }
    }
    npWrite(); // Envia as atualizações para a matriz de LEDs física
}

/**
 * @brief Restaura a cor original do LED que estava sob o cursor azul.
 * @details Chamado ao sair do modo de cadastro de setores. A cor restaurada
 *          depende se o setor está cadastrado e qual sua temperatura.
 */
void desligarLedAzul() {
    int index_cursor = getIndex(current_x, current_y); // Índice do LED sob o cursor
    if (setor_cadastrado[index_cursor]) { // Se o setor sob o cursor está cadastrado
        if (temperaturas_setores[index_cursor] > 100.0f) { // Temperatura alta
            npSetLED(index_cursor, red_r, red_g, red_b); // Vermelho
        } else { // Temperatura normal
            npSetLED(index_cursor, green_r, green_g, green_b); // Verde
        }
    } else { // Se o setor não está cadastrado
        npSetLED(index_cursor, 0, 0, 0); // Apagado
    }
    // npWrite() é chamado pela função update_led_colors() ou explicitamente
    // após esta função ser chamada ao sair do modo de cadastro (case 1 do menu principal).
}

/**
 * @brief Simula o acionamento de equipamentos contra incêndio.
 * @details Lista os setores com temperatura > 100°C. Se o usuário confirmar,
 *          reseta a temperatura desses setores para a temperatura ambiente.
 */
void acionar_equipamentos_contra_incendio() {
    clear_screen(); // Limpa a tela do terminal
    printf("\n--- Acionar Equipamentos Contra Incendio (AgroGraf) ---\n");
    printf("\nSetores com temperaturas acima de 100 graus Celsius:\n");
    int count = 0; // Contador de setores em alerta
    // Lista os setores com temperatura crítica
    for (int y_loop = 0; y_loop < 5; y_loop++) {
        for (int x_loop = 0; x_loop < 5; x_loop++) {
            int index = getIndex(x_loop, y_loop);
            if (setor_cadastrado[index] && temperaturas_setores[index] > 100.0f) {
                printf("%s (Indice %d): Temp: %.2f C\n", nomes_setores[index], index + 1, temperaturas_setores[index]);
                count++;
            }
        }
    }
    if (count == 0) { printf("Nenhum setor com temperatura acima de 100 graus Celsius.\n"); sleep_ms(1500); return; }

    char resposta; // Resposta do usuário (s/n)
    printf("\nDeseja voltar todos os setores listados para a temperatura ambiente? (s/n): ");
    scanf(" %c", &resposta); // Lê um caractere (com espaço antes para consumir newlines pendentes)
    int c_in;
    // Limpa o restante do buffer de entrada até o newline
    while((c_in = getchar()) != '\n' && c_in != EOF);

    if (resposta == 's' || resposta == 'S') { // Se o usuário confirmar
        float temperatura_ambiente = read_onboard_temperature(TEMPERATURE_UNITS); // Lê temp. ambiente
        // Itera por todos os setores
        for (int y_loop = 0; y_loop < 5; y_loop++) {
            for (int x_loop = 0; x_loop < 5; x_loop++) {
                int index = getIndex(x_loop, y_loop);
                // Se o setor estiver cadastrado e com temperatura alta
                if (setor_cadastrado[index] && temperaturas_setores[index] > 100.0f) {
                    temperaturas_setores[index] = temperatura_ambiente; // Reseta para temp. ambiente
                    printf("Equipamentos acionados no %s - temp. controlada (%.2f C).\n", nomes_setores[index], temperatura_ambiente);
                }
            }
        }
    } else {
        printf("Nenhuma acao realizada.\n");
    }
    printf("\nPressione Enter para retornar...\n");
    // Aguarda Enter para continuar. O 'if' é para o caso de 'c_in' já ser EOF ou \r de um CR/LF.
    if (c_in != EOF && c_in != '\r') getchar();
}

/**
 * @brief Inicializa o PWM para controlar o buzzer.
 * @param pin O pino GPIO ao qual o buzzer está conectado.
 * @details Configura o pino para a função PWM, ajusta o divisor de clock e o valor de wrap
 *          para controlar a frequência do buzzer, e inicia o PWM com duty cycle 0 (desligado).
 */
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM); // Configura o pino para PWM
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obtém o slice PWM do pino

    // Configuração padrão do PWM
    pwm_config config = pwm_get_default_config();
    // Define o divisor de clock do PWM. Ex: clock_sys (125MHz) / (244+0/16) / 256 ~= 2kHz
    // Frequência = ClockSys / (clkdiv_int + clkdiv_frac/16) / wrap
    // Para um tom audível, algo em torno de 1kHz-4kHz é comum.
    // Divisor 244 e Wrap 255 resulta em aprox. 2000 Hz (125,000,000 / 244 / 256)
    pwm_config_set_clkdiv_int_frac(&config, 244, 0); // Divisor de clock inteiro
    pwm_config_set_wrap(&config, 255);               // Valor de contagem máxima (TOP) do PWM

    pwm_init(slice_num, &config, true); // Inicializa o slice PWM com a configuração e habilita
    pwm_set_gpio_level(pin, 0);         // Define o duty cycle para 0 (buzzer desligado)
}

/**
 * @brief Aciona o buzzer com um tom contínuo.
 * @param pin O pino GPIO do buzzer.
 * @param duration_ms Duração do beep em milissegundos (atualmente não implementado para parar automaticamente).
 * @details Define o duty cycle do PWM para 50% (128 de 255) para gerar som.
 *          A parada do tom deve ser feita explicitamente por `stop_tone`.
 */
void beep(uint pin, uint duration_ms) {
    // Define o nível do PWM (duty cycle). 128 é 50% de 255 (wrap).
    pwm_set_gpio_level(pin, 128);
    // Nota: duration_ms não é usado para parar o tom automaticamente nesta implementação.
    // O tom continuará até que stop_tone() seja chamado.
}

/**
 * @brief Para o som do buzzer.
 * @param pin O pino GPIO do buzzer.
 * @details Define o duty cycle do PWM para 0%, silenciando o buzzer.
 */
void stop_tone(uint pin) {
    pwm_set_gpio_level(pin, 0); // Define o duty cycle para 0 (buzzer desligado)
}
