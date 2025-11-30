#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- Definicoes de Constantes ---
#define MAX_NOME 50
#define CAPACIDADE_HASH 7 // Tamanho da Tabela Hash (primo para melhor distribuicao)
#define MAX_PISTAS_SUSPEITO 5 // Maximo de pistas por suspeito

// =================================================================
// ESTRUTURAS DE DADOS
// =================================================================

// --- 1. Estrutura para o Mapa da Mansao (Arvore Binaria) ---
// Struct Sala: no da arvore binaria que representa um comodo na mansao.
typedef struct Sala {
    char nome[MAX_NOME];
    struct Sala *esquerda; // Caminho a esquerda
    struct Sala *direita;  // Caminho a direita
    bool tem_pista;        // Indica se esta sala tem uma pista a ser coletada
} Sala;

// --- 2. Estrutura para Pistas (Arvore Binaria de Busca - BST) ---
// Struct Pista: no da BST que armazena o texto da pista.
typedef struct Pista {
    char texto[MAX_NOME];
    struct Pista *esquerda;
    struct Pista *direita;
} Pista;

// --- 3. Estruturas para Suspeitos (Tabela Hash com Lista Encadeada) ---

// Struct PistaSuspeito: Pista associada a um suspeito (para a lista interna do Suspeito)
typedef struct PistaSuspeito {
    char texto[MAX_NOME];
    struct PistaSuspeito *proximo;
} PistaSuspeito;

// Struct Suspeito: Armazena nome, contador e a lista encadeada de pistas associadas.
typedef struct Suspeito {
    char nome[MAX_NOME];
    int contador; // Quantidade de vezes que o suspeito foi citado
    PistaSuspeito *lista_pistas; // Cabeca da lista de pistas
    struct Suspeito *proximo; // Proximo suspeito na mesma posicao do Hash (tratamento de colisao)
} Suspeito;

// Tabela Hash: Array de ponteiros para a struct Suspeito
Suspeito *tabelaHash[CAPACIDADE_HASH];


// =================================================================
// PROTOTIPOS DE FUNCOES
// =================================================================

// Funcoes do Mapa (Arvore Binaria - Nivel Novato)
Sala* criarSala(const char *nome, bool tem_pista);
void explorarSalas(Sala *sala_atual, Pista **raizPistas);

// Funcoes de Pistas (Arvore de Busca - Nivel Aventureiro)
Pista* criarPista(const char *texto);
Pista* inserirPista(Pista *raiz, const char *texto);
void emOrdem(Pista *raiz);

// Funcoes de Suspeitos (Tabela Hash - Nivel Mestre)
unsigned int funcaoHash(const char *nome);
void inicializarHash();
Suspeito* buscarSuspeito(const char *nome);
void inserirAssociacao(const char *nomeSuspeito, const char *textoPista);
void listarAssociacoes();
void encontrarSuspeitoMaisProvavel();
void liberarHash(); // Libera memoria alocada para a Tabela Hash
void liberarListaPistas(PistaSuspeito *cabeca);
void liberarArvore(Pista *raiz); // Libera memoria alocada para a BST de Pistas

// Funcoes Auxiliares de Construcao
Sala* construirMapa(Pista **raizPistas);


// =================================================================
// IMPLEMENTACAO DAS FUNCOES DO MAPA (ARVORE BINARIA)
// =================================================================

