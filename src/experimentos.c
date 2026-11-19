
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "projeto.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static const char *nomes[5] = {
    "lista_encadeada",
    "hash_aberto_divisao",
    "hash_aberto_fnv1a",
    "hash_encadeado_divisao",
    "hash_encadeado_fnv1a"
};

static double agora(void) {
#ifdef _WIN32
    LARGE_INTEGER frequencia;
    LARGE_INTEGER contador;

    QueryPerformanceFrequency(&frequencia);
    QueryPerformanceCounter(&contador);

    return (double)contador.QuadPart /
           (double)frequencia.QuadPart;
#else
    struct timespec tempo;

    clock_gettime(CLOCK_MONOTONIC, &tempo);

    return (double)tempo.tv_sec +
           (double)tempo.tv_nsec / 1000000000.0;
#endif
}

static uint64_t proximo_aleatorio(uint64_t *estado) {
    uint64_t valor =
        (*estado += UINT64_C(0x9E3779B97F4A7C15));

    valor = (valor ^ (valor >> 30)) *
            UINT64_C(0xBF58476D1CE4E5B9);

    valor = (valor ^ (valor >> 27)) *
            UINT64_C(0x94D049BB133111EB);

    return valor ^ (valor >> 31);
}

static int executar_configuracao(
    const Base *base,
    const uint32_t buscas[3][10000],
    size_t volume,
    size_t capacidade,
    int configuracao,
    FILE *saida
) {
    Lista lista = {0};
    HashAberto aberto = {0};
    HashEncadeado encadeado = {0};

    int sucesso = 0;
    int preparado = 0;

    MetodoHash metodo =
        (configuracao == 1 || configuracao == 3) ?
        HASH_DIVISAO : HASH_FNV1A;

    if (configuracao == 0) {
        lista_iniciar(&lista);
        preparado = 1;
    } else if (configuracao <= 2) {
        preparado = hash_aberto_iniciar(
            &aberto, capacidade, metodo);
    } else {
        preparado = hash_encadeado_iniciar(
            &encadeado, capacidade, metodo);
    }

    if (!preparado) {
        fprintf(stderr, "Falha ao inicializar estrutura.\n");
        goto finalizar;
    }

    size_t limites[3] = {
        volume * 3 / 10,
        volume * 6 / 10,
        volume
    };

    size_t anterior = 0;
    uint64_t colisoes_anteriores = 0;

    for (int etapa = 0; etapa < 3; etapa++) {
        double inicio = agora();

        for (size_t i = anterior; i < limites[etapa]; i++) {
            const Aluno *aluno = &base->registros[i];
            int inserido = 0;

            if (configuracao == 0) {
                inserido = lista_inserir(&lista, aluno);
            } else if (configuracao <= 2) {
                inserido = hash_aberto_inserir(&aberto, aluno);
            } else {
                inserido = hash_encadeado_inserir(
                    &encadeado, aluno);
            }

            if (!inserido) {
                fprintf(stderr,
                        "Erro ao inserir matricula %u.\n",
                        (unsigned int)aluno->matricula);
                goto finalizar;
            }
        }

        double tempo_insercao = agora() - inicio;
        size_t encontrados = 0;

        inicio = agora();

        for (size_t j = 0; j < 10000; j++) {
            uint32_t matricula = buscas[etapa][j];
            const Aluno *encontrado = NULL;

            if (configuracao == 0) {
                encontrado = lista_buscar(
                    &lista, matricula);
            } else if (configuracao <= 2) {
                encontrado = hash_aberto_buscar(
                    &aberto, matricula);
            } else {
                encontrado = hash_encadeado_buscar(
                    &encadeado, matricula);
            }

            if (!encontrado ||
                encontrado->matricula != matricula) {
                fprintf(stderr,
                        "Erro na busca da matricula %u.\n",
                        (unsigned int)matricula);
                goto finalizar;
            }

            encontrados++;
        }

        double tempo_busca = agora() - inicio;
        uint64_t colisoes_totais = 0;

        if (configuracao == 1 || configuracao == 2) {
            colisoes_totais = aberto.colisoes;
        } else if (configuracao >= 3) {
            colisoes_totais = encadeado.colisoes;
        }

        int percentual =
            etapa == 0 ? 30 : etapa == 1 ? 60 : 100;

        if (fprintf(
                saida,
                "%s,%zu,%d,%zu,%zu,%.9f,%.9f,",
                nomes[configuracao],
                volume,
                percentual,
                limites[etapa],
                limites[etapa] - anterior,
                tempo_insercao,
                tempo_busca
            ) < 0)
            goto finalizar;

        if (configuracao == 0) {
            if (fprintf(saida, ",,,,%zu\n",
                        encontrados) < 0)
                goto finalizar;
        } else {
            if (fprintf(
                    saida,
                    "%llu,%llu,%.9f,%zu,%zu\n",
                    (unsigned long long)(
                        colisoes_totais -
                        colisoes_anteriores),
                    (unsigned long long)colisoes_totais,
                    (double)limites[etapa] /
                    (double)capacidade,
                    capacidade,
                    encontrados
                ) < 0)
                goto finalizar;
        }

        if (fflush(saida) != 0)
            goto finalizar;

        printf(
            "Volume %zu | %s | %d%% | "
            "Insercao %.6f s | Busca %.6f s\n",
            volume,
            nomes[configuracao],
            percentual,
            tempo_insercao,
            tempo_busca
        );

        anterior = limites[etapa];
        colisoes_anteriores = colisoes_totais;
    }

    sucesso = 1;

finalizar:
    if (configuracao == 0) {
        lista_liberar(&lista);
    } else if (configuracao <= 2) {
        hash_aberto_liberar(&aberto);
    } else {
        hash_encadeado_liberar(&encadeado);
    }

    return sucesso;
}

