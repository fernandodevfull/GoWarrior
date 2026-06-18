#pragma once
#ifndef PAUSE_H
#define PAUSE_H

// ================================================================
//  MODULO: TELA DE PAUSE
//  Ativado por ESC. Bloqueia o game loop e desenha overlay.
//
//  Opcoes do menu:
//    0 - Continuar
//    1 - Reiniciar fase
//    2 - Salvar jogo
//    3 - Sair para o menu principal
//
//  Uso tipico no game loop:
//
//    // No KEY_DOWN:
//    if (keycode == ALLEGRO_KEY_ESCAPE)
//        pausado = pause_toggle(pausado);
//
//    // No TIMER (antes de qualquer logica):
//    if (pausado) {
//        PauseAcao acao = pause_update(&pause_state, &event);
//        pause_draw(&pause_state, font, font_medio);
//        al_flip_display();
//        if (acao == PAUSE_CONTINUAR) pausado = 0;
//        if (acao == PAUSE_SALVAR)    { ... salvar ... }
//        if (acao == PAUSE_REINICIAR) { ... reiniciar ... }
//        if (acao == PAUSE_MENU)      { ... ir ao menu ... }
//        continue;
//    }
// ================================================================

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

// Retorno de pause_update()
typedef enum {
    PAUSE_NENHUMA = 0,
    PAUSE_CONTINUAR = 1,
    PAUSE_REINICIAR = 2,
    PAUSE_SALVAR = 3,
    PAUSE_MENU = 4
} PauseAcao;

// Estado interno do menu de pause
typedef struct {
    int opcao_atual;    // 0..3
    int num_opcoes;     // sempre 4
    int confirmando;    // 1 = aguardando confirmacao de sair/reiniciar
} PauseState;

// Inicializa o estado (chame uma vez antes de usar)
void pause_init(PauseState* ps);

// Alterna pausado/despausado; retorna o novo valor
int pause_toggle(int pausado);

// Processa input enquanto pausado; retorna a acao escolhida (ou PAUSE_NENHUMA)
PauseAcao pause_update(PauseState* ps, const ALLEGRO_EVENT* ev);

// Desenha o overlay de pause sobre o frame atual
void pause_draw(const PauseState* ps,
    ALLEGRO_FONT* font_titulo,
    ALLEGRO_FONT* font_opcao,
    int tela_w, int tela_h);

#endif // PAUSE_H