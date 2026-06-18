#define _CRT_SECURE_NO_WARNINGS
#pragma once
// level.c

#include "level.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ----------------------------------------------------------------
// HELPERS
// ----------------------------------------------------------------
static void trim(char* s) {
    // remove \n e \r do final
    int len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' '))
        s[--len] = '\0';
}

// ----------------------------------------------------------------
// LOAD
// ----------------------------------------------------------------
int level_load(const char* filename, LevelData* out) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("[LEVEL] Arquivo nao encontrado: %s\n", filename);
        return 0;
    }

    // Zera tudo
    memset(out, 0, sizeof(LevelData));

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        trim(linha);

        // Ignora comentarios e linhas vazias
        if (linha[0] == '#' || linha[0] == '\0') continue;

        // Corta comentario inline
        char* comentario = strchr(linha, '#');
        if (comentario) *comentario = '\0';
        trim(linha);

        char chave[64] = { 0 };
        sscanf(linha, "%63s", chave);

        // --- Chaves simples ---
        if (strcmp(chave, "NOME") == 0) {
            sscanf(linha, "%*s %63[^\n]", out->nome);
        }
        else if (strcmp(chave, "BG") == 0) {
            sscanf(linha, "%*s %63s", out->bg_file);
        }
        else if (strcmp(chave, "BG2") == 0) {
            sscanf(linha, "%*s %63s", out->bg2_file);
        }
        else if (strcmp(chave, "SLIME_VEL") == 0) {
            sscanf(linha, "%*s %f", &out->slime_vel);
        }
        else if (strcmp(chave, "BOSS_VIDA") == 0) {
            sscanf(linha, "%*s %d", &out->boss_vida);
        }
        else if (strcmp(chave, "BOSS_VEL") == 0) {
            sscanf(linha, "%*s %f", &out->boss_vel);
        }

        // --- Plataforma normal ---
        else if (strcmp(chave, "PLAT") == 0) {
            if (out->num_plats < LEVEL_MAX_PLAT) {
                LevelPlat* p = &out->plats[out->num_plats];
                sscanf(linha, "%*s %d %d %d %d",
                    &p->tipo, &p->x, &p->y, &p->tiles);
                out->num_plats++;
            }
        }

        // --- Plataforma secreta ---
        else if (strcmp(chave, "PLAT_SEC") == 0) {
            sscanf(linha, "%*s %d %d %d",
                &out->plat_secreta.x,
                &out->plat_secreta.y,
                &out->plat_secreta.tiles);
            out->plat_secreta.tipo = 1;
            out->tem_plat_secreta = 1;
        }

        // --- Slime ---
        else if (strcmp(chave, "SLIME") == 0) {
            if (out->num_slimes < LEVEL_MAX_SLIMES) {
                LevelSlime* s = &out->slimes[out->num_slimes];
                sscanf(linha, "%*s %d %f %d %f %f",
                    &s->tipo, &s->x, &s->y_offset, &s->min, &s->max);
                out->num_slimes++;
            }
        }

        // --- Moeda ---
        else if (strcmp(chave, "COIN") == 0) {
            if (out->num_coins < LEVEL_MAX_COINS) {
                LevelCoin* c = &out->coins[out->num_coins];
                sscanf(linha, "%*s %f %f", &c->x, &c->y);
                out->num_coins++;
            }
        }

        // --- Fruta ---
        else if (strcmp(chave, "FRUIT") == 0) {
            if (out->num_fruits < LEVEL_MAX_FRUITS) {
                LevelFruit* fr = &out->fruits[out->num_fruits];
                sscanf(linha, "%*s %d %f %f", &fr->tipo, &fr->x, &fr->y);
                out->num_fruits++;
            }
        }
    }

    fclose(f);
    printf("[LEVEL] Fase carregada: %s\n", out->nome);
    return 1;
}

