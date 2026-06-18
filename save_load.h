
#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

//  MODULO: SAVE / LOAD
//  Persiste progresso completo entre sessoes.
//  Arquivo binario: save.dat


#define SAVE_FILE      "save.dat"
#define SAVE_MAGIC     0x474F5741  
#define SAVE_VERSION   1


typedef struct {
    unsigned int magic;             
    int          versao;            

    // Progresso geral
    char  nome_jogador[50];
    int   score;
    int   highscore;
    int   vidas;
    int   fase_atual;               // 1 ou 2

    // Posicao do jogador
    float pos_x;
    float pos_y;

    // Coletaveis
    int   coin_ativa[7];          
    int   fruit_ativa[5];          
    int   tesouro_coletado;
    int   plat_sec_pisada;

    
    int   slime_vivo[4];            
    int   slimes_mortos;

  
    int   boss_vida;
    int   boss_apareceu;
    int   boss_morto;

    
    int   pulo_duplo_ativo;
    float altura_salto_duplo;

    
    int   inv_quantidade;
    char  inv_itens[5][20];

  
    long  timestamp;
} SaveData;

// Retorna 1 em sucesso, 0 em falha
int  save_jogo(const SaveData* sd);
int  load_jogo(SaveData* sd);
int  existe_save(void);
void deletar_save(void);

// preenche/extrai SaveData a partir das variaveis do main
void savedata_print(const SaveData* sd);   

#endif 