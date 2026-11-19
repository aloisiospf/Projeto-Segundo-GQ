
#include "projeto.h"
#include <stdlib.h>
#include <string.h>

struct NoHash {
    const Aluno *aluno;
    struct NoHash *proximo;
};

static int eh_primo(size_t numero) {
    if (numero < 2)
        return 0;

    if (numero % 2 == 0)
        return numero == 2;

    for (size_t divisor = 3;
         divisor <= numero / divisor;
         divisor += 2) {

        if (numero % divisor == 0)
            return 0;
    }

    return 1;
}

size_t capacidade_prima(size_t valor) {
    if (valor <= 2)
        return 2;

    if (valor % 2 == 0)
        valor++;

    while (!eh_primo(valor)) {
        if (valor > SIZE_MAX - 2)
            return 0;

        valor += 2;
    }

    return valor;
}

size_t calcular_hash(uint32_t matricula,
                     size_t capacidade,
                     MetodoHash metodo) {
    if (metodo == HASH_DIVISAO)
        return matricula % capacidade;

    uint64_t resultado = UINT64_C(14695981039346656037);

    for (int i = 0; i < 4; i++) {
        resultado ^= (uint8_t)(matricula & 255u);
        resultado *= UINT64_C(1099511628211);
        matricula >>= 8;
    }

    return (size_t)(resultado % capacidade);
}

void hash_aberto_liberar(HashAberto *hash) {
    free(hash->posicoes);
    free(hash->indices_usados);
    memset(hash, 0, sizeof(*hash));
}

int hash_aberto_iniciar(HashAberto *hash,
                       size_t capacidade,
                       MetodoHash metodo) {
    memset(hash, 0, sizeof(*hash));

    if (!capacidade ||
        capacidade > SIZE_MAX / sizeof(*hash->posicoes))
        return 0;

    hash->posicoes = calloc(capacidade,
                            sizeof(*hash->posicoes));

    hash->indices_usados = calloc(capacidade,
                                 sizeof(*hash->indices_usados));

    if (!hash->posicoes || !hash->indices_usados) {
        hash_aberto_liberar(hash);
        return 0;
    }

    hash->capacidade = capacidade;
    hash->metodo = metodo;

    return 1;
}

int hash_aberto_inserir(HashAberto *hash,
                       const Aluno *aluno) {
    if (hash->quantidade == hash->capacidade)
        return 0;

    size_t inicial = calcular_hash(aluno->matricula,
                                   hash->capacidade,
                                   hash->metodo);

    size_t indice = inicial;

    while (hash->posicoes[indice]) {
        if (hash->posicoes[indice]->matricula ==
            aluno->matricula)
            return 0;

        indice++;

        if (indice == hash->capacidade)
            indice = 0;
    }

    if (hash->indices_usados[inicial])
        hash->colisoes++;
    else
        hash->indices_usados[inicial] = 1;

    hash->posicoes[indice] = aluno;
    hash->quantidade++;

    return 1;
}

const Aluno *hash_aberto_buscar(const HashAberto *hash,
                               uint32_t matricula) {
    size_t inicial = calcular_hash(matricula,
                                   hash->capacidade,
                                   hash->metodo);

    size_t indice = inicial;

    while (hash->posicoes[indice]) {
        if (hash->posicoes[indice]->matricula == matricula)
            return hash->posicoes[indice];

        indice++;

        if (indice == hash->capacidade)
            indice = 0;

        if (indice == inicial)
            break;
    }

    return NULL;
}

void hash_encadeado_liberar(HashEncadeado *hash) {
    if (hash->baldes) {
        for (size_t i = 0; i < hash->capacidade; i++) {
            NoHash *atual = hash->baldes[i];

            while (atual) {
                NoHash *proximo = atual->proximo;
                free(atual);
                atual = proximo;
            }
        }
    }

    free(hash->baldes);
    memset(hash, 0, sizeof(*hash));
}

int hash_encadeado_iniciar(HashEncadeado *hash,
                          size_t capacidade,
                          MetodoHash metodo) {
    memset(hash, 0, sizeof(*hash));

    if (!capacidade ||
        capacidade > SIZE_MAX / sizeof(*hash->baldes))
        return 0;

    hash->baldes = calloc(capacidade,
                          sizeof(*hash->baldes));

    if (!hash->baldes)
        return 0;

    hash->capacidade = capacidade;
    hash->metodo = metodo;

    return 1;
}

int hash_encadeado_inserir(HashEncadeado *hash,
                          const Aluno *aluno) {
    size_t indice = calcular_hash(aluno->matricula,
                                  hash->capacidade,
                                  hash->metodo);

    NoHash *atual = hash->baldes[indice];

    while (atual) {
        if (atual->aluno->matricula == aluno->matricula)
            return 0;

        atual = atual->proximo;
    }

    NoHash *novo = malloc(sizeof(NoHash));

    if (!novo)
        return 0;

    if (hash->baldes[indice])
        hash->colisoes++;

    novo->aluno = aluno;
    novo->proximo = hash->baldes[indice];
    hash->baldes[indice] = novo;

    hash->quantidade++;

    return 1;
}

const Aluno *hash_encadeado_buscar(const HashEncadeado *hash,
                                  uint32_t matricula) {
    size_t indice = calcular_hash(matricula,
                                  hash->capacidade,
                                  hash->metodo);

    NoHash *atual = hash->baldes[indice];

    while (atual) {
        if (atual->aluno->matricula == matricula)
            return atual->aluno;

        atual = atual->proximo;
    }

    return NULL;
}
