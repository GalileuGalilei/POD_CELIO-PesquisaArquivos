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
typedef struct {
    int matr;
    int pos_seek;
} indice;

void mostra(FILE* arq) {
    reg_aluno aluno;
    rewind(arq);
    while (fread(&aluno, sizeof(aluno), 1, arq))
        if (aluno.excluido == 0)
            printf("%d\t%s\t%d\t%d\n", aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
}
int pesquisa(FILE* arq, int matr, reg_aluno* al) {
    reg_aluno aluno;
    rewind(arq);
    while (fread(&aluno, sizeof(aluno), 1, arq))
        if (aluno.matr == matr && aluno.excluido == 0) {
            *al = aluno;
            return 1;
        }
    return 0;
}
void exclui(FILE* arq, int matr) {
    reg_aluno aluno;
    if (pesquisa(arq, matr, &aluno)) {
        int excl = 1;
        printf("Excluindo: %s\n", aluno.nome);
        fseek(arq, -1 * sizeof(int), SEEK_CUR);
        fwrite(&excl, sizeof(int), 1, arq);
        fflush(arq);
    }
}
void inclui(FILE* arq) {
    reg_aluno aluno;
    fseek(arq, 0, SEEK_END);
    //printf("Pos=%d", ftell(arq));
    printf("Informe os dados do aluno (matr, nome, nota1 e nota2) \n");
    scanf("%d%s%d%d", &aluno.matr, aluno.nome, &aluno.nota1, &aluno.nota2);
    aluno.excluido = 0;
    fwrite(&aluno, sizeof(aluno), 1, arq);
}
void main() {
    int matr, op;
    reg_aluno aluno;
    FILE* arq;
    if (fopen("alunos.dat", "r") == NULL) //access() só pra linux!!!
        arq = fopen("alunos.dat", "r+"); // arquivo existe
    else
        arq = fopen("alunos.dat", "w+"); // arquivo nao existia
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
            if (pesquisa(arq, matr, &aluno)) {
                printf("\nAluno encontrado:\n");
                printf("%d\t%s\t%d\t%d\n", aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
            }
            else
                printf("\nA matricula %d nao foi encontrada!\n", matr);
            break;
        case 3:
            inclui(arq);
            break;
        case 4:
            printf("\nDigite a matricula a ser excluida: ");
            scanf("%d", &matr);
            exclui(arq, matr);
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

