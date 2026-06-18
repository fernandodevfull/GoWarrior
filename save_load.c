#define _CRT_SECURE_NO_WARNINGS
#pragma once
// save_load.c
#include "save_load.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// ----------------------------------------------------------------
// SALVAR
// ----------------------------------------------------------------
int save_jogo(const SaveData* sd) {
    FILE* f = fopen(SAVE_FILE, "wb");
    if (!f) {
        printf("[SAVE] Erro ao abrir arquivo para escrita.\n");
        return 0;
    }

    size_t escritos = fwrite(sd, sizeof(SaveData), 1, f);
    fclose(f);

    if (escritos != 1) {
        printf("[SAVE] Erro ao escrever dados.\n");
        return 0;
    }

    printf("[SAVE] Jogo salvo com sucesso.\n");
    return 1;
}

// ----------------------------------------------------------------
// CARREGAR
// ----------------------------------------------------------------
int load_jogo(SaveData* sd) {
    FILE* f = fopen(SAVE_FILE, "rb");
    if (!f) {
        printf("[LOAD] Nenhum save encontrado.\n");
        return 0;
    }

    size_t lidos = fread(sd, sizeof(SaveData), 1, f);
    fclose(f);

    if (lidos != 1) {
        printf("[LOAD] Erro ao ler arquivo.\n");
        return 0;
    }

    // Valida magic number
    if (sd->magic != SAVE_MAGIC) {
        printf("[LOAD] Arquivo invalido (magic errado).\n");
        return 0;
    }

    // Valida versao
    if (sd->versao != SAVE_VERSION) {
        printf("[LOAD] Versao incompativel (%d != %d).\n",
            sd->versao, SAVE_VERSION);
        return 0;
    }

    printf("[LOAD] Save carregado: %s | Score: %d | Fase: %d\n",
        sd->nome_jogador, sd->score, sd->fase_atual);
    return 1;
}

// ----------------------------------------------------------------
// EXISTE SAVE
// ----------------------------------------------------------------
int existe_save(void) {
    FILE* f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

// ----------------------------------------------------------------
// DELETAR SAVE
// ----------------------------------------------------------------
void deletar_save(void) {
    if (remove(SAVE_FILE) == 0)
        printf("[SAVE] Save deletado.\n");
    else
        printf("[SAVE] Nenhum save para deletar.\n");
}

// ----------------------------------------------------------------
// DEBUG - PRINT
// ----------------------------------------------------------------
void savedata_print(const SaveData* sd) {
    printf("=== SAVE DATA ===\n");
    printf("Jogador    : %s\n", sd->nome_jogador);
    printf("Score      : %d\n", sd->score);
    printf("Highscore  : %d\n", sd->highscore);
    printf("Vidas      : %d\n", sd->vidas);
    printf("Fase atual : %d\n", sd->fase_atual);
    printf("Posicao    : %.1f, %.1f\n", sd->pos_x, sd->pos_y);
    printf("Boss morto : %s\n", sd->boss_morto ? "Sim" : "Nao");
    printf("Tesouro    : %s\n", sd->tesouro_coletado ? "Sim" : "Nao");
    printf("Slimes mortos: %d\n", sd->slimes_mortos);

    printf("Moedas     : ");
    for (int i = 0; i < 7; i++)
        printf("%d ", sd->coin_ativa[i]);
    printf("\n");

    printf("Frutas     : ");
    for (int i = 0; i < 5; i++)
        printf("%d ", sd->fruit_ativa[i]);
    printf("\n");

    printf("Inventario (%d itens):\n", sd->inv_quantidade);
    for (int i = 0; i < sd->inv_quantidade; i++)
        printf("  [%d] %s\n", i, sd->inv_itens[i]);

    printf("Timestamp  : %ld\n", sd->timestamp);
    printf("=================\n");
}
