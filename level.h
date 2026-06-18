#pragma once
#ifndef LEVEL_H
#define LEVEL_H

// ================================================================
//  MODULO: LEVEL LOADER
//  Le configuracoes de fase de arquivos .dat externos.
//
//  FORMATO DO ARQUIVO (texto puro, ex: fase1.dat):
//  -----------------------------------------------
//  # Comentarios comecam com '#'
//  NOME        Floresta Sombria
//  BG          cenario.jpg
//  BG2         cenario2.0.jpg          # usado se fase > 1
//  SLIME_VEL   2.5
//  BOSS_VIDA   150
//  BOSS_VEL    2.5
//
//  # Plataformas: PLAT tipo x y tiles
//  PLAT 1 100 560 5
//  PLAT 2 425 480 6
//  PLAT 1 1040 400 5
//  PLAT 2 200 350 4
//
//  # Plataforma secreta: PLAT_SEC x y tiles
//  PLAT_SEC 620 300 3
//
//  # Slimes: SLIME tipo(0=verde,1=roxo) x y min max
//  SLIME 0 150 0 50 380
//  SLIME 1 520 0 425 713
//  SLIME 0 860 0 700 1080
//  SLIME 1 1280 0 1040 1280
//
//  # Moedas: COIN x y
//  COIN 130 470
//  COIN 350 380
//  ...
//
//  # Frutas: FRUIT tipo x y  (tipo 0-3)
//  FRUIT 0 250 460
//  ...
// ================================================================

#define LEVEL_MAX_PLAT      8
#define LEVEL_MAX_SLIMES    4
#define LEVEL_MAX_COINS     10
#define LEVEL_MAX_FRUITS    8
#define LEVEL_NAME_LEN      64

// Dados de uma plataforma
typedef struct {
    int tipo, x, y, tiles;
} LevelPlat;

// Dados de um slime
typedef struct {
    int   tipo;      // 0=verde, 1=roxo
    float x;
    int   y_offset;  // 0 = chao; >0 = CHAO_Y - valor (calculado pelo loader)
    float min, max;
} LevelSlime;

// Dados de uma moeda
typedef struct { float x, y; } LevelCoin;

// Dados de uma fruta
typedef struct { int tipo; float x, y; } LevelFruit;

// Configuracao completa de uma fase
typedef struct {
    char  nome[LEVEL_NAME_LEN];
    char  bg_file[LEVEL_NAME_LEN];
    char  bg2_file[LEVEL_NAME_LEN];

    float slime_vel;
    int   boss_vida;
    float boss_vel;

    LevelPlat   plats[LEVEL_MAX_PLAT];
    int         num_plats;

    LevelPlat   plat_secreta;
    int         tem_plat_secreta;

    LevelSlime  slimes[LEVEL_MAX_SLIMES];
    int         num_slimes;

    LevelCoin   coins[LEVEL_MAX_COINS];
    int         num_coins;

    LevelFruit  fruits[LEVEL_MAX_FRUITS];
    int         num_fruits;
} LevelData;

// Carrega um .dat e preenche LevelData.
// Retorna 1 em sucesso, 0 se o arquivo nao for encontrado/invalido.
int level_load(const char* filename, LevelData* out);

// Gera arquivos .dat default para fase1 e fase2 (util para primeiro uso)
void level_gerar_defaults(void);

// Debug: imprime LevelData no console
void level_print(const LevelData* ld);

#endif // LEVEL_H