// ----------------------------------------------------------------
// GERAR DEFAULTS
// ----------------------------------------------------------------
void level_gerar_defaults(void) {

    // --- fase1.dat ---
    FILE* f = fopen("fase1.dat", "w");
    if (f) {
        fprintf(f,
            "# Fase 1\n"
            "NOME        Floresta Sombria\n"
            
            "BG2         png/cenario2_0.jpg\n"
            "SLIME_VEL   2.5\n"
            "BOSS_VIDA   150\n"
            "BOSS_VEL    2.5\n"
            "\n"
            "# Plataformas: PLAT tipo x y tiles\n"
            "PLAT 1 100 560 5\n"
            "PLAT 2 425 480 6\n"
            "PLAT 1 1040 400 5\n"
            "PLAT 2 200 350 4\n"
            "\n"
            "# Plataforma secreta: PLAT_SEC x y tiles\n"
            "PLAT_SEC 620 300 3\n"
            "\n"
            "# Slimes: SLIME tipo x y_offset min max\n"
            "SLIME 0 150 0 50 380\n"
            "SLIME 1 520 0 425 713\n"
            "SLIME 0 860 0 700 1080\n"
            "SLIME 1 1280 0 1040 1280\n"
            "\n"
            "# Moedas: COIN x y\n"
            "COIN 130 470\n"
            "COIN 350 380\n"
            "COIN 500 460\n"
            "COIN 700 300\n"
            "COIN 900 380\n"
            "COIN 1100 460\n"
            "COIN 1300 380\n"
            "\n"
            "# Frutas: FRUIT tipo x y\n"
            "FRUIT 0 250 460\n"
            "FRUIT 1 600 350\n"
            "FRUIT 2 950 420\n"
            "FRUIT 3 1200 350\n"
            "FRUIT 0 400 300\n"
        );
        fclose(f);
        printf("[LEVEL] fase1.dat gerado.\n");
    }

    // --- fase2.dat ---
    f = fopen("fase2.dat", "w");
    if (f) {
        fprintf(f,
            "# Fase 2\n"
            "NOME        Caverna Maldita\n"
            "BG          png/cenario2_0.jpg\n"
            "BG2         png/cenario.jpg\n"
            "SLIME_VEL   3.5\n"
            "BOSS_VIDA   250\n"
            "BOSS_VEL    3.5\n"
            "\n"
            "# Plataformas\n"
            "PLAT 2 80  540 4\n"
            "PLAT 1 350 460 5\n"
            "PLAT 2 700 380 4\n"
            "PLAT 1 1000 440 6\n"
            "PLAT 2 1250 360 4\n"
            "\n"
            "# Plataforma secreta\n"
            "PLAT_SEC 500 250 3\n"
            "\n"
            "# Slimes\n"
            "SLIME 1 200 0 80  420\n"
            "SLIME 1 600 0 450 750\n"
            "SLIME 0 950 0 750 1100\n"
            "SLIME 1 1300 0 1100 1366\n"
            "\n"
            "# Moedas\n"
            "COIN 100 450\n"
            "COIN 300 370\n"
            "COIN 550 460\n"
            "COIN 750 290\n"
            "COIN 950 360\n"
            "COIN 1150 440\n"
            "COIN 1320 370\n"
            "\n"
            "# Frutas\n"
            "FRUIT 1 280 440\n"
            "FRUIT 2 650 350\n"
            "FRUIT 0 1000 410\n"
            "FRUIT 3 1250 340\n"
            "FRUIT 1 450 280\n"
        );
        fclose(f);
        printf("[LEVEL] fase2.dat gerado.\n");
    }
}

// ----------------------------------------------------------------
// DEBUG PRINT
// ----------------------------------------------------------------
void level_print(const LevelData* ld) {
    printf("=== LEVEL DATA ===\n");
    printf("Nome      : %s\n", ld->nome);
    printf("BG        : %s\n", ld->bg_file);
    printf("BG2       : %s\n", ld->bg2_file);
    printf("Slime vel : %.1f\n", ld->slime_vel);
    printf("Boss vida : %d\n", ld->boss_vida);
    printf("Boss vel  : %.1f\n", ld->boss_vel);

    printf("\nPlataformas (%d):\n", ld->num_plats);
    for (int i = 0; i < ld->num_plats; i++)
        printf("  [%d] tipo=%d x=%d y=%d tiles=%d\n", i,
            ld->plats[i].tipo, ld->plats[i].x,
            ld->plats[i].y, ld->plats[i].tiles);

    if (ld->tem_plat_secreta)
        printf("Plat secreta: x=%d y=%d tiles=%d\n",
            ld->plat_secreta.x, ld->plat_secreta.y,
            ld->plat_secreta.tiles);

    printf("\nSlimes (%d):\n", ld->num_slimes);
    for (int i = 0; i < ld->num_slimes; i++)
        printf("  [%d] tipo=%d x=%.0f min=%.0f max=%.0f\n", i,
            ld->slimes[i].tipo, ld->slimes[i].x,
            ld->slimes[i].min, ld->slimes[i].max);

    printf("\nMoedas (%d):\n", ld->num_coins);
    for (int i = 0; i < ld->num_coins; i++)
        printf("  [%d] x=%.0f y=%.0f\n", i,
            ld->coins[i].x, ld->coins[i].y);

    printf("\nFrutas (%d):\n", ld->num_fruits);
    for (int i = 0; i < ld->num_fruits; i++)
        printf("  [%d] tipo=%d x=%.0f y=%.0f\n", i,
            ld->fruits[i].tipo, ld->fruits[i].x, ld->fruits[i].y);

    printf("==================\n");
}