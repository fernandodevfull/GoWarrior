// ================================================================
//  GUIA DE INTEGRACAO — Go Warrior! Modulos
//  Cole os trechos abaixo nas secoes correspondentes do main.c
// ================================================================

// ================================================================
//  1. INCLUDES  (logo apos os includes existentes)
// ================================================================
#include "save_load.h"
#include "level.h"
#include "pause.h"
#include "achievements.h"


// ================================================================
//  2. VARIAVEIS GLOBAIS  (dentro de main(), antes do game loop)
// ================================================================

// --- Pause ---
int        pausado      = 0;
PauseState pause_state;
pause_init(&pause_state);

// --- Achievements ---
AchievementSystem ach_sys;
ach_init(&ach_sys);
ach_carregar(&ach_sys);

// Contadores auxiliares para achievements
int ach_vidas_perdidas_fase = 0;   // zera quando fase muda
int ach_mortes_total        = 0;   // nunca zera (sessao inteira)
int ach_slimes_sem_dano     = 0;   // slimes mortos sem tomar dano nessa fase
int ach_coins_coletadas     = 0;
int ach_fruits_coletadas    = 0;

// --- Level Loader ---
LevelData level_data;
// Gera os .dat na primeira execucao (pode remover depois):
level_gerar_defaults();
// Carrega fase 1:
level_load("fase1.dat", &level_data);
// OBS: use level_data.slime_vel, level_data.boss_vida, etc.
//      em vez dos #defines quando quiser dados do arquivo.

// --- Save: verifica save existente e pergunta ao jogador ---
// (implemente a UI de confirmacao no menu; aqui so o load)
SaveData sd_carregado;
int tem_save = load_jogo(&sd_carregado);
// Se tem_save == 1, use sd_carregado para restaurar estado.


// ================================================================
//  3. TECLA ESC — TOGGLE PAUSE  (dentro do bloco KEY_DOWN)
// ================================================================
if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
    pausado = pause_toggle(pausado);


// ================================================================
//  4. BLOCO DE PAUSE  (no inicio do ALLEGRO_EVENT_TIMER, ANTES
//     de qualquer logica de jogo)
// ================================================================
if (pausado) {
    PauseAcao acao = pause_update(&pause_state, &event);

    if (acao == PAUSE_CONTINUAR) {
        pausado = 0;
    }
    else if (acao == PAUSE_SALVAR) {
        // Preenche SaveData e salva
        SaveData sd;
        sd.magic              = SAVE_MAGIC;
        sd.versao             = SAVE_VERSION;
        strncpy(sd.nome_jogador, heroi.nome, 49);
        sd.score              = score;
        sd.highscore          = highscore;
        sd.vidas              = vidas;
        sd.fase_atual         = fase_atual;
        sd.pos_x              = pos_x;
        sd.pos_y              = pos_y;
        sd.boss_vida          = (boss.vida) ? *boss.vida : 0;
        sd.boss_apareceu      = boss_apareceu;
        sd.boss_morto         = boss_morto;
        sd.pulo_duplo_ativo   = pulo_duplo_ativo;
        sd.altura_salto_duplo = ponteiro_altura_salto_duplo
                                ? *ponteiro_altura_salto_duplo : 0.f;
        sd.slimes_mortos      = slimes_mortos;
        sd.tesouro_coletado   = tesouro_coletado;
        sd.plat_sec_pisada    = plat_sec_pisada;
        sd.inv_quantidade     = inv_jogo.quantidade;
        sd.timestamp          = (long)time(NULL);
        for (int i = 0; i < 7; i++)  sd.coin_ativa[i]  = coin_ativa[i];
        for (int i = 0; i < 5; i++)  sd.fruit_ativa[i] = fruit_ativa[i];
        for (int i = 0; i < NUM_SLIMES; i++) sd.slime_vivo[i] = slime_vivo[i];
        for (int i = 0; i < inv_jogo.quantidade; i++)
            strncpy(sd.inv_itens[i], inv_jogo.itens[i], 19);
        save_jogo(&sd);
    }
    else if (acao == PAUSE_REINICIAR) {
        // Dispara o fluxo de game_over (reutiliza o reset existente)
        vidas = 0;
        goto game_over;
    }
    else if (acao == PAUSE_MENU) {
        // Volta para o menu sem salvar
        no_menu = 1;
        pausado  = 0;
        // Reseta estado do jogo (mesmo fluxo do game_over)
        vidas = 0;
        goto game_over;
    }

    // Desenha overlay de pause sobre o frame congelado
    pause_draw(&pause_state, font_grande, font_pequeno, TELA_W, TELA_H);
    al_flip_display();
    goto draw_frame; // Nao redesenha o jogo enquanto pausado
}


