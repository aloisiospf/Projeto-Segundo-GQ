
#ifndef PROJETO_H
#define PROJETO_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t matricula;
    uint32_t nome_offset;
    uint32_t curso_offset;
} Aluno;

typedef struct {
    Aluno *registros;
    char *textos;
    size_t quantidade;
    size_t bytes_usados;
    size_t capacidade_textos;
} Base;

typedef struct NoLista NoLista;

typedef struct {
    NoLista *primeiro;
    NoLista *ultimo;
    size_t quantidade;
} Lista;

typedef enum {
    HASH_DIVISAO = 1,
    HASH_FNV1A = 2
} MetodoHash;

typedef struct {
    const Aluno **posicoes;
    unsigned char *indices_usados;
    size_t capacidade;
    size_t quantidade;
    uint64_t colisoes;
    MetodoHash metodo;
} HashAberto;

typedef struct NoHash NoHash;

typedef struct {
    NoHash **baldes;
    size_t capacidade;
    size_t quantidade;
    uint64_t colisoes;
    MetodoHash metodo;
} HashEncadeado;

int base_carregar(Base *base, const char *caminho, size_t limite);
void base_liberar(Base *base);
const char *base_nome(const Base *base, const Aluno *aluno);
const char *base_curso(const Base *base, const Aluno *aluno);

void lista_iniciar(Lista *lista);
int lista_inserir(Lista *lista, const Aluno *aluno);
const Aluno *lista_buscar(const Lista *lista, uint32_t matricula);
void lista_liberar(Lista *lista);

size_t capacidade_prima(size_t valor);
size_t calcular_hash(uint32_t matricula, size_t capacidade,
                     MetodoHash metodo);

int hash_aberto_iniciar(HashAberto *hash, size_t capacidade,
                       MetodoHash metodo);
int hash_aberto_inserir(HashAberto *hash, const Aluno *aluno);
const Aluno *hash_aberto_buscar(const HashAberto *hash,
                               uint32_t matricula);
void hash_aberto_liberar(HashAberto *hash);

int hash_encadeado_iniciar(HashEncadeado *hash, size_t capacidade,
                          MetodoHash metodo);
int hash_encadeado_inserir(HashEncadeado *hash, const Aluno *aluno);
const Aluno *hash_encadeado_buscar(const HashEncadeado *hash,
                                  uint32_t matricula);
void hash_encadeado_liberar(HashEncadeado *hash);

int executar_experimentos(const char *caminho,
                          const size_t *volumes,
                          size_t quantidade_volumes,
                          const char *saida);

#endif
