#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>

/*
* arquivo : matr | nome | n1 | etc...
* 
* Chaves
* 
*   chaves internas : chave guardada dentro do arquivo
*   chaves externas : chave guardada em outro arquivo ou na memória etc.
* 
*   chaves primarias : chaves não repetem
*   chaves secundárias : pode repetir
*   chave composta : chaves formada pela combinação de vários campos
* 
* Ordenação
* 
*   ordenado fisicamente : desloca todos os registros existem no arquivo (muito custoso)
*   ordenado logicamente : adiciona um novo campo "próximo" que identifica em que posição do 
*        arquivo onde está a estrutura que deveria ser a próxima
* 
* Índices
*   algum tipo de estrutura, arquivo separado, memória etc. que sabe a posição de cada registro 
*   dentro do arquivo. Geralmente possui "posição | chave" e está ordenado
*        memória: arvore, hashTable krl4...
*        arquivo: tamanho fixo, hash table krl4...
*
*    vá para o fim do arquivo   : (fseek(arq, 0, SEEK_END))
*    leia a posição do registro : (posicao_reg = ftell(arq))
*    grava os dados             : (fwrite(&dados, sizeof(dados), 1, arq))
*
*/

typedef struct {
    int matr;
    char nome[25];
    int nota1;
    int nota2;
    int excluido;
} reg_aluno;

//TODO: Implementar um indice (em memoria) para facilitar a busca dos registros do arquivo
//TODO: Alterar as funcoes de busca, insercao e exclusao para usar o indice

//estrutura de pilha
typedef struct {
    int matr;
    int pos_seek;
    struct indice* prox; 
} indice;

void indice_insere(indice** raiz,int matr, int pos_seek)
{
    indice* new_ind = (indice*)malloc(sizeof(indice));
    new_ind->matr = matr;
    new_ind->pos_seek = pos_seek;

    new_ind->prox = *raiz;
    *raiz = new_ind;
}

void indice_exclui(indice** raiz, int matr)
{
    if (*raiz == NULL)
    {
        return;
    }

    indice* aux;
    if ((*raiz)->matr == matr)
    {
        aux = (*raiz)->prox;
        free(*raiz);
        *raiz = aux;
        return;
    }

    indice* ant = *raiz;
    for (indice* i = *raiz; i != NULL; i = i->prox)
    {
        if (i->matr == matr)
        {
            aux = i->prox;
            free(i);
            ant->prox = aux;
            return;
        }
        ant = i;
    }
}

//retorna o pos_seek
int indice_pesquisa(indice* raiz, int matr)
{
    for (indice* i = raiz; i != NULL; i = i->prox)
    {
        if (i->matr == matr)
        {
            return i->pos_seek;
        }
    }

    return -1;
}

void indice_inicializa(indice** ativos, indice** excluidos, FILE* arq)
{
    reg_aluno aluno;
    rewind(arq);

    while (fread(&aluno, sizeof(aluno), 1, arq))
    {
        if (aluno.excluido == 0)
        {
            indice_insere(ativos, aluno.matr, ftell(arq) - sizeof(aluno));
        }
        else
        {
            indice_insere(excluidos, aluno.matr, ftell(arq) - sizeof(aluno));
        }
    }
}

void mostra(FILE* arq) {
    reg_aluno aluno;
    rewind(arq);

    while (fread(&aluno, sizeof(aluno), 1, arq))
    {
        if (aluno.excluido == 0)
        {
            printf("%d\t %s\t %d\t %d\n", aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
        }
    }
}
int pesquisa(FILE* arq, int matr, reg_aluno* al, indice* ativos) {
    reg_aluno aluno;
    int pos_seek = indice_pesquisa(ativos, matr);

    if (pos_seek == -1)
    {
        return 0;
    }

    fseek(arq, pos_seek, SEEK_SET);
    fread(&aluno, sizeof(aluno), 1, arq);
    if (aluno.matr == matr && aluno.excluido == 0) {
        *al = aluno;
        return 1;
    }
    return 0;
}

//quando um aluno é excluido, ele é removido da lista de ativos e adicionados na lista de exluidos;
void exclui(FILE* arq, int matr, indice** ativos, indice** excluidos) 
{
    reg_aluno aluno;
    int pos_seek = indice_pesquisa(*ativos, matr);

    if (pos_seek == -1)
    {
        return 0;
    }

    fseek(arq, pos_seek, SEEK_SET);
    fread(&aluno, sizeof(aluno), 1, arq);
    aluno.excluido = 1;

    printf("Excluindo: %s\n", aluno.nome);

    fseek(arq, pos_seek, SEEK_SET);
    fwrite(&aluno, sizeof(aluno), 1, arq);
    indice_insere(excluidos, matr, pos_seek);
    indice_exclui(ativos, matr);
}

//procura na lista de excluídos uma posição válida, se não encontrar, insere no final do arquivo
void inclui(FILE* arq, indice** ativos, indice** excluidos)
{
    reg_aluno aluno;
    printf("Informe os dados do aluno (matr, nome, nota1 e nota2) \n");
    scanf("%d%s%d%d", &aluno.matr, aluno.nome, &aluno.nota1, &aluno.nota2);

    int pos_seek = indice_pesquisa(*excluidos, aluno.matr);

    if (pos_seek == -1)
    {
        fseek(arq, 0, SEEK_END);
        pos_seek = ftell(arq);
    }

    aluno.excluido = 0;
    indice_insere(ativos, aluno.matr, pos_seek);
    indice_exclui(excluidos, aluno.matr);
    fwrite(&aluno, sizeof(aluno), 1, arq);
}
void main() {
    int matr, op;
    reg_aluno aluno;
    indice* ativos = NULL;
    indice* excluidos = NULL;

    FILE* arq;

    if (fopen("alunos.dat", "r") != NULL) //access() só pra linux!!!
        arq = fopen("alunos.dat", "r+"); // arquivo existe
    else
        arq = fopen("alunos.dat", "w+"); // arquivo nao existia

    indice_inicializa(&ativos, &excluidos, arq);

    do {
        printf("\nMenu\n 1. Mostrar todos\n 2. Pesquisar\n 3. Incluir\n 4. Excluir\n 5. Sair\nInforme uma opcao: ");
        scanf("%d", &op);
        switch (op) {
        case 1:
            printf("\nAlunos gravados no arquivo: \n");
            mostra(arq);
            break;
        case 2:
            printf("\nDigite a matricula a ser buscada: ");
            scanf("%d", &matr);
            if (pesquisa(arq, matr, &aluno, ativos)) {
                printf("\nAluno encontrado:\n");
                printf("%d\t%s\t%d\t%d\n", aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
            }
            else
                printf("\nA matricula %d nao foi encontrada!\n", matr);
            break;
        case 3:
            inclui(arq, &ativos, &excluidos);
            break;
        case 4:
            printf("\nDigite a matricula a ser excluida: ");
            scanf("%d", &matr);
            exclui(arq, matr, &ativos, &excluidos);
            break;
        case 5:
            printf("\nSaindo...\n\n");
            break;
        default: printf("\nOpcao invalida!\n");
            break;
        }
    } while (op != 5);
    fclose(arq);
}