// criarSala(): cria, de forma dinamica, uma sala com nome.
Sala* criarSala(const char *nome, bool tem_pista) {
    Sala *novaSala = (Sala*)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        printf("[ERRO] Falha na alocacao de memoria para nova sala.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(novaSala->nome, nome, MAX_NOME - 1);
    novaSala->nome[MAX_NOME - 1] = '\0';
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    novaSala->tem_pista = tem_pista;
    return novaSala;
}

// Constrói o mapa da mansão com salas e pistas
Sala* construirMapa(Pista **raizPistas) {
	(void)raizPistas;
    // Definicao da Estrutura Fixa (Simulacao da Mansao)
    // Nivel 0
    Sala *hallEntrada = criarSala("Hall de Entrada", false);

    // Nivel 1
    Sala *salaEstar = criarSala("Sala de Estar", true);
    Sala *biblioteca = criarSala("Biblioteca", false);
    hallEntrada->esquerda = salaEstar;
    hallEntrada->direita = biblioteca;

    // Nivel 2
    Sala *cozinha = criarSala("Cozinha", false);
    Sala *jardim = criarSala("Jardim de Inverno", true);
    salaEstar->esquerda = cozinha;
    salaEstar->direita = jardim;

    Sala *escritorio = criarSala("Escritorio do Sr. Black", true);
    Sala *salaJantar = criarSala("Sala de Jantar", false);
    biblioteca->esquerda = escritorio;
    biblioteca->direita = salaJantar;

    // Nivel 3 (Nos-Folha ou caminhos sem mais nos)
    Sala *sotao = criarSala("Sotao", false);
    Sala *quartoHospedes = criarSala("Quarto de Hospedes", true);

    cozinha->esquerda = sotao;
    cozinha->direita = NULL; // Caminho sem saida
    
    jardim->esquerda = NULL; // Caminho sem saida
    jardim->direita = quartoHospedes;

    escritorio->esquerda = NULL;
    escritorio->direita = NULL;
    
    salaJantar->esquerda = criarSala("Adega", false); // Folha
    salaJantar->direita = criarSala("Garagem", false); // Folha

    // Configuracao das Pistas iniciais (Nivel Aventureiro)
    // As pistas serao coletadas ao passar pelas salas, mas pre-definidas aqui.
    // Pistas associadas aos suspeitos para o Nivel Mestre
    inserirAssociacao("Mordomo", "Chave do Escritorio");
    inserirAssociacao("Cozinheira", "Faca de cozinha faltando");
    inserirAssociacao("Jardineiro", "Pegadas de lama");
    inserirAssociacao("Mordomo", "Carta de divida");
    inserirAssociacao("Cozinheira", "Digitais na taca");
    inserirAssociacao("Jardineiro", "Rastros de areia");
    
    return hallEntrada;
}

// explorarSalas(): permite a navegacao do jogador pela arvore.
void explorarSalas(Sala *sala_atual, Pista **raizPistas) {
    Sala *atual = sala_atual;
    char escolha;

    printf("\n--- EXPLORACAO DA MANSAO DETECTIVE QUEST ---\n");
    printf("Voce esta no %s.\n", atual->nome);

    // Laco de exploracao continua
    while (atual != NULL) {
        printf("\nVoce chegou ao: %s\n", atual->nome);
        
        // Coleta de Pistas (Nivel Aventureiro)
        if (atual->tem_pista) {
            char textoPista[MAX_NOME];
            printf("[PISTA] Voce encontrou uma evidencia nesta sala!\n");

            // Simula a coleta de pistas nas salas (texto fixo para simplicidade)
            if (strcmp(atual->nome, "Sala de Estar") == 0) {
                strcpy(textoPista, "Um copo quebrado.");
                *raizPistas = inserirPista(*raizPistas, textoPista);
            } else if (strcmp(atual->nome, "Jardim de Inverno") == 0) {
                strcpy(textoPista, "Flores pisoteadas.");
                *raizPistas = inserirPista(*raizPistas, textoPista);
            } else if (strcmp(atual->nome, "Escritorio do Sr. Black") == 0) {
                strcpy(textoPista, "Documento rasgado.");
                *raizPistas = inserirPista(*raizPistas, textoPista);
            } else if (strcmp(atual->nome, "Quarto de Hospedes") == 0) {
                strcpy(textoPista, "Pequena mancha de oleo.");
                *raizPistas = inserirPista(*raizPistas, textoPista);
            }
            atual->tem_pista = false; // Garante que a pista so seja coletada uma vez
            printf("[PISTA COLETADA] '%s'\n", textoPista);
        }

        // Verifica se e um no-folha ou se a exploracao acabou
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("\n[FIM DA LINHA] Este comodo nao possui mais caminhos. Fim da exploracao.\n");
            break;
        }

        // Menu de escolha do jogador (Usabilidade)
        printf("\nEscolha o proximo caminho: ");
        if (atual->esquerda) {
            printf("(e) para %s", atual->esquerda->nome);
        }
        if (atual->direita) {
            printf(" | (d) para %s", atual->direita->nome);
        }
        printf(" | (s) para Sair da Mansao | (v) para Ver Pistas: ");
        
        // Leitura da escolha
        if (scanf(" %c", &escolha) != 1) {
            while (getchar() != '\n');
            printf("[AVISO] Entrada invalida. Tente novamente.\n");
            continue;
        }
        
        // Controle de fluxo e navegacao (Operadores condicionais)
        if (escolha == 'e' || escolha == 'E') {
            if (atual->esquerda != NULL) {
                atual = atual->esquerda;
            } else {
                printf("[AVISO] Nao ha caminho a esquerda. Tente outra direcao.\n");
            }
        } else if (escolha == 'd' || escolha == 'D') {
            if (atual->direita != NULL) {
                atual = atual->direita;
            } else {
                printf("[AVISO] Nao ha caminho a direita. Tente outra direcao.\n");
            }
        } else if (escolha == 's' || escolha == 'S') {
            printf("\nSaindo da mansao...\n");
            break;
        } else if (escolha == 'v' || escolha == 'V') {
            printf("\n--- PISTAS COLETADAS (BST EM ORDEM) ---\n");
            if (*raizPistas == NULL) {
                printf("Nenhuma pista coletada ate agora.\n");
            } else {
                emOrdem(*raizPistas);
            }
        } else {
            printf("[AVISO] Escolha invalida. Use 'e', 'd', 's' ou 'v'.\n");
        }
    }
}


