#define _CRT_SECURE_NO_WARNINGS
#pragma once
// pause.c
#include "pause.h"
#include <allegro5/allegro_primitives.h>
#include <string.h>

static const char* OPCOES[] = {
    "Continuar",
    "Reiniciar Fase",
    "Salvar Jogo",
    "Sair para o Menu"
};

// ----------------------------------------------------------------
// INIT
// ----------------------------------------------------------------
void pause_init(PauseState* ps) {
    ps->opcao_atual = 0;
    ps->num_opcoes = 4;
    ps->confirmando = 0;
}

// ----------------------------------------------------------------
// TOGGLE
// ----------------------------------------------------------------
int pause_toggle(int pausado) {
    return !pausado;
}

// ----------------------------------------------------------------
// UPDATE - processa input
// ----------------------------------------------------------------
PauseAcao pause_update(PauseState* ps, const ALLEGRO_EVENT* ev) {
    if (ev->type != ALLEGRO_EVENT_KEY_DOWN)
        return PAUSE_NENHUMA;

    int key = ev->keyboard.keycode;

    // --- Modo confirmacao (sair ou reiniciar) ---
    if (ps->confirmando) {
        if (key == ALLEGRO_KEY_Y || key == ALLEGRO_KEY_ENTER) {
            ps->confirmando = 0;
            if (ps->opcao_atual == 1) return PAUSE_REINICIAR;
            if (ps->opcao_atual == 3) return PAUSE_MENU;
        }
        if (key == ALLEGRO_KEY_N || key == ALLEGRO_KEY_ESCAPE) {
            ps->confirmando = 0;
        }
        return PAUSE_NENHUMA;
    }

    // --- Navegacao normal ---
    if (key == ALLEGRO_KEY_UP) {
        ps->opcao_atual--;
        if (ps->opcao_atual < 0)
            ps->opcao_atual = ps->num_opcoes - 1;
        return PAUSE_NENHUMA;
    }

    if (key == ALLEGRO_KEY_DOWN) {
        ps->opcao_atual++;
        if (ps->opcao_atual >= ps->num_opcoes)
            ps->opcao_atual = 0;
        return PAUSE_NENHUMA;
    }

    if (key == ALLEGRO_KEY_ENTER || key == ALLEGRO_KEY_SPACE) {
        switch (ps->opcao_atual) {
        case 0: return PAUSE_CONTINUAR;
        case 1: ps->confirmando = 1; return PAUSE_NENHUMA; // pede confirmacao
        case 2: return PAUSE_SALVAR;
        case 3: ps->confirmando = 1; return PAUSE_NENHUMA; // pede confirmacao
        }
    }

    if (key == ALLEGRO_KEY_ESCAPE)
        return PAUSE_CONTINUAR;

    return PAUSE_NENHUMA;
}

// ----------------------------------------------------------------
// DRAW
// ----------------------------------------------------------------
void pause_draw(const PauseState* ps,
    ALLEGRO_FONT* font_titulo,
    ALLEGRO_FONT* font_opcao,
    int tela_w, int tela_h) {

    // Overlay escuro semi-transparente
    al_draw_filled_rectangle(0, 0, tela_w, tela_h,
        al_map_rgba(0, 0, 0, 180));

    // Painel central
    int pw = 360, ph = 300;
    int px = (tela_w - pw) / 2;
    int py = (tela_h - ph) / 2;

    al_draw_filled_rectangle(px, py, px + pw, py + ph,
        al_map_rgba(10, 20, 10, 230));
    al_draw_rectangle(px, py, px + pw, py + ph,
        al_map_rgb(80, 200, 80), 2);

    // Titulo
    al_draw_text(font_titulo,
        al_map_rgb(80, 220, 80),
        tela_w / 2, py + 20,
        ALLEGRO_ALIGN_CENTRE, "PAUSADO");

    // Linha divisoria
    al_draw_line(px + 20, py + 65, px + pw - 20, py + 65,
        al_map_rgb(50, 150, 50), 1);

    // Opcoes
    int opcao_y = py + 85;
    for (int i = 0; i < ps->num_opcoes; i++) {
        int selecionado = (i == ps->opcao_atual);

        // Highlight na opcao selecionada
        if (selecionado) {
            al_draw_filled_rectangle(px + 15, opcao_y - 4,
                px + pw - 15, opcao_y + 28,
                al_map_rgba(50, 150, 50, 80));
        }

        ALLEGRO_COLOR cor = selecionado
            ? al_map_rgb(255, 230, 50)
            : al_map_rgb(200, 200, 200);

        // Seta indicadora
        if (selecionado)
            al_draw_text(font_opcao, cor, px + 25, opcao_y, 0, ">");

        al_draw_text(font_opcao, cor,
            tela_w / 2, opcao_y,
            ALLEGRO_ALIGN_CENTRE, OPCOES[i]);

        opcao_y += 46;
    }

    // Tela de confirmacao
    if (ps->confirmando) {
        const char* msg = (ps->opcao_atual == 1)
            ? "Reiniciar a fase? (S/N)"
            : "Sair para o menu? (S/N)";

        // Caixa de confirmacao
        int cx = tela_w / 2 - 200, cy = tela_h / 2 + 120;
        al_draw_filled_rectangle(cx, cy, cx + 400, cy + 50,
            al_map_rgba(0, 0, 0, 220));
        al_draw_rectangle(cx, cy, cx + 400, cy + 50,
            al_map_rgb(255, 100, 50), 2);
        al_draw_text(font_opcao,
            al_map_rgb(255, 150, 50),
            tela_w / 2, cy + 12,
            ALLEGRO_ALIGN_CENTRE, msg);
    }

    // Dica de navegacao
    al_draw_text(font_opcao,
        al_map_rgba(150, 150, 150, 180),
        tela_w / 2, py + ph - 25,
        ALLEGRO_ALIGN_CENTRE,
        "SETAS: navegar   ENTER: confirmar");
}