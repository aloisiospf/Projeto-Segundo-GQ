
#include "projeto.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NoLista {
    const Aluno *aluno;
    struct NoLista *proximo;
};

static int reservar_texto(Base *base, size_t adicional) {
    if (adicional > UINT32_MAX ||
        base->bytes_usados > UINT32_MAX - adicional)
        return 0;

    size_t necessario = base->bytes_usados + adicional;

    if (necessario <= base->capacidade_textos)
        return 1;

    size_t nova = base->capacidade_textos ?
                  base->capacidade_textos : 1048576;

    while (nova < necessario) {
        if (nova > SIZE_MAX / 2) {
            nova = necessario;
            break;
        }

        nova *= 2;
    }

    char *temporario = realloc(base->textos, nova);

    if (!temporario)
        return 0;

    base->textos = temporario;
    base->capacidade_textos = nova;
    return 1;
}

void base_liberar(Base *base) {
    free(base->registros);
    free(base->textos);
    memset(base, 0, sizeof(*base));
}

int base_carregar(Base *base, const char *caminho, size_t limite) {
    memset(base, 0, sizeof(*base));

    if (limite == 0 || limite > SIZE_MAX / sizeof(Aluno))
        return 0;

    FILE *arquivo = fopen(caminho, "rb");

    if (!arquivo) {
        perror(caminho);
        return 0;
    }

    base->registros = malloc(limite * sizeof(Aluno));

    if (!base->registros)
        goto falha;

    char linha[512];

    while (base->quantidade < limite &&
           fgets(linha, sizeof(linha), arquivo)) {

        size_t tamanho = strlen(linha);

        if (tamanho == sizeof(linha) - 1 &&
            linha[tamanho - 1] != '\n')
            goto falha;

        char *primeiro = strchr(linha, ';');
        char *segundo = primeiro ?
                        strchr(primeiro + 1, ';') : NULL;

        if (!primeiro || !segundo)
            goto falha;

        *primeiro = '\0';
        *segundo = '\0';

        char *nome = primeiro + 1;
        char *curso = segundo + 1;

        curso[strcspn(curso, "\r\n")] = '\0';

        if (!*nome || !*curso || strchr(curso, ';'))
            goto falha;

        errno = 0;
        char *fim = NULL;

        unsigned long long matricula =
            strtoull(linha, &fim, 10);

        if (errno || fim == linha || *fim ||
            linha[0] == '-' || matricula > UINT32_MAX)
            goto falha;

        size_t bytes_nome = strlen(nome) + 1;
        size_t bytes_curso = strlen(curso) + 1;

        if (!reservar_texto(base, bytes_nome + bytes_curso))
            goto falha;

        Aluno *aluno = &base->registros[base->quantidade];

        aluno->matricula = (uint32_t)matricula;
        aluno->nome_offset = (uint32_t)base->bytes_usados;

        memcpy(base->textos + base->bytes_usados,
               nome, bytes_nome);

        base->bytes_usados += bytes_nome;

        aluno->curso_offset = (uint32_t)base->bytes_usados;

        memcpy(base->textos + base->bytes_usados,
               curso, bytes_curso);

        base->bytes_usados += bytes_curso;
        base->quantidade++;
    }

    if (base->quantidade != limite || ferror(arquivo))
        goto falha;

    fclose(arquivo);
    return 1;

falha:
    fprintf(stderr, "Erro ao carregar o registro %zu.\n",
            base->quantidade + 1);
    fclose(arquivo);
    base_liberar(base);
    return 0;
}

const char *base_nome(const Base *base, const Aluno *aluno) {
    return base->textos + aluno->nome_offset;
}

const char *base_curso(const Base *base, const Aluno *aluno) {
    return base->textos + aluno->curso_offset;
}

void lista_iniciar(Lista *lista) {
    lista->primeiro = NULL;
    lista->ultimo = NULL;
    lista->quantidade = 0;
}

int lista_inserir(Lista *lista, const Aluno *aluno) {
    NoLista *novo = malloc(sizeof(NoLista));

    if (!novo)
        return 0;

    novo->aluno = aluno;
    novo->proximo = NULL;

    if (lista->ultimo)
        lista->ultimo->proximo = novo;
    else
        lista->primeiro = novo;

    lista->ultimo = novo;
    lista->quantidade++;

    return 1;
}

const Aluno *lista_buscar(const Lista *lista,
                         uint32_t matricula) {
    NoLista *atual = lista->primeiro;

    while (atual) {
        if (atual->aluno->matricula == matricula)
            return atual->aluno;

        atual = atual->proximo;
    }

    return NULL;
}

void lista_liberar(Lista *lista) {
    NoLista *atual = lista->primeiro;

    while (atual) {
        NoLista *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }

    lista_iniciar(lista);
}
