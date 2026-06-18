#define _CRT_SECURE_NO_WARNINGS

// ================================================================
//  INCLUDES
// ================================================================
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/keyboard.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// ================================================================
//  [MOD] MODULOS EXTERNOS
// ================================================================
#include "save_load.h"
#include "level.h"
#include "pause.h"
#include "achievements.h"

// ================================================================
//  CONFIGURACOES DE TELA
// ================================================================
#define TELA_W  1366
#define TELA_H  768

// ================================================================
//  CONFIGURACOES DO JOGADOR (KNIGHT)
// ================================================================
#define KNIGHT_FW      32
#define KNIGHT_FH      32
#define KNIGHT_FRAMES   4
#define KNIGHT_SCALE    3
#define KNIGHT_W       (KNIGHT_FW * KNIGHT_SCALE)
#define KNIGHT_H       (KNIGHT_FH * KNIGHT_SCALE)

#define DIR_CIMA      0
#define DIR_DIREITA   64
#define DIR_ESQUERDA  96
#define DIR_BAIXO     160

// ================================================================
//  CONFIGURACOES DO MACHADO (AXE)
// ================================================================
#define AXE_FW      17
#define AXE_FH      18
#define AXE_FRAMES   1
#define AXE_SCALE    3
#define AXE_W       (AXE_FW * AXE_SCALE)
#define AXE_H       (AXE_FH * AXE_SCALE)

// ================================================================
//  FISICA
// ================================================================
#define GRAVIDADE    0.6f
#define FORCA_PULO  -14.0f
#define VEL_MOVE     6

// ================================================================
//  ATAQUE DO JOGADOR
// ================================================================
#define ATTACK_DURATION     12
#define ATTACK_COOLDOWN     25
#define ATAQUE_W            45
#define ATAQUE_H            45
#define AXE_ATTACK_DURATION  6
#define ATTACK_W            19
#define ATTACK_H            19

// ================================================================
//  CONFIGURACOES DAS PLATAFORMAS
// ================================================================
#define PLAT_FW     16
#define PLAT_FH     16
#define PLAT_SCALE   3
#define PLAT_TW    (PLAT_FW * PLAT_SCALE)
#define PLAT_TH    (PLAT_FH * PLAT_SCALE)
#define CHAO_TILES  29
#define CHAO_Y     (TELA_H - PLAT_TH)

#define NUM_PLAT 4
int plat_tipo[NUM_PLAT] = { 1,   2,   1,   2 };
int plat_x[NUM_PLAT] = { 100, 425, 1040, 200 };
int plat_y[NUM_PLAT] = { 560, 480,  400, 350 };
int plat_tiles[NUM_PLAT] = { 5,   6,    5,   4 };

int plat_tipo_f2[NUM_PLAT] = { 2,   1,   2,   1 };
int plat_x_f2[NUM_PLAT] = { 100, 400, 1000, 250 };
int plat_y_f2[NUM_PLAT] = { 560, 480,  400, 350 };
int plat_tiles_f2[NUM_PLAT] = { 7,   8,    7,   6 };

// ================================================================
//  CONFIGURACOES DOS SLIMES
// ================================================================
#define SLIME_FW      32
#define SLIME_FH      24
#define SLIME_FRAMES   3
#define SLIME_SCALE    3
#define SLIME_W       (SLIME_FW * SLIME_SCALE)
#define SLIME_H       (SLIME_FH * SLIME_SCALE)
#define NUM_SLIMES     4
#define SLIME_VEL      2.5f
#define SLIME_VEL_F2   3.2f

// ================================================================
//  CONFIGURACOES DO BOSS
// ================================================================
#define BOSS_SCALE        5
#define BOSS_W_PX        (SLIME_FW * BOSS_SCALE)
#define BOSS_H_PX        (SLIME_FH * BOSS_SCALE)
#define BOSS_VEL_BASE     2.5f
#define BOSS_VIDA_MAX     150
#define BOSS_VIDA_MAX_F2  250
#define BOSS_DANO_PLAYER  1

// ================================================================
//  CONFIGURACOES DE MOEDAS E FRUTAS
// ================================================================
#define COIN_FW     16
#define COIN_FH     16
#define COIN_FRAMES 12
#define COIN_SCALE   4
#define COIN_W      (COIN_FW * COIN_SCALE)
#define COIN_H      (COIN_FH * COIN_SCALE)
#define NUM_COINS    7

#define FRUIT_FW     16
#define FRUIT_FH     16
#define FRUIT_FRAMES  3
#define FRUIT_SCALE   3
#define FRUIT_W      (FRUIT_FW * FRUIT_SCALE)
#define FRUIT_H      (FRUIT_FH * FRUIT_SCALE)
#define NUM_FRUITS    5

// ================================================================
//  CONFIGURACOES DE PARTICULAS
// ================================================================
#define MAX_PARTICLES 80

// ================================================================
//  CONFIGURACOES DE HIGH SCORE
// ================================================================
#define HIGHSCORE_FILE "highscore.txt"

// ================================================================
//  CONFIGURACOES DO INVENTARIO / ESTRUTURAS DE DADOS
// ================================================================
#define MAX_INVENTARIO  5
#define NOME_ITEM_MAX  20
#define MAPA_LINHAS    5
#define MAPA_COLUNAS  10

// ================================================================
//  STRUCTS
// ================================================================
typedef struct { float x, y, w, h; } Rect;
typedef struct { float x, y, vx, vy, life, r, g, b; } Particle;
typedef struct { char  itens[MAX_INVENTARIO][NOME_ITEM_MAX]; int quantidade; } Inventario;
typedef struct { int* itens; int capacidade; int quantidade; } Mochila;
typedef struct { void* duracao; int ativo; char nome[50]; } PowerUp;
typedef struct { int* vida; float velocidade; int ativo; } InimigoDinamico;
typedef struct { int   vida; int ataque; float velocidade; char nome[50]; } Inimigo;
typedef struct { char  nome[50]; int vidas, pontuacao; float pos_x, pos_y; int tem_pulo_duplo; } Jogador;
typedef struct { char  nome[50]; int valor_pontos; float peso; int posicao_x, posicao_y; } ItemColetavel;
typedef struct { char  caixa_dialogo[100]; } DialogoChefe;

// ================================================================
//  TAD 1: FILA CIRCULAR DE EVENTOS DE SCORE (FIFO)
//  Cada ponto ganho gera um evento com posicao e timer.
//  O bloco de desenho exibe "+N" flutuante ate o timer zerar.
// ================================================================
#define FILA_MAX 16

typedef struct {
    int   valor;        // pontos ganhos (+10, +30, etc.)
    float x, y;        // posicao na tela onde aparece
    int   timer;        // frames restantes ate sumir
} ScoreEvento;

typedef struct {
    ScoreEvento dados[FILA_MAX];
    int inicio;         // indice do primeiro elemento valido
    int fim;            // proximo slot livre
    int tamanho;
} FilaScore;

void fila_init(FilaScore* f) {
    f->inicio = f->fim = f->tamanho = 0;
}
int fila_cheia(const FilaScore* f) { return f->tamanho == FILA_MAX; }
int fila_vazia(const FilaScore* f) { return f->tamanho == 0; }

void fila_enfileirar(FilaScore* f, ScoreEvento ev) {
    if (fila_cheia(f)) return;              // descarta se lotada
    f->dados[f->fim] = ev;
    f->fim = (f->fim + 1) % FILA_MAX;      // avanca circular
    f->tamanho++;
}

ScoreEvento fila_frente(const FilaScore* f) {
    return f->dados[f->inicio];
}

void fila_desenfileirar(FilaScore* f) {
    if (fila_vazia(f)) return;
    f->inicio = (f->inicio + 1) % FILA_MAX;
    f->tamanho--;
}

// ================================================================
//  TAD 2: PILHA DE HISTORICO DE ITENS COLETADOS (LIFO)
//  Empilha cada item ao coletar. Ao tomar dano, desempilha
//  o ultimo item — penalidade didatica (perde os pontos dele).
// ================================================================
#define PILHA_MAX 32

typedef struct {
    char nome[NOME_ITEM_MAX];
    int  pontos;
} HistoricoItem;

typedef struct {
    HistoricoItem dados[PILHA_MAX];
    int topo;           // -1 = vazia
} PilhaHistorico;

void pilha_init(PilhaHistorico* p) { p->topo = -1; }
int pilha_vazia(const PilhaHistorico* p) { return p->topo == -1; }
int pilha_cheia(const PilhaHistorico* p) { return p->topo == PILHA_MAX - 1; }

void pilha_empilhar(PilhaHistorico* p, const char* nome, int pts) {
    if (pilha_cheia(p)) return;
    strncpy(p->dados[++p->topo].nome, nome, NOME_ITEM_MAX - 1);
    p->dados[p->topo].nome[NOME_ITEM_MAX - 1] = '\0';
    p->dados[p->topo].pontos = pts;
    printf("[PILHA] Empilhou: %s (+%d pts) | topo=%d\n",
        nome, pts, p->topo);
}

HistoricoItem pilha_desempilhar(PilhaHistorico* p) {
    HistoricoItem vazio = { "", 0 };
    if (pilha_vazia(p)) return vazio;
    HistoricoItem item = p->dados[p->topo--];
    printf("[PILHA] Desempilhou: %s (-%d pts) | topo=%d\n",
        item.nome, item.pontos, p->topo);
    return item;
}

HistoricoItem pilha_topo_ver(const PilhaHistorico* p) {
    if (pilha_vazia(p)) { HistoricoItem v = { "", 0 }; return v; }
    return p->dados[p->topo];
}

// ================================================================
//  TAD 3: LISTA ENCADEADA DE PARTICULAS (alocacao dinamica)
//  Cada particula e um no alocado com malloc no spawn e
//  liberado com free quando life <= 0. Sem limite fixo de slots.
// ================================================================
typedef struct NoParticula {
    Particle           dados;
    struct NoParticula* prox;
} NoParticula;

typedef struct {
    NoParticula* cabeca;
    int          tamanho;
} ListaParticulas;

void lista_part_init(ListaParticulas* l) {
    l->cabeca = NULL;
    l->tamanho = 0;
}

// Insere nova particula no inicio da lista — O(1)
void lista_part_add(ListaParticulas* l,
    float x, float y, float r, float g, float b) {
    NoParticula* no = malloc(sizeof(NoParticula));
    if (!no) return;
    no->dados.x = x;   no->dados.y = y;
    no->dados.vx = ((rand() % 200) - 100) / 45.0f;
    no->dados.vy = -((rand() % 80) + 20) / 35.0f;
    no->dados.life = 1.0f;
    no->dados.r = r;   no->dados.g = g;   no->dados.b = b;
    no->prox = l->cabeca;
    l->cabeca = no;
    l->tamanho++;
}

// Atualiza fisica e remove nos mortos — chame no TIMER
void lista_part_atualizar(ListaParticulas* l) {
    NoParticula** ptr = &l->cabeca;
    while (*ptr) {
        Particle* p = &(*ptr)->dados;
        p->x += p->vx;
        p->y += p->vy;
        p->vy += 0.12f;
        p->life -= 0.035f;
        if (p->life <= 0.f) {
            NoParticula* morto = *ptr;
            *ptr = morto->prox;     // remove da lista
            free(morto);            // libera memoria
            l->tamanho--;
        }
        else {
            ptr = &(*ptr)->prox;    // avanca
        }
    }
}

// Desenha todas as particulas vivas — chame no bloco de desenho
void lista_part_desenhar(const ListaParticulas* l) {
    for (NoParticula* n = l->cabeca; n; n = n->prox) {
        const Particle* p = &n->dados;
        al_draw_filled_circle(p->x, p->y, p->life * 5.f,
            al_map_rgba_f(p->r, p->g, p->b, p->life));
    }
}

// Libera todos os nos — chame no cleanup final
void lista_part_destruir(ListaParticulas* l) {
    NoParticula* n = l->cabeca;
    while (n) { NoParticula* prox = n->prox; free(n); n = prox; }
    l->cabeca = NULL; l->tamanho = 0;
}

