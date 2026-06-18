#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "achievements.h"
#include <stdio.h>
#include <string.h>
#include <allegro5/allegro_primitives.h>

// ----------------------------------------------------------------
// INIT - define nomes e descricoes de cada conquista
// ----------------------------------------------------------------
void ach_init(AchievementSystem* as) {
    memset(as, 0, sizeof(AchievementSystem));

    struct { char nome[40]; char desc[80]; } dados[ACH_TOTAL] = {
        {"Primeiro Sangue",     "Mate o primeiro slime"},
        {"Exterminador",        "Mate todos os slimes sem tomar dano"},
        {"Cacador de Chefes",   "Derrote o boss da fase 1"},
        {"Lenda",               "Derrote o boss final (fase 2)"},
        {"Colecionador",        "Colete todas as moedas de uma fase"},
        {"Goloso",              "Colete todas as frutas de uma fase"},
        {"Explorador",          "Descubra a plataforma secreta"},
        {"Tesouro Real",        "Colete o tesouro secreto"},
        {"Palavra Certa",       "Use a palavra secreta no menu"},
        {"Imortal",             "Termine uma fase sem perder vida"},
        {"Speed Run",           "Derrote o boss com score >= 800"},
        {"Invicto",             "Complete o jogo inteiro sem morrer"},
    };

    for (int i = 0; i < ACH_TOTAL; i++) {
        as->lista[i].id = i;
        strncpy(as->lista[i].nome, dados[i].nome, 39);
        strncpy(as->lista[i].descricao, dados[i].desc, 79);
        as->lista[i].desbloqueada = 0;
    }

    ach_carregar(as); // tenta carregar save existente
}

// ----------------------------------------------------------------
// SALVAR / CARREGAR
// ----------------------------------------------------------------
void ach_salvar(const AchievementSystem* as) {
    FILE* f = fopen(ACH_SAVE_FILE, "wb");
    if (!f) return;

    unsigned int magic = ACH_MAGIC;
    fwrite(&magic, sizeof(magic), 1, f);

    for (int i = 0; i < ACH_TOTAL; i++)
        fwrite(&as->lista[i].desbloqueada, sizeof(int), 1, f);

    fclose(f);
}

void ach_carregar(AchievementSystem* as) {
    FILE* f = fopen(ACH_SAVE_FILE, "rb");
    if (!f) return;

    unsigned int magic = 0;
    fread(&magic, sizeof(magic), 1, f);

    if (magic != ACH_MAGIC) { fclose(f); return; } // arquivo invalido

    for (int i = 0; i < ACH_TOTAL; i++)
        fread(&as->lista[i].desbloqueada, sizeof(int), 1, f);

    fclose(f);
}

// ----------------------------------------------------------------
// DESBLOQUEAR
// ----------------------------------------------------------------
int ach_desbloquear(AchievementSystem* as, int id) {
    if (id < 0 || id >= ACH_TOTAL) return 0;
    if (as->lista[id].desbloqueada) return 0; // ja tinha

    as->lista[id].desbloqueada = 1;
    ach_salvar(as);

    // Ativa notificacao
    as->notif_ativa = 1;
    as->notif_id = id;
    as->notif_timer = 180; // ~3 segundos a 60fps
    as->notif_y = -60;

    return 1; // acabou de desbloquear
}