// ================================================================
//  5. ACHIEVEMENTS — verificacao continua  (no fim do bloco TIMER,
//     antes de `redraw = 1`)
// ================================================================
{
    AchievementContext ctx;
    ctx.slimes_mortos_total     = slimes_mortos;
    ctx.slimes_mortos_sem_dano  = ach_slimes_sem_dano;
    ctx.boss1_morto             = (boss_morto && fase_atual == 1);
    ctx.boss2_morto             = (boss_morto && fase_atual == 2);
    ctx.moedas_coletadas        = ach_coins_coletadas;
    ctx.num_moedas_total        = NUM_COINS;
    ctx.frutas_coletadas        = ach_fruits_coletadas;
    ctx.num_frutas_total        = NUM_FRUITS;
    ctx.plat_sec_pisada         = plat_sec_pisada;
    ctx.tesouro_coletado        = tesouro_coletado;
    ctx.palavra_secreta_usada   = bonus_palavra;
    ctx.vidas_perdidas_fase     = ach_vidas_perdidas_fase;
    ctx.score                   = score;
    ctx.mortes_total            = ach_mortes_total;
    ach_verificar(&ach_sys, &ctx);
}


// ================================================================
//  6. CONTADORES AUXILIARES — onde atualizar cada um
// ================================================================

// 6a. Quando o jogador toma dano (no bloco de dano por slime/boss):
ach_vidas_perdidas_fase++;

// 6b. Quando um slime morre (no loop de ataque):
if (ach_vidas_perdidas_fase == 0)   // so conta se nao tomou dano
    ach_slimes_sem_dano++;

// 6c. Quando o jogador morre (em game_over):
ach_mortes_total++;
ach_vidas_perdidas_fase = 0;
ach_slimes_sem_dano     = 0;
ach_coins_coletadas     = 0;
ach_fruits_coletadas    = 0;

// 6d. Quando uma moeda e coletada:
ach_coins_coletadas++;

// 6e. Quando uma fruta e coletada:
ach_fruits_coletadas++;

// 6f. Ao mudar de fase (transicao_timer == 0):
ach_vidas_perdidas_fase = 0;
ach_slimes_sem_dano     = 0;
ach_coins_coletadas     = 0;
ach_fruits_coletadas    = 0;
level_load("fase2.dat", &level_data); // carrega dados da fase 2


// ================================================================
//  7. DESENHO — notificacao de achievement (no bloco de desenho,
//     ANTES de al_flip_display())
// ================================================================
ach_draw_notif(&ach_sys, font_pequeno, TELA_W);


// ================================================================
//  8. LIMPEZA FINAL  (na secao de cleanup, antes do return 0)
// ================================================================
ach_salvar(&ach_sys);
// save_load, level, pause nao tem recursos a liberar (sem malloc)


// ================================================================
//  9. USANDO O LEVEL LOADER  (substituicao dos arrays hardcoded)
//
//  Onde o codigo atual usa plat_x[], plat_y[], etc. fixos,
//  voce pode substituir por:
// ================================================================
for (int i = 0; i < level_data.num_plats && i < NUM_PLAT; i++) {
    plat_x[i]     = level_data.plats[i].x;
    plat_y[i]     = level_data.plats[i].y;
    plat_tiles[i] = level_data.plats[i].tiles;
    plat_tipo[i]  = level_data.plats[i].tipo;
}
if (level_data.tem_plat_secreta) {
    plat_sec_x     = level_data.plat_secreta.x;
    plat_sec_y     = level_data.plat_secreta.y;
    plat_sec_tiles = level_data.plat_secreta.tiles;
}
for (int i = 0; i < level_data.num_slimes && i < NUM_SLIMES; i++) {
    slime_fx[i]   = level_data.slimes[i].x;
    slime_fy[i]   = level_data.slimes[i].y_offset > 0
                    ? level_data.slimes[i].y_offset - SLIME_H
                    : CHAO_Y - SLIME_H;
    slime_tipo[i] = level_data.slimes[i].tipo;
    slime_min[i]  = level_data.slimes[i].min;
    slime_max[i]  = level_data.slimes[i].max;
}
for (int i = 0; i < level_data.num_coins && i < NUM_COINS; i++) {
    coin_x[i] = level_data.coins[i].x;
    coin_y[i] = level_data.coins[i].y;
}
for (int i = 0; i < level_data.num_fruits && i < NUM_FRUITS; i++) {
    fruit_x[i]    = level_data.fruits[i].x;
    fruit_y[i]    = level_data.fruits[i].y;
    fruit_tipo[i] = level_data.fruits[i].tipo;
}
// Velocidade e boss do .dat:
slime_vel_atual = level_data.slime_vel;
if (boss.vida) *boss.vida = level_data.boss_vida;
boss_vel = level_data.boss_vel;


// ================================================================
//  10. COMPILACAO  (exemplo Makefile / linha de comando)
// ================================================================
/*
  gcc main.c save_load.c level.c pause.c achievements.c   \
      -o go_warrior                                         \
      -lallegro -lallegro_font -lallegro_ttf                \
      -lallegro_image -lallegro_primitives                  \
      -lallegro_native_dialog                               \
      -lm -Wall -Wextra

  No Windows com MinGW, adicione as flags especificas do Allegro
  que ja estao no seu build atual.
*/