int executar_experimentos(
    const char *caminho,
    const size_t *volumes,
    size_t quantidade_volumes,
    const char *saida
) {
    FILE *resultado = fopen(saida, "w");

    if (!resultado) {
        perror(saida);
        return 0;
    }

    if (fprintf(
            resultado,
            "configuracao,volume,percentual,"
            "registros_acumulados,registros_intervalo,"
            "tempo_insercao_s,tempo_busca_s,"
            "colisoes_intervalo,colisoes_acumuladas,"
            "fator_carga,capacidade_hash,"
            "buscas_encontradas\n"
        ) < 0) {
        fclose(resultado);
        return 0;
    }

    for (size_t v = 0; v < quantidade_volumes; v++) {
        size_t volume = volumes[v];

        if (volume < 10 ||
            volume > (SIZE_MAX - 1) / 2) {
            fprintf(stderr, "Volume invalido.\n");
            fclose(resultado);
            return 0;
        }

        Base base;

        if (!base_carregar(&base, caminho, volume)) {
            fclose(resultado);
            return 0;
        }

        size_t capacidade =
            capacidade_prima(2 * volume + 1);

        if (!capacidade) {
            base_liberar(&base);
            fclose(resultado);
            return 0;
        }

        uint32_t buscas[3][10000];

        size_t limites[3] = {
            volume * 3 / 10,
            volume * 6 / 10,
            volume
        };

        uint64_t estado =
            UINT64_C(0x96ABCDEF12345678) ^
            (uint64_t)volume;

        for (int etapa = 0; etapa < 3; etapa++) {
            for (size_t j = 0; j < 10000; j++) {
                size_t indice = (size_t)(
                    proximo_aleatorio(&estado) %
                    limites[etapa]
                );

                buscas[etapa][j] =
                    base.registros[indice].matricula;
            }
        }

        printf(
            "\nVolume: %zu | Capacidade hash: %zu\n",
            volume, capacidade
        );

        for (int configuracao = 0;
             configuracao < 5;
             configuracao++) {

            if (!executar_configuracao(
                    &base,
                    buscas,
                    volume,
                    capacidade,
                    configuracao,
                    resultado
                )) {
                base_liberar(&base);
                fclose(resultado);
                return 0;
            }
        }

        base_liberar(&base);
    }

    if (fclose(resultado) != 0)
        return 0;

    return 1;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 4) {
        fprintf(
            stderr,
            "Uso: %s arquivo.txt [todos|volume] "
            "[resultado.csv]\n",
            argv[0]
        );

        return 1;
    }

    size_t volumes[3] = {
        100000,
        1000000,
        10000000
    };

    size_t quantidade_volumes = 3;

    const char *saida =
        argc == 4 ? argv[3] :
        "resultados/resultados.csv";

    if (argc >= 3 &&
        strcmp(argv[2], "todos") != 0) {

        errno = 0;
        char *fim = NULL;

        unsigned long long valor =
            strtoull(argv[2], &fim, 10);

        if (errno ||
            fim == argv[2] ||
            *fim ||
            argv[2][0] == '-' ||
            valor > SIZE_MAX ||
            valor < 10) {

            fprintf(stderr, "Volume invalido.\n");
            return 1;
        }

        volumes[0] = (size_t)valor;
        quantidade_volumes = 1;
    }

    if (!executar_experimentos(
            argv[1],
            volumes,
            quantidade_volumes,
            saida
        ))
        return 1;

    printf(
        "\nExperimentos concluidos. Arquivo: %s\n",
        saida
    );

    return 0;
}