// =================================================================
// IMPLEMENTACAO DAS FUNCOES DE PISTAS (BST)
// =================================================================

// criarPista(): Cria, de forma dinamica, uma pista com nome.
Pista* criarPista(const char *texto) {
    Pista *novaPista = (Pista*)malloc(sizeof(Pista));
    if (novaPista == NULL) {
        printf("[ERRO] Falha na alocacao de memoria para nova pista.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(novaPista->texto, texto, MAX_NOME - 1);
    novaPista->texto[MAX_NOME - 1] = '\0';
    novaPista->esquerda = NULL;
    novaPista->direita = NULL;
    return novaPista;
}

// inserirPista(): Insere uma pista na Arvore de Busca Binaria.
Pista* inserirPista(Pista *raiz, const char *texto) {
    if (raiz == NULL) {
        return criarPista(texto);
    }

    int comparacao = strcmp(texto, raiz->texto);

    if (comparacao < 0) {
        raiz->esquerda = inserirPista(raiz->esquerda, texto);
    } else if (comparacao > 0) {
        raiz->direita = inserirPista(raiz->direita, texto);
    } 
    // Se comparacao == 0, a pista ja existe e nao e inserida

    return raiz;
}

// emOrdem(): Exibe as pistas em ordem alfabética (percurso emOrdem da BST).
void emOrdem(Pista *raiz) {
    if (raiz != NULL) {
        emOrdem(raiz->esquerda);
        printf("- %s\n", raiz->texto);
        emOrdem(raiz->direita);
    }
}


// =================================================================
// IMPLEMENTACAO DAS FUNCOES DE SUSPEITOS (TABELA HASH)
// =================================================================

// funcaoHash(): Funcao simples que usa o primeiro caractere do nome.
unsigned int funcaoHash(const char *nome) {
    return (unsigned int)nome[0] % CAPACIDADE_HASH;
}

// inicializarHash(): Inicializa a tabela hash com ponteiros nulos.
void inicializarHash() {
    for (int i = 0; i < CAPACIDADE_HASH; i++) {
        tabelaHash[i] = NULL;
    }
}

// buscarSuspeito(): Busca um suspeito na tabela hash.
Suspeito* buscarSuspeito(const char *nome) {
    unsigned int indice = funcaoHash(nome);
    Suspeito *atual = tabelaHash[indice];

    // Percorre a lista encadeada (tratamento de colisao)
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL; // Suspeito nao encontrado
}

// inserirAssociacao(): Insere ou atualiza um suspeito e associa uma nova pista.
void inserirAssociacao(const char *nomeSuspeito, const char *textoPista) {
    Suspeito *suspeito = buscarSuspeito(nomeSuspeito);

    if (suspeito == NULL) {
        // Suspeito nao existe, cria novo no
        unsigned int indice = funcaoHash(nomeSuspeito);
        suspeito = (Suspeito*)malloc(sizeof(Suspeito));
        if (suspeito == NULL) {
             printf("[ERRO] Falha na alocacao de memoria para novo suspeito.\n");
             return;
        }
        strncpy(suspeito->nome, nomeSuspeito, MAX_NOME - 1);
        suspeito->nome[MAX_NOME - 1] = '\0';
        suspeito->contador = 0;
        suspeito->lista_pistas = NULL;
        
        // Insere no inicio da lista encadeada do bucket (tratamento de colisao)
        suspeito->proximo = tabelaHash[indice];
        tabelaHash[indice] = suspeito;
    }

    // Atualiza contador
    suspeito->contador++;

    // Adiciona a pista a lista encadeada do suspeito
    PistaSuspeito *novaPista = (PistaSuspeito*)malloc(sizeof(PistaSuspeito));
    if (novaPista == NULL) {
         printf("[ERRO] Falha na alocacao de memoria para nova pista do suspeito.\n");
         return;
    }
    strncpy(novaPista->texto, textoPista, MAX_NOME - 1);
    novaPista->texto[MAX_NOME - 1] = '\0';
    novaPista->proximo = suspeito->lista_pistas;
    suspeito->lista_pistas = novaPista;
}

// listarAssociacoes(): Mostra todos os suspeitos e suas pistas (percorre toda a Hash).
void listarAssociacoes() {
    printf("\n--- SUSPEITOS E PISTAS ASSOCIADAS (TABELA HASH) ---\n");
    bool encontrado = false;
    for (int i = 0; i < CAPACIDADE_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        while (atual != NULL) {
            encontrado = true;
            printf("\nSuspeito: %s (Citacoes: %d)\n", atual->nome, atual->contador);
            printf("  Pistas:\n");
            PistaSuspeito *pistaAtual = atual->lista_pistas;
            if (pistaAtual == NULL) {
                printf("    - Nenhuma pista associada.\n");
            } else {
                while (pistaAtual != NULL) {
                    printf("    - %s\n", pistaAtual->texto);
                    pistaAtual = pistaAtual->proximo;
                }
            }
            atual = atual->proximo;
        }
    }
    if (!encontrado) {
        printf("Nenhum suspeito registrado.\n");
    }
}

// encontrarSuspeitoMaisProvavel(): Encontra o suspeito com maior contador.
void encontrarSuspeitoMaisProvavel() {
    char nomeMaisProvavel[MAX_NOME] = "";
    int maxContador = -1;
    bool encontrado = false;

    for (int i = 0; i < CAPACIDADE_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        while (atual != NULL) {
            encontrado = true;
            if (atual->contador > maxContador) {
                maxContador = atual->contador;
                strncpy(nomeMaisProvavel, atual->nome, MAX_NOME - 1);
                nomeMaisProvavel[MAX_NOME - 1] = '\0';
            }
            atual = atual->proximo;
        }
    }

    printf("\n--- VEREDITO ---\n");
    if (encontrado && maxContador > 0) {
        printf("O suspeito mais provavel e: %s\n", nomeMaisProvavel);
        printf("Baseado em %d citacoes de pistas.\n", maxContador);
    } else {
        printf("Nao ha pistas suficientes ou o crime foi perfeito.\n");
    }
}

// Funcoes de liberacao de memoria
void liberarListaPistas(PistaSuspeito *cabeca) {
    PistaSuspeito *temp;
    while (cabeca != NULL) {
        temp = cabeca;
        cabeca = cabeca->proximo;
        free(temp);
    }
}

void liberarHash() {
    for (int i = 0; i < CAPACIDADE_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        Suspeito *temp;
        while (atual != NULL) {
            temp = atual;
            atual = atual->proximo;
            liberarListaPistas(temp->lista_pistas);
            free(temp);
        }
        tabelaHash[i] = NULL;
    }
}

void liberarArvore(Pista *raiz) {
    if (raiz != NULL) {
        liberarArvore(raiz->esquerda);
        liberarArvore(raiz->direita);
        free(raiz);
    }
}

// =================================================================
// FUNCAO PRINCIPAL
// =================================================================

int main() {
    
    // Inicializacao das estruturas
    Pista *raizPistas = NULL; // Cabeca da BST de pistas
    inicializarHash();

    // main(): monta o mapa inicial e da inicio a exploracao.
    Sala *hallEntrada = construirMapa(&raizPistas);

    printf("--- Detective Quest: O Misterio da Mansao ---\n");
    printf("Bem-vindo(a)! Sua missao e explorar a mansao a partir do Hall de Entrada.\n");

    // Inicio da exploracao (Nivel Novato)
    explorarSalas(hallEntrada, &raizPistas);
    
    // Analise das Pistas e Suspeitos (Nivel Mestre)
    listarAssociacoes();
    encontrarSuspeitoMaisProvavel();
    
    // Liberacao de memoria
    liberarHash();
    liberarArvore(raizPistas);
    // A liberacao da arvore do mapa foi omitida para simplificacao, mas seria necessaria

    return 0;
}