// Wrapper: spawn N particulas na lista (substitui spawn_particles)
void spawn_lista(ListaParticulas* l, float x, float y,
    float r, float g, float b, int count) {
    for (int i = 0; i < count; i++)
        lista_part_add(l, x, y, r, g, b);
}

// ================================================================
//  COLISAO
// ================================================================
int colidem(Rect a, Rect b) {
    return a.x < b.x + b.w && a.x + a.w > b.x
        && a.y < b.y + b.h && a.y + a.h > b.y;
}

// ================================================================
//  PARTICULAS
// ================================================================
void spawn_particles(Particle* ps, float x, float y,
    float r, float g, float b, int count) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
        if (ps[i].life <= 0.0f) {
            ps[i].x = x;  ps[i].y = y;
            ps[i].vx = ((rand() % 200) - 100) / 45.0f;
            ps[i].vy = -((rand() % 80) + 20) / 35.0f;
            ps[i].life = 1.0f;
            ps[i].r = r;  ps[i].g = g;  ps[i].b = b;
            spawned++;
        }
    }
}

// ================================================================
//  HIGH SCORE
// ================================================================
int carregar_highscore(void) {
    FILE* f = fopen(HIGHSCORE_FILE, "r");
    if (!f) return 0;
    int val = 0;
    fscanf(f, "%d", &val);
    fclose(f);
    return val;
}

void salvar_highscore(int val) {
    FILE* f = fopen(HIGHSCORE_FILE, "w");
    if (!f) return;
    fprintf(f, "%d", val);
    fclose(f);
}

// ================================================================
//  HUD — CORACAO
// ================================================================
void draw_coracao(float cx, float cy, float size, ALLEGRO_COLOR cor) {
    al_draw_filled_circle(cx - size * 0.28f, cy - size * 0.15f, size * 0.35f, cor);
    al_draw_filled_circle(cx + size * 0.28f, cy - size * 0.15f, size * 0.35f, cor);
    al_draw_filled_triangle(
        cx - size * 0.62f, cy + size * 0.05f,
        cx + size * 0.62f, cy + size * 0.05f,
        cx, cy + size * 0.90f, cor);
}

// ================================================================
//  PALAVRA SECRETA / DIALOGO DO CHEFE
// ================================================================
int verificar_palavra_secreta(const char* tentativa) {
    char ps[] = "LUX";
    if (strcmp(ps, tentativa) == 0) { printf("A porta magica se abre!\n"); return 1; }
    printf("Palavra incorreta.\n");
    return 0;
}

void montar_nome_heroi(char* nome_base, const char* titulo) {
    strcat(nome_base, " ");
    strcat(nome_base, titulo);
}

void ativar_dialogo_chefe(DialogoChefe* d) {
    char f[] = "Voce nunca saira daqui com vida!";
    strcpy(d->caixa_dialogo, f);
    printf("Chefe diz: %s\n", d->caixa_dialogo);
}

// ================================================================
//  INVENTARIO
// ================================================================
void inventario_adicionar(Inventario* inv, const char* item) {
    if (inv->quantidade < MAX_INVENTARIO) {
        strncpy(inv->itens[inv->quantidade], item, NOME_ITEM_MAX - 1);
        inv->itens[inv->quantidade][NOME_ITEM_MAX - 1] = '\0';
        inv->quantidade++;
        printf("Item adicionado: %s\n", item);
    }
}

int inventario_tem_item(const Inventario* inv, const char* item) {
    for (int i = 0; i < inv->quantidade; i++)
        if (strcmp(inv->itens[i], item) == 0) {
            printf("Item '%s' encontrado.\n", item);
            return 1;
        }
    printf("Item nao encontrado.\n");
    return 0;
}

// ================================================================
//  MOCHILA
// ================================================================
Mochila criar_mochila(int cap) {
    Mochila m;
    m.itens = malloc(cap * sizeof(int));
    m.capacidade = cap;
    m.quantidade = 0;
    printf("Mochila criada (cap %d).\n", cap);
    return m;
}

void mochila_adicionar(Mochila* m, int val) {
    if (m->quantidade >= m->capacidade) {
        int  nc = m->capacidade * 2;
        int* nv = realloc(m->itens, nc * sizeof(int));
        if (!nv) { printf("Erro ao expandir mochila!\n"); return; }
        m->itens = nv;
        m->capacidade = nc;
    }
    m->itens[m->quantidade++] = val;
}

void destruir_mochila(Mochila* m) {
    free(m->itens);
    m->itens = NULL;
    m->capacidade = m->quantidade = 0;
}

// ================================================================
//  POWER-UP
// ================================================================
PowerUp criar_powerup(const char* nome_pu, int dur) {
    PowerUp pu;
    pu.duracao = malloc(sizeof(int));
    if (pu.duracao) { *((int*)pu.duracao) = dur; printf("Power-up '%s' ativado!\n", nome_pu); }
    pu.ativo = 1;
    strncpy(pu.nome, nome_pu, 49);
    pu.nome[49] = '\0';
    return pu;
}

void powerup_tick(PowerUp* pu, int s) {
    if (!pu->ativo || !pu->duracao) return;
    *((int*)pu->duracao) -= s;
    if (*((int*)pu->duracao) <= 0) {
        printf("Power-up '%s' expirou!\n", pu->nome);
        free(pu->duracao);
        pu->duracao = NULL;
        pu->ativo = 0;
    }
}

// ================================================================
//  INIMIGO DINAMICO
// ================================================================
InimigoDinamico criar_inimigo(int vida_inicial, float vel) {
    InimigoDinamico ini;
    ini.vida = malloc(sizeof(int));
    if (ini.vida) { *ini.vida = vida_inicial; printf("Inimigo criado! Vida: %d\n", *ini.vida); }
    float* pv = malloc(sizeof(float));
    if (pv) { *pv = vel; ini.velocidade = *pv; free(pv); }
    else      ini.velocidade = vel;
    ini.ativo = 1;
    return ini;
}

void destruir_inimigo(InimigoDinamico* ini) {
    free(ini->vida);
    ini->vida = NULL;
    ini->ativo = 0;
}

int* criar_status_inimigos(int quantidade) {
    int* s = calloc(quantidade, sizeof(int));
    if (!s) return NULL;
    for (int i = 0; i < quantidade; i++) s[i] = 1;
    return s;
}

// ================================================================
//  INIMIGO STRUCT / JOGADOR / ITEM COLETAVEL
// ================================================================
Inimigo criar_inimigo_struct(const char* nome, int vida, int ataque, float vel) {
    Inimigo ini;
    strncpy(ini.nome, nome, 49); ini.nome[49] = '\0';
    ini.vida = vida; ini.ataque = ataque; ini.velocidade = vel;
    printf("Inimigo: %s | Vida: %d | Atq: %d | Vel: %.1f\n",
        ini.nome, ini.vida, ini.ataque, ini.velocidade);
    return ini;
}

Inimigo* criar_onda_inimigos(int n) {
    Inimigo* o = malloc(n * sizeof(Inimigo));
    if (!o) return NULL;
    printf("Onda de %d inimigos criada.\n", n);
    return o;
}

void mover_personagem(Jogador* j, float dx, float dy) {
    j->pos_x += dx;
    j->pos_y += dy;
}

Jogador atualizar_jogador(Jogador j, int dp, int pd) {
    j.pontuacao += dp;
    j.tem_pulo_duplo = pd;
    return j;
}

ItemColetavel item_mais_valioso(ItemColetavel* inv, int n) {
    ItemColetavel m = inv[0];
    for (int i = 1; i < n; i++)
        if (inv[i].valor_pontos > m.valor_pontos) m = inv[i];
    printf("Item mais valioso: %s (%d pts)\n", m.nome, m.valor_pontos);
    return m;
}

// ================================================================
//  MAPA DE FASE
// ================================================================
int* criar_mapa_fase(void) {
    int* m = malloc(MAPA_LINHAS * MAPA_COLUNAS * sizeof(int));
    if (!m) return NULL;
    for (int i = 0; i < MAPA_LINHAS; i++)
        for (int j = 0; j < MAPA_COLUNAS; j++)
            *(m + i * MAPA_COLUNAS + j) = (i == MAPA_LINHAS - 1) ? 1 : 0;
    *(m + (MAPA_LINHAS - 2) * MAPA_COLUNAS + 4) = 2;
    *(m + (MAPA_LINHAS - 2) * MAPA_COLUNAS + 7) = 2;
    return m;
}

