#pragma once
#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

// ================================================================
//  MODULO: CONQUISTAS (ACHIEVEMENTS)
//  Verifica condicoes durante o jogo e desbloqueia conquistas.
//  Persistidas em "conquistas.dat" (binario simples).
//
//  LISTA DE CONQUISTAS:
//  ID  NOME                   DESCRICAO
//   0  Primeiro Sangue        Mate o primeiro slime
//   1  Exterminador           Mate todos os slimes sem tomar dano
//   2  Caçador de Chefes      Derrote o boss da fase 1
//   3  Lenda                  Derrote o boss final (fase 2)
//   4  Colecionador           Colete todas as moedas de uma fase
//   5  Goloso                 Colete todas as frutas de uma fase
//   6  Explorador             Descubra a plataforma secreta
//   7  Tesouro Real           Colete o tesouro secreto
//   8  Palavra Certa          Use a palavra secreta no menu
//   9  Imortal                Termine uma fase sem perder vida
//  10  Speed Run              Derrote o boss com score >= 800
//  11  Invicto                Complete o jogo inteiro sem morrer
// ================================================================

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define ACH_TOTAL          12
#define ACH_SAVE_FILE      "conquistas.dat"
#define ACH_MAGIC          0x41434800   // "ACH\0"

// Dados de uma conquista
typedef struct {
    int   id;
    char  nome[40];
    char  descricao[80];
    int   desbloqueada;     // 1 = ja conquistada
} Achievement;

// Estado do modulo (inclui notificacao em tela)
typedef struct {
    Achievement lista[ACH_TOTAL];

    // Notificacao flutuante (quando conquista e desbloqueada)
    int   notif_ativa;
    int   notif_id;
    int   notif_timer;      // frames restantes
    float notif_y;          // posicao Y animada
} AchievementSystem;

// ---- Ciclo de vida ----

    void ach_init(AchievementSystem * as);
    void ach_salvar(const AchievementSystem * as);
    void ach_carregar(AchievementSystem * as);

    // ---- Desbloqueio ----
    // Retorna 1 se acabou de desbloquear (era locked), 0 caso contrario
    int ach_desbloquear(AchievementSystem * as, int id);

    // ---- Verificacoes automaticas (chame no game loop) ----
    // Parametros refletem o estado atual do jogo
    typedef struct {
        int   slimes_mortos_total;      // acumulado na sessao
        int   slimes_mortos_sem_dano;   // slimes mortos sem tomar dano nessa fase
        int   boss1_morto;
        int   boss2_morto;
        int   moedas_coletadas;
        int   num_moedas_total;
        int   frutas_coletadas;
        int   num_frutas_total;
        int   plat_sec_pisada;
        int   tesouro_coletado;
        int   palavra_secreta_usada;
        int   vidas_perdidas_fase;      // 0 = nao tomou dano nessa fase
        int   score;
        int   mortes_total;             // 0 = nao morreu nenhuma vez
    } AchievementContext;

    void ach_verificar(AchievementSystem * as, const AchievementContext * ctx);

    // ---- Desenho ----
    // Desenha notificacao flutuante (chame todo frame, mesmo sem notificacao ativa)
    void ach_draw_notif(AchievementSystem * as, ALLEGRO_FONT * font, int tela_w);

    // Desenha tela de conquistas completa (para pause/menu)
    void ach_draw_lista(const AchievementSystem * as,
        ALLEGRO_FONT * font_titulo,
        ALLEGRO_FONT * font_item,
        int tela_w, int tela_h);
   AchievementContext;
    // Debug
    void ach_print(const AchievementSystem * as);


#endif // ACHIEVEMENTS_H