// ----------------------------------------------------------------
// VERIFICACOES AUTOMATICAS
// ----------------------------------------------------------------
void ach_verificar(AchievementSystem* as, const AchievementContext* ctx) {
    if (ctx->slimes_mortos_total >= 1)
        ach_desbloquear(as, 0); // Primeiro Sangue

    if (ctx->slimes_mortos_sem_dano > 0 && ctx->vidas_perdidas_fase == 0)
        ach_desbloquear(as, 1); // Exterminador

    if (ctx->boss1_morto)
        ach_desbloquear(as, 2); // Cacador de Chefes

    if (ctx->boss2_morto)
        ach_desbloquear(as, 3); // Lenda

    if (ctx->num_moedas_total > 0 &&
        ctx->moedas_coletadas >= ctx->num_moedas_total)
        ach_desbloquear(as, 4); // Colecionador

    if (ctx->num_frutas_total > 0 &&
        ctx->frutas_coletadas >= ctx->num_frutas_total)
        ach_desbloquear(as, 5); // Goloso

    if (ctx->plat_sec_pisada)
        ach_desbloquear(as, 6); // Explorador

    if (ctx->tesouro_coletado)
        ach_desbloquear(as, 7); // Tesouro Real

    if (ctx->palavra_secreta_usada)
        ach_desbloquear(as, 8); // Palavra Certa

    if (ctx->vidas_perdidas_fase == 0)
        ach_desbloquear(as, 9); // Imortal

    if (ctx->score >= 800 && ctx->boss1_morto)
        ach_desbloquear(as, 10); // Speed Run

    if (ctx->boss2_morto && ctx->mortes_total == 0)
        ach_desbloquear(as, 11); // Invicto
}

// ----------------------------------------------------------------
// DESENHO - NOTIFICACAO FLUTUANTE
// ----------------------------------------------------------------

void ach_draw_notif(AchievementSystem* as, ALLEGRO_FONT* font, int tela_w) {
    if (!as->notif_ativa) return;

    // Anima entrada
    float alvo_y = 10;
    if (as->notif_y < alvo_y)
        as->notif_y += 4;

    // Caixa
    int bw = 320, bh = 50;
    int bx = tela_w - bw - 10;
    int by = (int)as->notif_y;

    al_draw_filled_rectangle(bx, by, bx + bw, by + bh,
        al_map_rgba(0, 0, 0, 200));
    al_draw_rectangle(bx, by, bx + bw, by + bh,
        al_map_rgb(255, 200, 0), 2);

    // Texto
    al_draw_text(font, al_map_rgb(255, 200, 0),
        bx + 10, by + 6, 0, "Conquista Desbloqueada!");
    al_draw_text(font, al_map_rgb(255, 255, 255),
        bx + 10, by + 26, 0,
        as->lista[as->notif_id].nome);

    // Countdown
    as->notif_timer--;
    if (as->notif_timer <= 0) {
        as->notif_ativa = 0;
        as->notif_y = -60;
    }
}

// ----------------------------------------------------------------
// DESENHO - TELA DE LISTA
// ----------------------------------------------------------------
void ach_draw_lista(const AchievementSystem* as,
    ALLEGRO_FONT* font_titulo,
    ALLEGRO_FONT* font_item,
    int tela_w, int tela_h) {

    // Fundo escuro
    al_draw_filled_rectangle(0, 0, tela_w, tela_h,
        al_map_rgba(0, 0, 0, 220));

    al_draw_text(font_titulo, al_map_rgb(255, 200, 0),
        tela_w / 2, 20, ALLEGRO_ALIGN_CENTRE, "CONQUISTAS");

    int y = 80;
    for (int i = 0; i < ACH_TOTAL; i++) {
        ALLEGRO_COLOR cor = as->lista[i].desbloqueada
            ? al_map_rgb(80, 255, 80)
            : al_map_rgb(120, 120, 120);

        char linha[128];
        snprintf(linha, sizeof(linha), "%s  %s - %s",
            as->lista[i].desbloqueada ? "[X]" : "[ ]",
            as->lista[i].nome,
            as->lista[i].descricao);

        al_draw_text(font_item, cor, tela_w / 2, y,
            ALLEGRO_ALIGN_CENTRE, linha);
        y += 40;
    }
}

// ----------------------------------------------------------------
// DEBUG
// ----------------------------------------------------------------
void ach_print(const AchievementSystem* as) {
    printf("=== CONQUISTAS ===\n");
    for (int i = 0; i < ACH_TOTAL; i++) {
        printf("[%s] %s\n",
            as->lista[i].desbloqueada ? "X" : " ",
            as->lista[i].nome);
    }
}