// ================================================================
//  MAIN
// ================================================================
int main(void) {
    srand((unsigned int)time(NULL));

    // ------------------------------------------------------------
    //  Inicializacao das estruturas de dados do trabalho
    // ------------------------------------------------------------
    int* level_map = criar_mapa_fase();
    if (!level_map) { printf("Erro ao criar mapa!\n"); return 1; }

    int* status_inimigos = criar_status_inimigos(NUM_SLIMES);

    Inimigo cogumelo_zumbi = criar_inimigo_struct("Cogumelo Zumbi", 20, 5, 0.8f);
    Inimigo cavaleiro_esqueleto = criar_inimigo_struct("Cavaleiro Esqueleto", 80, 15, 1.2f);
    (void)cavaleiro_esqueleto;

    Mochila mochila = criar_mochila(3);
    mochila_adicionar(&mochila, 10); mochila_adicionar(&mochila, 25);
    mochila_adicionar(&mochila, 7);  mochila_adicionar(&mochila, 42);

    PowerUp pulo_duplo_pu = criar_powerup("Super Salto", 10);

    Jogador heroi;
    heroi.vidas = 3; heroi.pontuacao = 0;
    heroi.pos_x = 50.0f; heroi.pos_y = 0.0f; heroi.tem_pulo_duplo = 0;
    heroi = atualizar_jogador(heroi, 10, 1);
    Jogador* ptr_heroi = &heroi;
    mover_personagem(ptr_heroi, 10.0f, 5.0f);

    ItemColetavel moeda_bronze = { "Moeda Bronze", 10, 0.5f, 0, 0 };
    ItemColetavel moeda_prata = { "Moeda Prata",  50, 0.7f, 0, 0 };
    ItemColetavel moeda_ouro = { "Moeda Ouro",  100, 1.0f, 0, 0 };
    ItemColetavel coins_arr[3] = { moeda_bronze, moeda_prata, moeda_ouro };
    item_mais_valioso(coins_arr, 3);

    int n_onda = 3;
    Inimigo* onda = criar_onda_inimigos(n_onda);
    if (onda) {
        onda[0] = criar_inimigo_struct("Slime Verde", 30, 5, 2.0f);
        onda[1] = criar_inimigo_struct("Slime Roxo", 50, 8, 1.5f);
        onda[2] = criar_inimigo_struct("Cavaleiro", 80, 15, 1.2f);
    }

    DialogoChefe dialogo;
    strcpy(dialogo.caixa_dialogo, "(Silencio)");
    ativar_dialogo_chefe(&dialogo);

    Inventario inv;
    inv.quantidade = 0;
    inventario_adicionar(&inv, "Espada");
    inventario_adicionar(&inv, "Escudo");
    inventario_adicionar(&inv, "Chave");
    inventario_tem_item(&inv, "Chave");
    inventario_tem_item(&inv, "Pocao");

    // ------------------------------------------------------------
    //  [MOD] INICIALIZACAO DOS MODULOS
    // ------------------------------------------------------------

    // --- Level Loader ---
    LevelData level_data;
    level_gerar_defaults();             // cria fase1.dat e fase2.dat se nao existirem
    level_load("fase1.dat", &level_data);

    // --- Pause ---
    int        pausado = 0;
    PauseState pause_state;
    pause_init(&pause_state);

    // --- Achievements ---
    AchievementSystem ach_sys;
    ach_init(&ach_sys);
    ach_carregar(&ach_sys);

    int ach_vidas_perdidas_fase = 0;
    int ach_mortes_total = 0;
    int ach_slimes_sem_dano = 0;
    int ach_coins_coletadas = 0;
    int ach_fruits_coletadas = 0;

    // --- Save: tenta carregar progresso anterior ---
    SaveData sd_carregado;
    int tem_save = load_jogo(&sd_carregado);
    // Aplicado mais abaixo, apos o menu, se o jogador confirmar.

    // ------------------------------------------------------------
    //  Inicializacao do Allegro
    // ------------------------------------------------------------
    al_init();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_native_dialog_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* display = al_create_display(TELA_W, TELA_H);
    al_set_window_position(display, 200, 200);
    al_set_window_title(display, "Go Warrior!");

    ALLEGRO_FONT* font = al_load_font("./font.ttf", 24, 0);
    ALLEGRO_FONT* font_grande = al_load_font("./font.ttf", 55, 0);
    ALLEGRO_FONT* font_medio = al_load_font("./font.ttf", 32, 0);
    ALLEGRO_FONT* font_pequeno = al_load_font("./font.ttf", 18, 0);

    if (!font) {
        al_show_native_message_box(display, "Erro", "Erro", "font.ttf nao encontrado!", NULL, 0);
        return -1;
    }

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // ------------------------------------------------------------
    //  Carregamento de sprites e cenarios
    // ------------------------------------------------------------
    ALLEGRO_BITMAP* spr_knight = al_load_bitmap("./knight.png");
    ALLEGRO_BITMAP* spr_slime_g = al_load_bitmap("./slime_green.png");
    ALLEGRO_BITMAP* spr_slime_p = al_load_bitmap("./slime_purple.png");
    ALLEGRO_BITMAP* spr_coin = al_load_bitmap("./coin.png");
    ALLEGRO_BITMAP* spr_fruit = al_load_bitmap("./fruit.png");
    ALLEGRO_BITMAP* spr_platform = al_load_bitmap("./platforms.png");
    ALLEGRO_BITMAP* bg = al_load_bitmap("./cenario.jpg");
    ALLEGRO_BITMAP* bg2 = al_load_bitmap("./cenario2.0.jpg");
    ALLEGRO_BITMAP* axe = al_load_bitmap("./Axe2.png");
    ALLEGRO_BITMAP* spr_fase2 = al_load_bitmap("./fase2_banner_novo.png");

    if (!spr_knight || !bg) {
        al_show_native_message_box(display, "Erro", "Erro", "Arquivo nao encontrado!", NULL, 0);
        return -1;
    }

    ALLEGRO_EVENT_QUEUE* eq = al_create_event_queue();
    al_register_event_source(eq, al_get_display_event_source(display));
    al_register_event_source(eq, al_get_timer_event_source(timer));
    al_register_event_source(eq, al_get_keyboard_event_source());
    al_start_timer(timer);

    int highscore = carregar_highscore();

    // ------------------------------------------------------------
    //  Estado do menu
    // ------------------------------------------------------------
    char nome_menu[50] = "";  int nome_len = 0;
    char palavra_input[20] = "";  int palavra_len = 0;
    int  no_menu = 1;
    int  campo_ativo = 0;
    int  bonus_palavra = 0;
    int  palavra_checada = 0;

    // [MOD] Flag de "continuar save" mostrada no menu
    int menu_opcao_save = 0;   // 0 = nenhuma; 1 = perguntando; 2 = confirmado

    // ------------------------------------------------------------
    //  Estado do jogador
    // ------------------------------------------------------------
    float knight_frame = 0.f;
    float pos_x = (TELA_W - KNIGHT_W) / 2.0f;
    float pos_y = CHAO_Y - KNIGHT_H;
    float vel_y = 0.f;
    int   no_chao = 0;
    int   current_frame_y = DIR_BAIXO;
    char  nome_heroi_hud[100] = "";

    // ------------------------------------------------------------
    //  Estado do ataque / machado
    // ------------------------------------------------------------
    int attack_active = 0;
    int attack_timer = 0;
    int attack_cooldown = 0;
    int attack_facing = 1;

    // ------------------------------------------------------------
    //  Pulo duplo
    // ------------------------------------------------------------
    float* ponteiro_altura_salto_duplo = NULL;
    int    pulo_duplo_ativo = 0;
    int    pulos_restantes = 1;

    // ------------------------------------------------------------
    //  Teclas
    // ------------------------------------------------------------
    int key_esq = 0, key_dir = 0;

    // ------------------------------------------------------------
    //  Slimes
    // ------------------------------------------------------------
    float slime_fx[NUM_SLIMES] = { 150.f,          520.f,          860.f,          1280.f };
    int   slime_fy[NUM_SLIMES] = { CHAO_Y - SLIME_H, 480 - SLIME_H,   CHAO_Y - SLIME_H, 400 - SLIME_H };
    int   slime_tipo[NUM_SLIMES] = { 0, 1, 0, 1 };
    float slime_dir[NUM_SLIMES] = { 1.f, -1.f, 1.f, -1.f };
    float slime_min[NUM_SLIMES] = { 50.f, 425.f,  700.f, 1040.f };
    float slime_max[NUM_SLIMES] = { 380.f, 713.f, 1080.f, 1280.f };
    float slime_frame = 0.f;
    float slime_vel_atual = SLIME_VEL;

    int slime_vivo[NUM_SLIMES];
    for (int i = 0; i < NUM_SLIMES; i++) slime_vivo[i] = 1;
    int slimes_mortos = 0;

    // ------------------------------------------------------------
    //  Boss
    // ------------------------------------------------------------
    InimigoDinamico boss = criar_inimigo(cogumelo_zumbi.vida, cogumelo_zumbi.velocidade);
    if (boss.vida) *boss.vida = BOSS_VIDA_MAX;

    float boss_px = 1200.0f;
    float boss_py = (float)(CHAO_Y - BOSS_H_PX);
    float boss_vel = BOSS_VEL_BASE;
    int   boss_apareceu = 0;
    int   boss_morto = 0;
    int   boss_invencivel = 0;
    int   boss_flash = 0;

    // ------------------------------------------------------------
    //  Vitoria / fase / transicao
    // ------------------------------------------------------------
    int vitoria = 0;
    int vitoria_timer = 0;
    int fase_atual = 1;
    int transicao_fase = 0;
    int transicao_timer = 0;
    int jogo_completo = 0;

    // ------------------------------------------------------------
    //  Plataforma secreta e tesouro
    // ------------------------------------------------------------
    int   tesouro_local = 0;
    int* plataforma_secreta = &tesouro_local;
    int   plat_sec_x = 620;
    int   plat_sec_y = 300;
    int   plat_sec_tiles = 3;
    int   plat_sec_pisada = 0;
    float tesouro_x = plat_sec_x + (plat_sec_tiles * PLAT_TW) / 2.0f - COIN_W / 2.0f;
    float tesouro_y = plat_sec_y - COIN_H - 10;
    int   tesouro_coletado = 0;
    int   msg_timer = 0;

    // ------------------------------------------------------------
    //  Moedas
    // ------------------------------------------------------------
    float coin_x[NUM_COINS] = { 130, 350, 540,  750, 900, 1050, 1250 };
    float coin_y[NUM_COINS] = { 470, 380, 550,  290, 550,  310,  480 };
    int   coin_ativa[NUM_COINS];
    for (int i = 0; i < NUM_COINS; i++) coin_ativa[i] = 1;
    float coin_frame = 0.f;

    // ------------------------------------------------------------
    //  Frutas
    // ------------------------------------------------------------
    float fruit_x[NUM_FRUITS] = { 250,  600,  780, 1000, 1180 };
    float fruit_y[NUM_FRUITS] = { 460,  380,  530,  460,  300 };
    int   fruit_tipo[NUM_FRUITS] = { 0,    1,    2,    3,    1 };
    int   fruit_ativa[NUM_FRUITS];
    for (int i = 0; i < NUM_FRUITS; i++) fruit_ativa[i] = 1;
    float fruit_frame = 0.f;

    // ------------------------------------------------------------
    //  [TAD 3] LISTA ENCADEADA DE PARTICULAS (substitui array fixo)
    //  Cada particula e um no alocado com malloc no spawn e
    //  liberado com free quando life <= 0.
    // ------------------------------------------------------------
    ListaParticulas lista_part;
    lista_part_init(&lista_part);

    // ------------------------------------------------------------
    //  [TAD 1] FILA CIRCULAR DE EVENTOS DE SCORE (FIFO)
    //  Cada "+10", "+30" e enfileirado e desenfileirado ao expirar.
    // ------------------------------------------------------------
    FilaScore fila_score;
    fila_init(&fila_score);

    // ------------------------------------------------------------
    //  [TAD 2] PILHA DE HISTORICO DE ITENS COLETADOS (LIFO)
    //  Ao tomar dano, desempilha o ultimo item (penalidade).
    // ------------------------------------------------------------
    PilhaHistorico pilha_hist;
    pilha_init(&pilha_hist);

    // ------------------------------------------------------------
    //  Estado geral
    // ------------------------------------------------------------
    int score = 0;
    int vidas = 3;
    int invencivel = 0;
    int redraw = 0;

    Inventario inv_jogo; inv_jogo.quantidade = 0;

    ItemColetavel* moedas_nivel = malloc(NUM_COINS * sizeof(ItemColetavel));
    if (moedas_nivel)
        for (int i = 0; i < NUM_COINS; i++) {
            snprintf(moedas_nivel[i].nome, 50, "Moeda_%d", i + 1);
            moedas_nivel[i].valor_pontos = 10; moedas_nivel[i].peso = 0.5f;
            moedas_nivel[i].posicao_x = (int)coin_x[i];
            moedas_nivel[i].posicao_y = (int)coin_y[i];
        }

    // ============================================================
    //  GAME LOOP
    // ============================================================
    while (1) {
        ALLEGRO_EVENT event;
        al_wait_for_event(eq, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) break;

        // --------------------------------------------------------
        //  MENU INICIAL
        // --------------------------------------------------------
        if (no_menu) {
            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                if (event.keyboard.keycode == ALLEGRO_KEY_TAB)
                    campo_ativo = (campo_ativo == 0) ? 1 : 0;

            if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
                // [MOD] Tecla C no menu: continuar save existente
                if (tem_save && menu_opcao_save == 1
                    && event.keyboard.keycode == ALLEGRO_KEY_C) {
                    // Aplica os dados do save
                    strncpy(heroi.nome, sd_carregado.nome_jogador, 49);
                    strncpy(nome_heroi_hud, sd_carregado.nome_jogador, 99);
                    montar_nome_heroi(nome_heroi_hud,
                        sd_carregado.pulo_duplo_ativo ? "o Iluminado" : "Guerreiro");
                    score = sd_carregado.score;
                    highscore = sd_carregado.highscore;
                    vidas = sd_carregado.vidas;
                    fase_atual = sd_carregado.fase_atual;
                    pos_x = sd_carregado.pos_x;
                    pos_y = sd_carregado.pos_y;
                    tesouro_coletado = sd_carregado.tesouro_coletado;
                    plat_sec_pisada = sd_carregado.plat_sec_pisada;
                    slimes_mortos = sd_carregado.slimes_mortos;
                    boss_apareceu = sd_carregado.boss_apareceu;
                    boss_morto = sd_carregado.boss_morto;
                    if (boss.vida) *boss.vida = sd_carregado.boss_vida;
                    pulo_duplo_ativo = sd_carregado.pulo_duplo_ativo;
                    if (pulo_duplo_ativo) {
                        ponteiro_altura_salto_duplo = malloc(sizeof(float));
                        if (ponteiro_altura_salto_duplo)
                            *ponteiro_altura_salto_duplo = sd_carregado.altura_salto_duplo;
                        pulos_restantes = 2;
                    }
                    for (int i = 0; i < NUM_COINS; i++) coin_ativa[i] = sd_carregado.coin_ativa[i];
                    for (int i = 0; i < NUM_FRUITS; i++) fruit_ativa[i] = sd_carregado.fruit_ativa[i];
                    for (int i = 0; i < NUM_SLIMES; i++) slime_vivo[i] = sd_carregado.slime_vivo[i];
                    inv_jogo.quantidade = sd_carregado.inv_quantidade;
                    for (int i = 0; i < inv_jogo.quantidade; i++)
                        strncpy(inv_jogo.itens[i], sd_carregado.inv_itens[i], NOME_ITEM_MAX - 1);

                    // Carrega fase correta do .dat
                    if (fase_atual == 2) {
                        level_load("fase2.dat", &level_data);
                        slime_vel_atual = level_data.slime_vel;
                        for (int i = 0; i < NUM_PLAT; i++) {
                            plat_x[i] = plat_x_f2[i]; plat_y[i] = plat_y_f2[i];
                            plat_tiles[i] = plat_tiles_f2[i]; plat_tipo[i] = plat_tipo_f2[i];
                        }
                    }
                    no_menu = 0;
                    printf("[SAVE] Save restaurado!\n");
                    goto save_loaded; // pula o fluxo normal de inicio
                }

                if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    if (tem_save && menu_opcao_save == 0) {
                        menu_opcao_save = 1; // mostra opcao de continuar
                    }
                    else if (campo_ativo == 1 && palavra_len > 0 && !palavra_checada) {
                        bonus_palavra = verificar_palavra_secreta(palavra_input);
                        palavra_checada = 1;
                    }
                    else if (nome_len > 0) {
                        strncpy(nome_heroi_hud, nome_menu, 99);
                        montar_nome_heroi(nome_heroi_hud, bonus_palavra ? "o Iluminado" : "Guerreiro");
                        strcpy(heroi.nome, nome_menu);
                        if (bonus_palavra) {
                            score += 50;
                            ponteiro_altura_salto_duplo = malloc(sizeof(float));
                            if (ponteiro_altura_salto_duplo) {
                                *ponteiro_altura_salto_duplo = 15.5f;
                                pulo_duplo_ativo = 1;
                                pulos_restantes = 2;
                            }
                        }
                        no_menu = 0;
                    }
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                    if (campo_ativo == 0 && nome_len > 0)
                        nome_menu[--nome_len] = '\0';
                    else if (campo_ativo == 1 && palavra_len > 0) {
                        palavra_input[--palavra_len] = '\0';
                        palavra_checada = 0;
                    }
                }
                else if (event.keyboard.unichar >= 32) {
                    if (campo_ativo == 0 && nome_len < 49) {
                        nome_menu[nome_len++] = (char)event.keyboard.unichar;
                        nome_menu[nome_len] = '\0';
                    }
                    else if (campo_ativo == 1 && palavra_len < 19) {
                        palavra_input[palavra_len++] = (char)event.keyboard.unichar;
                        palavra_input[palavra_len] = '\0';
                        palavra_checada = 0;
                    }
                }
            }

            // -- Desenho do menu --
            al_clear_to_color(al_map_rgb(10, 10, 30));
            al_draw_text(font_grande, al_map_rgb(255, 215, 0), TELA_W / 2, 100, ALLEGRO_ALIGN_CENTRE, "GO WARRIOR!");
            if (spr_knight)
                al_draw_scaled_bitmap(spr_knight, 0, DIR_BAIXO, KNIGHT_FW, KNIGHT_FH,
                    TELA_W / 2 - 64, 200, 128, 128, 0);

            ALLEGRO_COLOR cb = (campo_ativo == 0) ? al_map_rgb(255, 215, 0) : al_map_rgb(255, 255, 255);
            al_draw_text(font, al_map_rgb(255, 255, 255), TELA_W / 2, 355, ALLEGRO_ALIGN_CENTRE, "Digite seu nome:");
            al_draw_rectangle(TELA_W / 2 - 220, 385, TELA_W / 2 + 220, 425, cb, 2);
            al_draw_text(font, al_map_rgb(255, 255, 0), TELA_W / 2, 393, ALLEGRO_ALIGN_CENTRE, nome_menu);

            ALLEGRO_COLOR cp = (campo_ativo == 1) ? al_map_rgb(100, 255, 100) : al_map_rgb(180, 180, 180);
            al_draw_text(font, al_map_rgb(200, 255, 200), TELA_W / 2, 440, ALLEGRO_ALIGN_CENTRE,
                "Palavra secreta (opcional, TAB para alternar):");
            al_draw_rectangle(TELA_W / 2 - 150, 468, TELA_W / 2 + 150, 505, cp, 2);
            char mascara[20] = "";
            for (int i = 0; i < palavra_len; i++) mascara[i] = '*';
            mascara[palavra_len] = '\0';
            al_draw_text(font, al_map_rgb(100, 255, 100), TELA_W / 2, 475, ALLEGRO_ALIGN_CENTRE, mascara);

            if (palavra_checada) {
                if (bonus_palavra)
                    al_draw_text(font_pequeno, al_map_rgb(100, 255, 100), TELA_W / 2, 512,
                        ALLEGRO_ALIGN_CENTRE, "A porta magica se abre! +50 pts + Pulo Duplo!");
                else
                    al_draw_text(font_pequeno, al_map_rgb(255, 100, 100), TELA_W / 2, 512,
                        ALLEGRO_ALIGN_CENTRE, "Palavra incorreta.");
            }

            // [MOD] Botao de continuar save
            if (tem_save) {
                if (menu_opcao_save == 0) {
                    al_draw_text(font_pequeno, al_map_rgb(100, 200, 255), TELA_W / 2, 512,
                        ALLEGRO_ALIGN_CENTRE, "Save encontrado! Pressione ENTER para ver opcoes.");
                }
                else if (menu_opcao_save == 1) {
                    al_draw_filled_rectangle(TELA_W / 2 - 200, 500, TELA_W / 2 + 200, 545,
                        al_map_rgba(20, 40, 80, 200));
                    al_draw_rectangle(TELA_W / 2 - 200, 500, TELA_W / 2 + 200, 545,
                        al_map_rgb(100, 200, 255), 1);
                    char save_info[80];
                    snprintf(save_info, sizeof(save_info), "Save: %s | Fase %d | Score %d",
                        sd_carregado.nome_jogador, sd_carregado.fase_atual, sd_carregado.score);
                    al_draw_text(font_pequeno, al_map_rgb(200, 230, 255), TELA_W / 2, 505,
                        ALLEGRO_ALIGN_CENTRE, save_info);
                    al_draw_text(font_pequeno, al_map_rgb(100, 255, 100), TELA_W / 2, 525,
                        ALLEGRO_ALIGN_CENTRE, "[C] Continuar save   [ENTER] Novo jogo");
                }
            }

            al_draw_text(font, al_map_rgb(160, 160, 160), TELA_W / 2, 555, ALLEGRO_ALIGN_CENTRE,
                "WASD / Setas = mover   ESPACO = pular   Z / X = atacar   ESC = pause");
            al_draw_text(font, al_map_rgb(100, 255, 100), TELA_W / 2, 583, ALLEGRO_ALIGN_CENTRE,
                "Mate todos os slimes para enfrentar o BOSS!");
            char hs_txt[60]; snprintf(hs_txt, sizeof(hs_txt), "Recorde: %d", highscore);
            al_draw_text(font, al_map_rgb(255, 215, 0), TELA_W / 2, 618, ALLEGRO_ALIGN_CENTRE, hs_txt);
            al_draw_text(font, al_map_rgb(160, 160, 160), TELA_W / 2, 650, ALLEGRO_ALIGN_CENTRE,
                "Pressione ENTER para jogar");
            al_flip_display();
            continue;
        }

        // Ponto de entrada apos restaurar save
    save_loaded:;

        // ========================================================
        //  [MOD] BLOCO DE PAUSE
        //  ESC abre/fecha o pause.
        //  Enquanto pausado, KEY_DOWN vai para pause_update (exceto
        //  o proprio ESC que acabou de abrir — tratado no else if).
        // ========================================================
        if (event.type == ALLEGRO_EVENT_KEY_DOWN &&
            event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            pausado = pause_toggle(pausado);
            key_esq = 0; key_dir = 0;
            redraw = 1;   // forca redesenho imediato para mostrar/ocultar overlay
        }
        else if (pausado) {
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                PauseAcao acao = pause_update(&pause_state, &event);
                redraw = 1;   // qualquer tecla no pause redesenha o overlay

                if (acao == PAUSE_CONTINUAR) {
                    pausado = 0;
                    key_esq = 0; key_dir = 0;
                }
                else if (acao == PAUSE_SALVAR) {
                    SaveData sd;
                    sd.magic = SAVE_MAGIC;
                    sd.versao = SAVE_VERSION;
                    strncpy(sd.nome_jogador, heroi.nome, 49);
                    sd.score = score;
                    sd.highscore = highscore;
                    sd.vidas = vidas;
                    sd.fase_atual = fase_atual;
                    sd.pos_x = pos_x;
                    sd.pos_y = pos_y;
                    sd.boss_vida = (boss.vida) ? *boss.vida : 0;
                    sd.boss_apareceu = boss_apareceu;
                    sd.boss_morto = boss_morto;
                    sd.pulo_duplo_ativo = pulo_duplo_ativo;
                    sd.altura_salto_duplo = ponteiro_altura_salto_duplo
                        ? *ponteiro_altura_salto_duplo : 0.f;
                    sd.slimes_mortos = slimes_mortos;
                    sd.tesouro_coletado = tesouro_coletado;
                    sd.plat_sec_pisada = plat_sec_pisada;
                    sd.inv_quantidade = inv_jogo.quantidade;
                    sd.timestamp = (long)time(NULL);
                    for (int i = 0; i < NUM_COINS; i++) sd.coin_ativa[i] = coin_ativa[i];
                    for (int i = 0; i < NUM_FRUITS; i++) sd.fruit_ativa[i] = fruit_ativa[i];
                    for (int i = 0; i < NUM_SLIMES; i++) sd.slime_vivo[i] = slime_vivo[i];
                    for (int i = 0; i < inv_jogo.quantidade; i++)
                        strncpy(sd.inv_itens[i], inv_jogo.itens[i], 19);
                    save_jogo(&sd);
                    tem_save = 1;
                }
                else if (acao == PAUSE_REINICIAR) {
                    pausado = 0;
                    key_esq = 0; key_dir = 0;
                    vidas = 0;
                    goto game_over;
                }
                else if (acao == PAUSE_MENU) {
                    pausado = 0;
                    no_menu = 1;
                    key_esq = 0; key_dir = 0;
                    vidas = 0;
                    goto game_over;
                }
            }
            // Enquanto pausado: so vai ao bloco de desenho, nunca ao TIMER de jogo
            goto draw_frame;
        }
        // ========================================================
        //  fim bloco pause
        // ========================================================

        // --------------------------------------------------------
        //  LEITURA DE TECLAS  (so chega aqui se NAO estiver pausado)
        // --------------------------------------------------------
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_A) key_esq = 1;
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_D) key_dir = 1;
            if (event.keyboard.keycode == ALLEGRO_KEY_UP ||
                event.keyboard.keycode == ALLEGRO_KEY_W ||
                event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                if (no_chao) {
                    vel_y = FORCA_PULO; no_chao = 0;
                    pulos_restantes = pulo_duplo_ativo ? 1 : 0;
                    current_frame_y = DIR_CIMA;
                }
                else if (pulo_duplo_ativo && pulos_restantes > 0) {
                    float alt = ponteiro_altura_salto_duplo ? *ponteiro_altura_salto_duplo : 15.5f;
                    vel_y = -(alt * 0.9f);
                    pulos_restantes = 0;
                    current_frame_y = DIR_CIMA;
                }
            }
            if ((event.keyboard.keycode == ALLEGRO_KEY_Z || event.keyboard.keycode == ALLEGRO_KEY_X)
                && attack_cooldown == 0) {
                attack_active = 1;
                attack_timer = ATTACK_DURATION;
                attack_cooldown = ATTACK_COOLDOWN;
            }
        }
        if (event.type == ALLEGRO_EVENT_KEY_UP) {
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_A) key_esq = 0;
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT || event.keyboard.keycode == ALLEGRO_KEY_D) key_dir = 0;
        }

        // --------------------------------------------------------
        //  LOGICA DO JOGO  (tick do timer a 60 fps)
        // --------------------------------------------------------
        if (event.type == ALLEGRO_EVENT_TIMER) {
            // (pause ja tratado acima; aqui so logica de jogo normal)

            // -- Tela de transicao de fase em andamento --
            if (transicao_fase) {
                if (transicao_timer > 0) transicao_timer--;

                if (transicao_timer == 0 && fase_atual == 1 && !jogo_completo) {
                    fase_atual = 2;
                    transicao_fase = 0;
                    vitoria = 0; vitoria_timer = 0;
                    slime_vel_atual = SLIME_VEL_F2;

                    pos_x = (TELA_W - KNIGHT_W) / 2.0f;
                    pos_y = CHAO_Y - KNIGHT_H;
                    vel_y = 0.f;

                    for (int i = 0; i < NUM_PLAT; i++) {
                        plat_x[i] = plat_x_f2[i];
                        plat_y[i] = plat_y_f2[i];
                        plat_tiles[i] = plat_tiles_f2[i];
                        plat_tipo[i] = plat_tipo_f2[i];
                    }

                    float fx2[NUM_SLIMES] = { 130.f, 460.f, 1040.f, 280.f };
                    int   fy2[NUM_SLIMES] = { 560 - SLIME_H, 480 - SLIME_H, 400 - SLIME_H, 350 - SLIME_H };
                    float fd2[NUM_SLIMES] = { 1.f, -1.f, 1.f, -1.f };
                    float fmn[NUM_SLIMES] = { 100.f, 400.f, 1000.f, 250.f };
                    float fmx[NUM_SLIMES] = { 436.f, 784.f, 1336.f, 538.f };
                    for (int i = 0; i < NUM_SLIMES; i++) {
                        slime_fx[i] = fx2[i]; slime_fy[i] = fy2[i];
                        slime_dir[i] = fd2[i]; slime_min[i] = fmn[i]; slime_max[i] = fmx[i];
                        slime_vivo[i] = 1;
                    }
                    slimes_mortos = 0;

                    float cx2[NUM_COINS] = { 100, 300, 550, 680, 850, 1050, 1220 };
                    float cy2[NUM_COINS] = { 460, 350, 530, 270, 520,  290,  460 };
                    for (int i = 0; i < NUM_COINS; i++) { coin_x[i] = cx2[i]; coin_y[i] = cy2[i]; coin_ativa[i] = 1; }
                    float frx[NUM_FRUITS] = { 200, 580, 800,  980, 1150 };
                    float fry[NUM_FRUITS] = { 450, 370, 500,  440,  280 };
                    for (int i = 0; i < NUM_FRUITS; i++) { fruit_x[i] = frx[i]; fruit_y[i] = fry[i]; fruit_ativa[i] = 1; }

                    plat_sec_x = 580; plat_sec_y = 270;
                    tesouro_x = plat_sec_x + (plat_sec_tiles * PLAT_TW) / 2.0f - COIN_W / 2.0f;
                    tesouro_y = plat_sec_y - COIN_H - 10;
                    *plataforma_secreta = 0; plat_sec_pisada = 0; tesouro_coletado = 0;

                    boss_apareceu = 0; boss_morto = 0; boss_invencivel = 0; boss_flash = 0;
                    boss_px = 1200.0f; boss_py = (float)(CHAO_Y - BOSS_H_PX);
                    boss_vel = BOSS_VEL_BASE * 1.6f;
                    if (boss.vida) *boss.vida = BOSS_VIDA_MAX_F2;

                    inv_jogo.quantidade = 0;

                    // [MOD] Carrega dados da fase 2 do arquivo externo
                    level_load("fase2.dat", &level_data);
                    slime_vel_atual = level_data.slime_vel;
                    if (boss.vida) *boss.vida = level_data.boss_vida;
                    boss_vel = level_data.boss_vel;

                    // [MOD] Zera contadores de conquistas ao mudar de fase
                    ach_vidas_perdidas_fase = 0;
                    ach_slimes_sem_dano = 0;
                    ach_coins_coletadas = 0;
                    ach_fruits_coletadas = 0;

                    printf("=== FASE 2 INICIADA! Boss vida: %d | Vel boss: %.1f ===\n",
                        *boss.vida, boss_vel);
                }
                if (transicao_timer == 0 && jogo_completo) transicao_fase = 0;

                redraw = 1;
                goto draw_frame;
            }

            // -- Movimento horizontal --
            if (key_esq) {
                pos_x -= VEL_MOVE; current_frame_y = DIR_ESQUERDA; attack_facing = -1;
                if (pos_x < 0) pos_x = 0;
            }
            if (key_dir) {
                pos_x += VEL_MOVE; current_frame_y = DIR_DIREITA; attack_facing = 1;
                if (pos_x + KNIGHT_W > TELA_W) pos_x = TELA_W - KNIGHT_W;
            }
            if (current_frame_y == DIR_ESQUERDA) attack_facing = -1;
            else if (current_frame_y == DIR_DIREITA) attack_facing = 1;

            // -- Gravidade --
            vel_y += GRAVIDADE;
            pos_y += vel_y;
            no_chao = 0;
            Rect rk = { pos_x + 10, pos_y + 5, KNIGHT_W - 20, KNIGHT_H - 5 };

            // -- Colisao com o chao --
            Rect r_chao = { 0, (float)CHAO_Y, (float)TELA_W, (float)PLAT_TH };
            if (colidem(rk, r_chao) && vel_y >= 0) {
                pos_y = CHAO_Y - KNIGHT_H; vel_y = 0; no_chao = 1;
                pulos_restantes = pulo_duplo_ativo ? 1 : 0;
                if (!key_esq && !key_dir) current_frame_y = DIR_BAIXO;
            }

            // -- Colisao com plataformas normais --
            for (int i = 0; i < NUM_PLAT; i++) {
                Rect rp = { (float)plat_x[i], (float)plat_y[i],
                            (float)(plat_tiles[i] * PLAT_TW), (float)PLAT_TH };
                float margem = vel_y + 8.f; if (margem < 8.f) margem = 8.f;
                if (colidem(rk, rp) && vel_y >= 0 && (pos_y + KNIGHT_H - vel_y) <= plat_y[i] + margem) {
                    pos_y = plat_y[i] - KNIGHT_H; vel_y = 0; no_chao = 1;
                    pulos_restantes = pulo_duplo_ativo ? 1 : 0;
                    if (!key_esq && !key_dir) current_frame_y = DIR_BAIXO;
                }
            }

            // -- Colisao com plataforma secreta --
            Rect rp_sec = { (float)plat_sec_x, (float)plat_sec_y,
                            (float)(plat_sec_tiles * PLAT_TW), (float)PLAT_TH };
            float margem_sec = vel_y + 8.f; if (margem_sec < 8.f) margem_sec = 8.f;
            if (colidem(rk, rp_sec) && vel_y >= 0 && (pos_y + KNIGHT_H - vel_y) <= plat_sec_y + margem_sec) {
                pos_y = plat_sec_y - KNIGHT_H; vel_y = 0; no_chao = 1;
                pulos_restantes = pulo_duplo_ativo ? 1 : 0;
                if (!key_esq && !key_dir) current_frame_y = DIR_BAIXO;
                if (!plat_sec_pisada) {
                    *plataforma_secreta = 1; plat_sec_pisada = 1; msg_timer = 180;
                    inventario_adicionar(&inv_jogo, "Chave");
                }
            }

            // -- Coleta do tesouro secreto --
            if (tesouro_local == 1 && !tesouro_coletado) {
                Rect rt = { tesouro_x, tesouro_y, COIN_W * 2, COIN_H * 2 };
                if (colidem(rk, rt)) {
                    tesouro_coletado = 1; score += 200;
                    if (boss.vida && *boss.vida > 0) {
                        *boss.vida -= 35;
                        printf("Tesouro! Boss vida: %d\n", *boss.vida);
                    }
                    spawn_lista(&lista_part, tesouro_x + COIN_W, tesouro_y + COIN_H, 1.f, .85f, 0.f, 20);
                }
            }

            if (pos_y < 0) { pos_y = 0; vel_y = 0; }

            // -- Timers de ataque --
            if (attack_timer > 0) { attack_timer--; if (attack_timer == 0) attack_active = 0; }
            if (attack_cooldown > 0) attack_cooldown--;

            // -- Hitbox do ataque --
            Rect r_ataque;
            if (attack_facing >= 0)
                r_ataque = (Rect){ pos_x + KNIGHT_W - 5, pos_y + KNIGHT_H * 0.25f, (float)ATAQUE_W, (float)ATAQUE_H };
            else
                r_ataque = (Rect){ pos_x - ATAQUE_W + 5, pos_y + KNIGHT_H * 0.25f, (float)ATAQUE_W, (float)ATAQUE_H };

            // -- Processamento do ataque --
            if (attack_active) {
                // Acerta slimes
                for (int i = 0; i < NUM_SLIMES; i++) {
                    if (!slime_vivo[i]) continue;
                    Rect rs = { slime_fx[i], (float)slime_fy[i], (float)SLIME_W, (float)SLIME_H };
                    if (colidem(r_ataque, rs)) {
                        slime_vivo[i] = 0; slimes_mortos++; score += 30;
                        // [TAD 1] Fila: enfileira notificacao "+30"
                        fila_enfileirar(&fila_score, (ScoreEvento) {
                            30,
                                slime_fx[i] + SLIME_W * .5f, (float)slime_fy[i], 70
                        });
                        spawn_lista(&lista_part,
                            slime_fx[i] + SLIME_W * .5f, (float)slime_fy[i] + SLIME_H * .5f,
                            slime_tipo[i] == 0 ? .3f : .6f,
                            slime_tipo[i] == 0 ? .9f : .1f,
                            slime_tipo[i] == 0 ? .2f : .9f, 18);
                        printf("Slime %d derrotado! Mortos: %d/%d\n", i, slimes_mortos, NUM_SLIMES);

                        // [MOD] Contador para conquista Exterminador
                        if (ach_vidas_perdidas_fase == 0)
                            ach_slimes_sem_dano++;

                        if (slimes_mortos >= NUM_SLIMES && !boss_apareceu && !boss_morto) {
                            boss_apareceu = 1;
                            boss_px = (pos_x < TELA_W / 2) ? (float)(TELA_W - BOSS_W_PX - 60) : 60.f;
                            boss_py = (float)(CHAO_Y - BOSS_H_PX);
                            printf("\n*** O BOSS APARECEU! (Fase %d) ***\n", fase_atual);
                        }
                    }
                }

                // Acerta boss
                if (boss_apareceu && !boss_morto && boss_invencivel == 0) {
                    Rect rb = { boss_px + 10, boss_py + 10,
                                (float)(BOSS_W_PX - 20), (float)(BOSS_H_PX - 10) };
                    if (colidem(r_ataque, rb) && boss.vida) {
                        int dano = (fase_atual == 2) ? 35 : 50;
                        *boss.vida -= dano;
                        boss_invencivel = 40; boss_flash = 10;
                        spawn_lista(&lista_part,
                            boss_px + BOSS_W_PX * .5f, boss_py + BOSS_H_PX * .3f, 1.f, .2f, 0.f, 22);
                        printf("Boss atingido! Vida: %d\n", *boss.vida);

                        if (*boss.vida <= 0) {
                            boss_morto = 1;
                            score += (fase_atual == 2) ? 1000 : 500;
                            for (int k = 0; k < 3; k++)
                                spawn_lista(&lista_part,
                                    boss_px + BOSS_W_PX * .5f,
                                    boss_py + BOSS_H_PX * (.2f + k * .3f),
                                    1.f, .5f + k * .2f, 0.f, 30);

                            if (fase_atual == 1) {
                                printf("\n*** BOSS DA FASE 1 DERROTADO! Preparando Fase 2... ***\n");
                                vitoria = 1; vitoria_timer = 90;
                                transicao_fase = 1; transicao_timer = 300;
                            }
                            else {
                                printf("\n*** PARABENS! JOGO COMPLETO! ***\n");
                                vitoria = 1; vitoria_timer = 0;
                                jogo_completo = 1;
                                transicao_fase = 1; transicao_timer = 400;
                            }

                            if (score > highscore) { highscore = score; salvar_highscore(highscore); }
                        }
                    }
                }
            }

            // -- Animacoes --
            if (key_esq || key_dir || !no_chao) {
                knight_frame += .2f; if (knight_frame >= KNIGHT_FRAMES) knight_frame -= KNIGHT_FRAMES;
            }
            else knight_frame = 0;
            slime_frame += .1f;  if (slime_frame >= SLIME_FRAMES) slime_frame -= SLIME_FRAMES;
            coin_frame += .2f;  if (coin_frame >= COIN_FRAMES)  coin_frame -= COIN_FRAMES;
            fruit_frame += .08f; if (fruit_frame >= FRUIT_FRAMES) fruit_frame -= FRUIT_FRAMES;
            if (msg_timer > 0) msg_timer--;
            if (vitoria_timer > 0) vitoria_timer--;

            static int pu_tick = 0; pu_tick++;
            if (pu_tick >= 180 && pulo_duplo_pu.ativo) { pu_tick = 0; powerup_tick(&pulo_duplo_pu, 3); }

            // -- Slimes patrulham --
            for (int i = 0; i < NUM_SLIMES; i++) {
                if (!slime_vivo[i]) continue;
                slime_fx[i] += slime_dir[i] * slime_vel_atual;
                if (slime_fx[i] <= slime_min[i]) { slime_fx[i] = slime_min[i]; slime_dir[i] = 1.f; }
                if (slime_fx[i] + SLIME_W >= slime_max[i]) { slime_fx[i] = slime_max[i] - SLIME_W; slime_dir[i] = -1.f; }
            }

            // -- Boss persegue o jogador --
            if (boss_apareceu && !boss_morto) {
                if (boss_invencivel > 0) boss_invencivel--;
                if (boss_flash > 0) boss_flash--;

                float cb2 = boss_px + BOSS_W_PX * .5f, ck = pos_x + KNIGHT_W * .5f;
                if (ck < cb2) boss_px -= boss_vel; else boss_px += boss_vel;
                if (boss_px < 0)                   boss_px = 0;
                if (boss_px + BOSS_W_PX > TELA_W)  boss_px = TELA_W - BOSS_W_PX;
                boss_py = (float)(CHAO_Y - BOSS_H_PX);

                Rect rb2 = { boss_px + 15, boss_py + 15,
                             (float)(BOSS_W_PX - 30), (float)(BOSS_H_PX - 15) };
                if (colidem(rk, rb2) && invencivel == 0) {
                    vidas--; invencivel = 90;
                    ach_vidas_perdidas_fase++;
                    // [TAD 2] Pilha: desempilha ultimo item (penalidade boss)
                    if (!pilha_vazia(&pilha_hist)) {
                        HistoricoItem perdido = pilha_desempilhar(&pilha_hist);
                        score -= perdido.pontos;
                        if (score < 0) score = 0;
                        fila_enfileirar(&fila_score, (ScoreEvento) {
                            -perdido.pontos,
                                pos_x + KNIGHT_W * .5f, pos_y, 70
                        });
                    }
                    spawn_lista(&lista_part, pos_x + KNIGHT_W * .5f, pos_y + KNIGHT_H * .5f, 1.f, .1f, .1f, 12);
                    printf("Boss acertou! Vidas: %d\n", vidas);
                    if (vidas <= 0) goto game_over;
                }
            }

            // -- [TAD 3] Lista encadeada: atualiza e libera particulas mortas --
            lista_part_atualizar(&lista_part);

            // -- [TAD 1] Fila: tick do evento de score no topo --
            if (!fila_vazia(&fila_score)) {
                fila_score.dados[fila_score.inicio].timer--;
                if (fila_score.dados[fila_score.inicio].timer <= 0)
                    fila_desenfileirar(&fila_score);
            }

            // -- Coleta de moedas --
            for (int i = 0; i < NUM_COINS; i++) {
                if (!coin_ativa[i]) continue;
                Rect rc = { coin_x[i], coin_y[i], (float)COIN_W, (float)COIN_H };
                if (colidem(rk, rc)) {
                    coin_ativa[i] = 0; score += 10; mochila_adicionar(&mochila, 10);
                    ach_coins_coletadas++;
                    // [TAD 1] Fila: enfileira notificacao "+10"
                    fila_enfileirar(&fila_score, (ScoreEvento) {
                        10,
                            coin_x[i] + COIN_W * .5f, coin_y[i], 70
                    });
                    // [TAD 2] Pilha: empilha item coletado
                    pilha_empilhar(&pilha_hist, "Moeda", 10);
                    spawn_lista(&lista_part, coin_x[i] + COIN_W * .5f, coin_y[i] + COIN_H * .5f, 1.f, .85f, .1f, 10);
                }
            }

            // -- Coleta de frutas --
            for (int i = 0; i < NUM_FRUITS; i++) {
                if (!fruit_ativa[i]) continue;
                Rect rf = { fruit_x[i], fruit_y[i], (float)FRUIT_W, (float)FRUIT_H };
                if (colidem(rk, rf)) {
                    fruit_ativa[i] = 0; score += 25; mochila_adicionar(&mochila, 25);
                    ach_fruits_coletadas++;
                    // [TAD 1] Fila: enfileira notificacao "+25"
                    fila_enfileirar(&fila_score, (ScoreEvento) {
                        25,
                            fruit_x[i] + FRUIT_W * .5f, fruit_y[i], 70
                    });
                    // [TAD 2] Pilha: empilha item coletado
                    pilha_empilhar(&pilha_hist, "Fruta", 25);
                    spawn_lista(&lista_part, fruit_x[i] + FRUIT_W * .5f, fruit_y[i] + FRUIT_H * .5f, .2f, 1.f, .3f, 10);
                }
            }

            // -- Dano por toque em slimes --
            if (invencivel > 0) invencivel--;
            for (int i = 0; i < NUM_SLIMES; i++) {
                if (!slime_vivo[i]) continue;
                Rect rs = { slime_fx[i] + 5, (float)(slime_fy[i] + 5),
                            (float)(SLIME_W - 10), (float)(SLIME_H - 5) };
                if (colidem(rk, rs) && invencivel == 0) {
                    vidas--; invencivel = 90;
                    ach_vidas_perdidas_fase++;
                    // [TAD 2] Pilha: desempilha ultimo item (penalidade de dano)
                    if (!pilha_vazia(&pilha_hist)) {
                        HistoricoItem perdido = pilha_desempilhar(&pilha_hist);
                        score -= perdido.pontos;
                        if (score < 0) score = 0;
                        // [TAD 1] Fila: notifica a perda de pontos
                        fila_enfileirar(&fila_score, (ScoreEvento) {
                            -perdido.pontos,
                                pos_x + KNIGHT_W * .5f, pos_y, 70
                        });
                    }
                    spawn_lista(&lista_part, pos_x + KNIGHT_W * .5f, pos_y + KNIGHT_H * .5f, 1.f, .1f, .1f, 12);
                    if (vidas <= 0) {
                    game_over:
                        if (score > highscore) {
                            highscore = score; salvar_highscore(highscore);
                            printf("Novo recorde! %d\n", highscore);
                        }
                        // [MOD] Incrementa mortes totais antes do reset
                        ach_mortes_total++;
                        ach_vidas_perdidas_fase = 0;
                        ach_slimes_sem_dano = 0;
                        ach_coins_coletadas = 0;
                        ach_fruits_coletadas = 0;

                        // [TAD 1] Fila: esvazia ao reiniciar
                        fila_init(&fila_score);
                        // [TAD 2] Pilha: esvazia ao reiniciar
                        pilha_init(&pilha_hist);
                        // [TAD 3] Lista: destroi nos e reinicia
                        lista_part_destruir(&lista_part);
                        lista_part_init(&lista_part);

                        vidas = 3; score = 0; fase_atual = 1; slime_vel_atual = SLIME_VEL;
                        pos_x = (TELA_W - KNIGHT_W) / 2.0f; pos_y = CHAO_Y - KNIGHT_H; vel_y = 0;

                        int px1[] = { 100,425,1040,200 }; int py1[] = { 560,480,400,350 };
                        int pt1[] = { 5,  6,   5,  4 }; int pty1[] = { 1,  2,  1,  2 };
                        for (int j = 0; j < NUM_PLAT; j++) {
                            plat_x[j] = px1[j]; plat_y[j] = py1[j];
                            plat_tiles[j] = pt1[j]; plat_tipo[j] = pty1[j];
                        }

                        float sfx[] = { 150.f,520.f,860.f,1280.f };
                        int   sfy[] = { CHAO_Y - SLIME_H, 480 - SLIME_H, CHAO_Y - SLIME_H, 400 - SLIME_H };
                        float sfd[] = { 1.f,-1.f,1.f,-1.f };
                        float sfmn[] = { 50.f,425.f, 700.f,1040.f };
                        float sfmx[] = { 380.f,713.f,1080.f,1280.f };
                        for (int j = 0; j < NUM_SLIMES; j++) {
                            slime_fx[j] = sfx[j]; slime_fy[j] = sfy[j]; slime_dir[j] = sfd[j];
                            slime_min[j] = sfmn[j]; slime_max[j] = sfmx[j]; slime_vivo[j] = 1;
                        }
                        slimes_mortos = 0;

                        *plataforma_secreta = 0; plat_sec_x = 620; plat_sec_y = 300;
                        tesouro_x = plat_sec_x + (plat_sec_tiles * PLAT_TW) / 2.0f - COIN_W / 2.0f;
                        tesouro_y = plat_sec_y - COIN_H - 10;
                        plat_sec_pisada = 0; tesouro_coletado = 0;

                        float cxr[] = { 130,350,540,750,900,1050,1250 };
                        float cyr[] = { 470,380,550,290,550, 310, 480 };
                        float fxr[] = { 250,600,780,1000,1180 };
                        float fyr[] = { 460,380,530, 460, 300 };
                        for (int j = 0; j < NUM_COINS; j++) { coin_x[j] = cxr[j]; coin_y[j] = cyr[j]; coin_ativa[j] = 1; }
                        for (int j = 0; j < NUM_FRUITS; j++) { fruit_x[j] = fxr[j]; fruit_y[j] = fyr[j]; fruit_ativa[j] = 1; }

                        inv_jogo.quantidade = 0;
                        boss_apareceu = 0; boss_morto = 0; boss_invencivel = 0; boss_flash = 0;
                        boss_px = 1200.f; boss_py = (float)(CHAO_Y - BOSS_H_PX); boss_vel = BOSS_VEL_BASE;
                        if (boss.vida) *boss.vida = BOSS_VIDA_MAX;

                        vitoria = 0; vitoria_timer = 0;
                        transicao_fase = 0; transicao_timer = 0; jogo_completo = 0;
                        if (status_inimigos) for (int j = 0; j < NUM_SLIMES; j++) status_inimigos[j] = 1;

                        // [MOD] Recarrega fase 1 do arquivo externo apos reset
                        level_load("fase1.dat", &level_data);

                        printf("Game Over! Voltando para a Fase 1...\n");
                    }
                }
            }

            // ====================================================
            //  [MOD] VERIFICACAO DE CONQUISTAS  (fim do timer)
            // ====================================================
            {
                AchievementContext ctx;
                ctx.slimes_mortos_total = slimes_mortos;
                ctx.slimes_mortos_sem_dano = ach_slimes_sem_dano;
                ctx.boss1_morto = (boss_morto && fase_atual == 1);
                ctx.boss2_morto = (boss_morto && fase_atual == 2);
                ctx.moedas_coletadas = ach_coins_coletadas;
                ctx.num_moedas_total = NUM_COINS;
                ctx.frutas_coletadas = ach_fruits_coletadas;
                ctx.num_frutas_total = NUM_FRUITS;
                ctx.plat_sec_pisada = plat_sec_pisada;
                ctx.tesouro_coletado = tesouro_coletado;
                ctx.palavra_secreta_usada = bonus_palavra;
                ctx.vidas_perdidas_fase = ach_vidas_perdidas_fase;
                ctx.score = score;
                ctx.mortes_total = ach_mortes_total;
                ach_verificar(&ach_sys, &ctx);
            }

            redraw = 1;
        } // fim ALLEGRO_EVENT_TIMER

        // --------------------------------------------------------
        //  DESENHO
        // --------------------------------------------------------
    draw_frame:
        if (redraw && al_is_event_queue_empty(eq)) {
            redraw = 0;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            ALLEGRO_BITMAP* bg_atual = (fase_atual == 2 && bg2) ? bg2 : bg;
            if (bg_atual)
                al_draw_scaled_bitmap(bg_atual, 0, 0,
                    al_get_bitmap_width(bg_atual), al_get_bitmap_height(bg_atual),
                    0, 0, TELA_W, TELA_H, 0);

            // -- Plataformas --
            if (spr_platform) {
                for (int t = 0; t < CHAO_TILES; t++) {
                    int sx = (t == 0) ? 0 : (t == CHAO_TILES - 1) ? PLAT_FW * 2 : PLAT_FW;
                    al_draw_scaled_bitmap(spr_platform, sx, 0, PLAT_FW, PLAT_FH,
                        t * PLAT_TW, CHAO_Y, PLAT_TW, PLAT_TH, 0);
                }
                for (int i = 0; i < NUM_PLAT; i++) {
                    int sy = plat_tipo[i] * PLAT_FH;
                    for (int t = 0; t < plat_tiles[i]; t++) {
                        int sx = (t == 0) ? 0 : (t == plat_tiles[i] - 1) ? PLAT_FW * 2 : PLAT_FW;
                        al_draw_scaled_bitmap(spr_platform, sx, sy, PLAT_FW, PLAT_FH,
                            plat_x[i] + t * PLAT_TW, plat_y[i], PLAT_TW, PLAT_TH, 0);
                    }
                }
                for (int t = 0; t < plat_sec_tiles; t++) {
                    int sx = (t == 0) ? 0 : (t == plat_sec_tiles - 1) ? PLAT_FW * 2 : PLAT_FW;
                    if (!plat_sec_pisada && (int)(al_get_time() * 3) % 2 == 0)
                        al_draw_tinted_scaled_bitmap(spr_platform, al_map_rgb(180, 255, 180),
                            sx, 0, PLAT_FW, PLAT_FH,
                            plat_sec_x + t * PLAT_TW, plat_sec_y, PLAT_TW, PLAT_TH, 0);
                    else
                        al_draw_scaled_bitmap(spr_platform, sx, 0, PLAT_FW, PLAT_FH,
                            plat_sec_x + t * PLAT_TW, plat_sec_y, PLAT_TW, PLAT_TH, 0);
                }
            }

            // -- Moedas --
            if (spr_coin) {
                for (int i = 0; i < NUM_COINS; i++) {
                    if (!coin_ativa[i]) continue;
                    al_draw_scaled_bitmap(spr_coin, COIN_FW * (int)coin_frame, 0, COIN_FW, COIN_FH,
                        coin_x[i], coin_y[i], COIN_W, COIN_H, 0);
                }
                if (tesouro_local == 1 && !tesouro_coletado)
                    al_draw_tinted_scaled_bitmap(spr_coin, al_map_rgb(255, 215, 0),
                        COIN_FW * (int)coin_frame, 0, COIN_FW, COIN_FH,
                        tesouro_x, tesouro_y, COIN_W * 2, COIN_H * 2, 0);
            }

            // -- Frutas --
            if (spr_fruit)
                for (int i = 0; i < NUM_FRUITS; i++) {
                    if (!fruit_ativa[i]) continue;
                    al_draw_scaled_bitmap(spr_fruit,
                        FRUIT_FW * (int)fruit_frame, fruit_tipo[i] * FRUIT_FH, FRUIT_FW, FRUIT_FH,
                        fruit_x[i], fruit_y[i], FRUIT_W, FRUIT_H, 0);
                }

            // -- Boss --
            if (boss_apareceu && !boss_morto && spr_slime_p) {
                ALLEGRO_COLOR boss_tint;
                if (fase_atual == 2)
                    boss_tint = (boss_flash > 0 && boss_flash % 2 == 0)
                    ? al_map_rgb(255, 50, 200) : al_map_rgb(200, 100, 255);
                else
                    boss_tint = (boss_flash > 0 && boss_flash % 2 == 0)
                    ? al_map_rgb(255, 80, 80) : al_map_rgb(255, 180, 180);

                int boss_flip = (pos_x + KNIGHT_W * .5f < boss_px + BOSS_W_PX * .5f)
                    ? ALLEGRO_FLIP_HORIZONTAL : 0;
                al_draw_tinted_scaled_bitmap(spr_slime_p, boss_tint,
                    SLIME_FW * (int)slime_frame, 0, SLIME_FW, SLIME_FH,
                    boss_px, boss_py, BOSS_W_PX, BOSS_H_PX, boss_flip);

                if (boss.vida) {
                    int   boss_vida_max = (fase_atual == 2) ? BOSS_VIDA_MAX_F2 : BOSS_VIDA_MAX;
                    float ratio = (float)*boss.vida / (float)boss_vida_max;
                    if (ratio < 0) ratio = 0;
                    float bx = boss_px, by = boss_py - 18;
                    al_draw_filled_rectangle(bx, by, bx + BOSS_W_PX, by + 10, al_map_rgb(60, 0, 0));
                    ALLEGRO_COLOR bar_cor = (fase_atual == 2) ? al_map_rgb(180, 20, 200) : al_map_rgb(220, 40, 40);
                    al_draw_filled_rectangle(bx, by, bx + BOSS_W_PX * ratio, by + 10, bar_cor);
                    al_draw_rectangle(bx, by, bx + BOSS_W_PX, by + 10, al_map_rgb(255, 200, 0), 1);
                }
                const char* boss_label = (fase_atual == 2) ? "!! BOSS FINAL !!" : "!! BOSS !!";
                ALLEGRO_COLOR boss_label_cor = (fase_atual == 2)
                    ? al_map_rgb(200, 50, 255) : al_map_rgb(255, 80, 80);
                al_draw_text(font_pequeno, boss_label_cor,
                    boss_px + BOSS_W_PX * .5f, boss_py - 38, ALLEGRO_ALIGN_CENTRE, boss_label);
            }

            // -- Slimes --
            for (int i = 0; i < NUM_SLIMES; i++) {
                if (!slime_vivo[i]) continue;
                ALLEGRO_BITMAP* sb = (slime_tipo[i] == 0) ? spr_slime_g : spr_slime_p;
                if (!sb) continue;
                al_draw_scaled_bitmap(sb,
                    SLIME_FW * (int)slime_frame, 0, SLIME_FW, SLIME_FH,
                    slime_fx[i], (float)slime_fy[i], SLIME_W, SLIME_H,
                    (slime_dir[i] < 0) ? ALLEGRO_FLIP_HORIZONTAL : 0);
            }

            // -- Knight + Machado --
            int knight_visivel = (invencivel == 0 || (invencivel / 5) % 2 == 0);
            if (spr_knight && knight_visivel) {
                const float AXE_DRAW_SCALE = 2.0f;
                float pivot_sx = AXE_FW * 0.3f;
                float pivot_sy = AXE_FH * 0.9f;
                int   knight_flip = (current_frame_y == DIR_ESQUERDA) ? ALLEGRO_FLIP_HORIZONTAL : 0;
                int   draw_frame_y = (current_frame_y == DIR_ESQUERDA) ? DIR_DIREITA : current_frame_y;
                float hand_y = pos_y + KNIGHT_H * 0.75f;
                float hand_x, angle, sx_axe;

                if (attack_facing >= 0) {
                    hand_x = pos_x + KNIGHT_W * 0.72f; sx_axe = AXE_DRAW_SCALE;
                    angle = attack_active
                        ? (-1.2f + (1.f - (float)attack_timer / ATTACK_DURATION) * 2.4f)
                        : -0.25f;
                }
                else {
                    hand_x = pos_x + KNIGHT_W * 0.28f; sx_axe = -AXE_DRAW_SCALE;
                    angle = attack_active
                        ? (1.2f - (1.f - (float)attack_timer / ATTACK_DURATION) * 2.4f)
                        : 0.25f;
                }

                al_draw_scaled_bitmap(spr_knight,
                    KNIGHT_FW * (int)knight_frame, draw_frame_y, KNIGHT_FW, KNIGHT_FH,
                    pos_x, pos_y, KNIGHT_W, KNIGHT_H, knight_flip);
                if (axe)
                    al_draw_scaled_rotated_bitmap(axe, pivot_sx, pivot_sy,
                        hand_x, hand_y, sx_axe, AXE_DRAW_SCALE, angle, 0);
            }

            // -- [TAD 3] Lista encadeada: desenha particulas vivas --
            lista_part_desenhar(&lista_part);

            // -- [TAD 1] Fila: desenha notificacoes de score flutuantes --
            {
                FilaScore copia = fila_score;   // itera sem consumir
                int slot = copia.inicio;
                for (int f = 0; f < copia.tamanho; f++) {
                    ScoreEvento* ev = &copia.dados[slot];
                    float alpha = ev->timer / 70.f;
                    float dy = (70 - ev->timer) * 0.6f; // sobe com o tempo
                    char txt[12];
                    if (ev->valor >= 0)
                        snprintf(txt, sizeof(txt), "+%d", ev->valor);
                    else
                        snprintf(txt, sizeof(txt), "%d", ev->valor);
                    ALLEGRO_COLOR cor = (ev->valor >= 0)
                        ? al_map_rgba_f(1.f, 1.f, 0.f, alpha)   // amarelo = ganho
                        : al_map_rgba_f(1.f, 0.3f, 0.3f, alpha); // vermelho = perda
                    al_draw_text(font_pequeno, cor,
                        ev->x, ev->y - dy, ALLEGRO_ALIGN_CENTRE, txt);
                    slot = (slot + 1) % FILA_MAX;
                }
            }

            // --------------------------------------------------------
            //  HUD
            // --------------------------------------------------------
            char hud[120];
            snprintf(hud, sizeof(hud), "%s   SCORE: %d",
                nome_heroi_hud[0] ? nome_heroi_hud : heroi.nome, score);
            al_draw_text(font, al_map_rgb(0, 0, 0), 7, 7, 0, hud);
            al_draw_text(font, al_map_rgb(255, 255, 0), 5, 5, 0, hud);

            char hs_hud[40]; snprintf(hs_hud, sizeof(hs_hud), "Recorde: %d", highscore);
            al_draw_text(font, al_map_rgb(0, 0, 0), TELA_W / 2 + 1, 7, ALLEGRO_ALIGN_CENTRE, hs_hud);
            al_draw_text(font, al_map_rgb(255, 215, 0), TELA_W / 2, 5, ALLEGRO_ALIGN_CENTRE, hs_hud);

            char fase_txt[20]; snprintf(fase_txt, sizeof(fase_txt), "FASE %d", fase_atual);
            ALLEGRO_COLOR fase_cor = (fase_atual == 2) ? al_map_rgb(180, 80, 255) : al_map_rgb(100, 200, 255);
            al_draw_text(font, al_map_rgb(0, 0, 0), TELA_W - 5 + 1, 7, ALLEGRO_ALIGN_RIGHT, fase_txt);
            al_draw_text(font, fase_cor, TELA_W - 5, 5, ALLEGRO_ALIGN_RIGHT, fase_txt);

            float hx2 = TELA_W - 160.f, hy2 = 30.f, hr2 = 14.f, gap2 = 38.f;
            for (int i = 0; i < 3; i++)
                draw_coracao(hx2 + i * gap2, hy2, hr2,
                    (i < vidas) ? al_map_rgb(220, 40, 60) : al_map_rgb(60, 30, 35));

            if (pulo_duplo_ativo && ponteiro_altura_salto_duplo) {
                char pd[50]; snprintf(pd, sizeof(pd), "Pulo Duplo [%.1f]  (%d restante)",
                    *ponteiro_altura_salto_duplo, pulos_restantes);
                al_draw_text(font_pequeno, al_map_rgb(0, 0, 0), 7, 37, 0, pd);
                al_draw_text(font_pequeno, al_map_rgb(100, 220, 255), 5, 35, 0, pd);
            }

            if (inv_jogo.quantidade > 0) {
                char it[80]; snprintf(it, sizeof(it), "Inventario: ");
                for (int i = 0; i < inv_jogo.quantidade; i++) {
                    if (i > 0) strncat(it, ", ", sizeof(it) - strlen(it) - 1);
                    strncat(it, inv_jogo.itens[i], sizeof(it) - strlen(it) - 1);
                }
                al_draw_text(font_pequeno, al_map_rgb(0, 0, 0), 7, 57, 0, it);
                al_draw_text(font_pequeno, al_map_rgb(200, 200, 100), 5, 55, 0, it);
            }

            {
                int vivos = 0;
                for (int i = 0; i < NUM_SLIMES; i++) if (slime_vivo[i]) vivos++;
                if (vivos > 0 && !boss_apareceu) {
                    char sh[60]; snprintf(sh, sizeof(sh), "Slimes: %d/%d   [Z/X] = atacar", vivos, NUM_SLIMES);
                    al_draw_text(font_pequeno, al_map_rgb(0, 0, 0), 7, 77, 0, sh);
                    al_draw_text(font_pequeno, al_map_rgb(180, 255, 180), 5, 75, 0, sh);
                }
            }

            if (boss_apareceu && !boss_morto && boss.vida && *boss.vida > 0) {
                int   boss_vida_max = (fase_atual == 2) ? BOSS_VIDA_MAX_F2 : BOSS_VIDA_MAX;
                float ratio = (float)*boss.vida / (float)boss_vida_max;
                int   bw = 320, bh = 18;
                float bx = TELA_W / 2.f - bw / 2.f, by = TELA_H - 45;
                al_draw_filled_rectangle(bx, by, bx + bw, by + bh, al_map_rgb(40, 0, 0));
                ALLEGRO_COLOR bar2 = (fase_atual == 2) ? al_map_rgb(160, 20, 200) : al_map_rgb(200, 30, 30);
                al_draw_filled_rectangle(bx, by, bx + bw * ratio, by + bh, bar2);
                al_draw_rectangle(bx, by, bx + bw, by + bh, al_map_rgb(255, 200, 0), 2);
                const char* boss_lbl = (fase_atual == 2) ? "BOSS FINAL" : "BOSS";
                al_draw_text(font_pequeno, al_map_rgb(255, 255, 255),
                    TELA_W / 2.f, by - 18, ALLEGRO_ALIGN_CENTRE, boss_lbl);
            }

            if (msg_timer > 0) {
                float alpha = (msg_timer > 60) ? 1.f : msg_timer / 60.f;
                int a = (int)(alpha * 255);
                al_draw_text(font_medio, al_map_rgba(255, 215, 0, a), TELA_W / 2, TELA_H / 2 - 40,
                    ALLEGRO_ALIGN_CENTRE, "Plataforma Secreta descoberta!");
                al_draw_text(font_medio, al_map_rgba(255, 255, 255, a), TELA_W / 2, TELA_H / 2 + 10,
                    ALLEGRO_ALIGN_CENTRE, "O TESOURO apareceu! +200 pontos!");
            }

            if (vitoria && vitoria_timer > 0 && (!transicao_fase || transicao_timer > 260)) {
                float alpha = (vitoria_timer > 60) ? 1.f : vitoria_timer / 60.f;
                int a = (int)(alpha * 255);
                al_draw_filled_rectangle(0, TELA_H / 2 - 80, TELA_W, TELA_H / 2 + 80,
                    al_map_rgba(0, 0, 0, (int)(alpha * 160)));
                al_draw_text(font_grande, al_map_rgba(255, 215, 0, a), TELA_W / 2, TELA_H / 2 - 60,
                    ALLEGRO_ALIGN_CENTRE, "BOSS DERROTADO!");
                char vs[60]; snprintf(vs, sizeof(vs), "+%d pts!  Total: %d",
                    (fase_atual == 2) ? 1000 : 500, score);
                al_draw_text(font_medio, al_map_rgba(255, 255, 255, a), TELA_W / 2, TELA_H / 2 + 10,
                    ALLEGRO_ALIGN_CENTRE, vs);
            }

            if (slimes_mortos == NUM_SLIMES && !boss_apareceu && !transicao_fase) {
                static int aviso = 0; aviso++;
                if ((aviso / 15) % 2 == 0)
                    al_draw_text(font_medio, al_map_rgb(255, 50, 50),
                        TELA_W / 2, TELA_H / 2 - 20, ALLEGRO_ALIGN_CENTRE,
                        "TODOS OS SLIMES DERROTADOS!");
            }

            if (tesouro_local == 0 && !plat_sec_pisada)
                al_draw_text(font, al_map_rgba(100, 255, 100, 180),
                    plat_sec_x + (plat_sec_tiles * PLAT_TW) / 2, plat_sec_y - 35,
                    ALLEGRO_ALIGN_CENTRE, "?");

            // ====================================================
            //  [MOD] NOTIFICACAO DE CONQUISTA (sobre tudo)
            // ====================================================
            ach_draw_notif(&ach_sys, font_pequeno, TELA_W);

            // ====================================================
            //  [MOD] OVERLAY DE PAUSE (o ultimo a ser desenhado,
            //  sobre o frame congelado do jogo + HUD)
            // ====================================================
            if (pausado)
                pause_draw(&pause_state, font_grande, font_pequeno, TELA_W, TELA_H);

            // --------------------------------------------------------
            //  TELA DE TRANSICAO DE FASE
            // --------------------------------------------------------
            if (transicao_fase) {
                int total = jogo_completo ? 400 : 300;
                float fade;
                if (transicao_timer > total - 40) fade = (float)(total - transicao_timer) / 40.f;
                else if (transicao_timer < 50)     fade = transicao_timer / 50.f;
                else                               fade = 1.0f;
                if (fade > 1.f) fade = 1.f;
                if (fade < 0.f) fade = 0.f;

                al_draw_filled_rectangle(0, 0, TELA_W, TELA_H, al_map_rgba(8, 5, 20, (int)(fade * 245)));

                if (fade > 0.3f) {
                    int star_a = (int)((fade - 0.3f) / 0.7f * 180);
                    srand(42);
                    for (int s = 0; s < 60; s++) {
                        float sx2 = (float)(rand() % TELA_W);
                        float sy2 = (float)(rand() % (int)(TELA_H * 0.85f));
                        float sr = 1.0f + (rand() % 3) * 0.5f;
                        int   sp = (rand() % 3 == 0);
                        float pulse = sp ? (0.7f + 0.3f * (float)sin(al_get_time() * 3.0 + s)) : 1.0f;
                        al_draw_filled_circle(sx2, sy2, sr * pulse, al_map_rgba(255, 240, 200, star_a));
                    }
                    srand((unsigned int)time(NULL));
                }

                if (transicao_timer > 40) {
                    int ta = (int)(fade * 255);
                    if (!jogo_completo) {
                        if (spr_fase2)
                            al_draw_tinted_scaled_bitmap(spr_fase2, al_map_rgba(255, 255, 255, ta / 3),
                                0, 0, al_get_bitmap_width(spr_fase2), al_get_bitmap_height(spr_fase2),
                                0, 0, TELA_W, TELA_H, 0);

                        al_draw_filled_rectangle(TELA_W / 2 - 300, TELA_H / 2 - 165, TELA_W / 2 + 300, TELA_H / 2 - 162,
                            al_map_rgba(255, 100, 50, ta));
                        al_draw_text(font_grande, al_map_rgba(255, 100, 50, ta), TELA_W / 2, TELA_H / 2 - 155, ALLEGRO_ALIGN_CENTRE, "BOSS DERROTADO!");
                        al_draw_text(font_grande, al_map_rgba(100, 220, 255, ta), TELA_W / 2, TELA_H / 2 - 75, ALLEGRO_ALIGN_CENTRE, "NOVA FASE DESBLOQUEADA!");
                        al_draw_text(font_grande, al_map_rgba(120, 100, 0, ta / 2), TELA_W / 2 + 3, TELA_H / 2 + 18, ALLEGRO_ALIGN_CENTRE, "FASE  2");
                        al_draw_text(font_grande, al_map_rgba(255, 215, 0, ta), TELA_W / 2, TELA_H / 2 + 15, ALLEGRO_ALIGN_CENTRE, "FASE  2");
                        al_draw_text(font_medio, al_map_rgba(255, 180, 180, ta), TELA_W / 2, TELA_H / 2 + 90, ALLEGRO_ALIGN_CENTRE, "Boss mais forte  -  Slimes mais rapidos!");
                        al_draw_filled_rectangle(TELA_W / 2 - 300, TELA_H / 2 + 138, TELA_W / 2 + 300, TELA_H / 2 + 141,
                            al_map_rgba(100, 220, 255, ta));

                        float prog = 1.f - (float)transicao_timer / (float)total;
                        float bx2 = TELA_W / 2.f - 200.f, by2 = TELA_H - 60.f;
                        al_draw_filled_rectangle(bx2, by2, bx2 + 400, by2 + 8, al_map_rgba(40, 40, 80, ta));
                        al_draw_filled_rectangle(bx2, by2, bx2 + 400 * prog, by2 + 8, al_map_rgba(100, 220, 255, ta));
                        al_draw_rectangle(bx2, by2, bx2 + 400, by2 + 8, al_map_rgba(180, 180, 255, ta / 2), 1);
                        al_draw_text(font_pequeno, al_map_rgba(180, 180, 255, ta),
                            TELA_W / 2, by2 - 22, ALLEGRO_ALIGN_CENTRE, "Preparando proxima fase...");
                    }
                    else {
                        al_draw_filled_rectangle(TELA_W / 2 - 380, TELA_H / 2 - 180, TELA_W / 2 + 380, TELA_H / 2 + 160,
                            al_map_rgba(50, 35, 0, (int)(fade * 120)));
                        al_draw_rectangle(TELA_W / 2 - 380, TELA_H / 2 - 180, TELA_W / 2 + 380, TELA_H / 2 + 160,
                            al_map_rgba(255, 215, 0, ta), 2);
                        al_draw_text(font_grande, al_map_rgba(255, 215, 0, ta), TELA_W / 2, TELA_H / 2 - 155, ALLEGRO_ALIGN_CENTRE, "PARABENS, GUERREIRO!");
                        al_draw_text(font_grande, al_map_rgba(255, 100, 50, ta), TELA_W / 2, TELA_H / 2 - 75, ALLEGRO_ALIGN_CENTRE, "JOGO COMPLETO!");
                        char fs[60]; snprintf(fs, sizeof(fs), "Pontuacao Final: %d", score);
                        al_draw_text(font_medio, al_map_rgba(255, 255, 255, ta), TELA_W / 2, TELA_H / 2 + 15, ALLEGRO_ALIGN_CENTRE, fs);
                        if (score >= highscore)
                            al_draw_text(font_medio, al_map_rgba(255, 215, 0, ta), TELA_W / 2, TELA_H / 2 + 60, ALLEGRO_ALIGN_CENTRE, "NOVO RECORDE!");
                        al_draw_text(font_medio, al_map_rgba(180, 255, 180, ta), TELA_W / 2, TELA_H / 2 + 115, ALLEGRO_ALIGN_CENTRE, "Obrigado por jogar!");
                    }
                }
            }

            al_flip_display();
        }
    } // fim game loop

    // ============================================================
    //  LIMPEZA / LIBERACAO DE RECURSOS
    // ============================================================
    if (score > highscore) salvar_highscore(score);

    // [MOD] Salva conquistas ao fechar
    ach_salvar(&ach_sys);

    free(level_map); free(status_inimigos); free(onda); free(moedas_nivel);
    destruir_mochila(&mochila);
    destruir_inimigo(&boss);
    if (ponteiro_altura_salto_duplo) { free(ponteiro_altura_salto_duplo); ponteiro_altura_salto_duplo = NULL; }
    if (pulo_duplo_pu.duracao) { free(pulo_duplo_pu.duracao);       pulo_duplo_pu.duracao = NULL; }

    if (bg)           al_destroy_bitmap(bg);
    if (bg2)          al_destroy_bitmap(bg2);
    if (spr_knight)   al_destroy_bitmap(spr_knight);
    if (spr_slime_g)  al_destroy_bitmap(spr_slime_g);
    if (spr_slime_p)  al_destroy_bitmap(spr_slime_p);
    if (spr_coin)     al_destroy_bitmap(spr_coin);
    if (spr_fruit)    al_destroy_bitmap(spr_fruit);
    if (spr_platform) al_destroy_bitmap(spr_platform);
    if (axe)          al_destroy_bitmap(axe);
    if (spr_fase2)    al_destroy_bitmap(spr_fase2);

    al_destroy_font(font);
    al_destroy_font(font_grande);
    al_destroy_font(font_medio);
    al_destroy_font(font_pequeno);
    al_destroy_display(display);
    al_destroy_event_queue(eq);

    return 0;
}