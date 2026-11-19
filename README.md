# Projeto 2ºGQ — Comparação de Estruturas de Dados

## 1. Objetivo

Este projeto tem como objetivo implementar e comparar o desempenho de diferentes estruturas de dados na inserção e busca de estudantes por matrícula, utilizando a linguagem C.

A base disponibilizada contém 10 milhões de registros, com matrícula, nome completo e curso.

## 2. Estruturas implementadas

Foram escolhidas três estruturas de dados:

- Lista simplesmente encadeada.
- Tabela hash com endereçamento aberto e sondagem linear.
- Tabela hash com encadeamento separado.

## 3. Funções hash

Foram implementadas duas funções hash:

- Método da divisão.
- Algoritmo FNV-1a de 64 bits.

As duas funções são utilizadas nas tabelas hash, resultando em quatro configurações, além da lista simplesmente encadeada.

## 4. Organização do projeto

- `src/projeto.h`: definições das estruturas e funções.
- `src/dados_lista.c`: leitura dos registros e implementação da lista encadeada.
- `src/hash.c`: implementação das funções hash e das tabelas hash.
- `src/experimentos.c`: execução dos experimentos e medição de desempenho.
- `dados/alunos.txt`: base original de estudantes.
- `resultados/`: arquivos CSV produzidos pelos experimentos.

## 5. Compilação

O programa utiliza C11 e pode ser compilado com GCC.

No terminal, dentro da pasta principal do projeto, executar:

`gcc -std=c11 -O2 -Wall -Wextra -pedantic src/dados_lista.c src/hash.c src/experimentos.c -o projeto.exe`

## 6. Execução

Para testar com 1.000 registros:

`.\projeto.exe dados/alunos.txt 1000 resultados/teste.csv`

Para executar os experimentos completos:

`.\projeto.exe dados/alunos.txt todos resultados/resultados.csv`

## 7. Metodologia

São realizados experimentos com três volumes:

- 100.000 registros.
- 1.000.000 de registros.
- 10.000.000 de registros.

Cada volume é testado nos níveis de preenchimento de 30%, 60% e 100%.

Em cada etapa, são medidas:

- Tempo de inserção dos registros no intervalo.
- Tempo total de 10.000 buscas por matrículas existentes.
- Quantidade de colisões nas tabelas hash.
- Fator de carga das tabelas hash.

Os registros são inseridos na ordem original da base. As mesmas matrículas e a mesma sequência de buscas são utilizadas nas cinco configurações.

O tempo de leitura do arquivo e de seleção das matrículas não é incluído nas medições.

A capacidade das quatro tabelas hash é igual e permanece fixa para cada volume. É escolhido o primeiro número primo maior ou igual a duas vezes o volume, mais um, mantendo o fator de carga máximo próximo de 0,5.

## 8. Resultados

Os experimentos produzem arquivos CSV com tempos de inserção e busca, colisões e fatores de carga.

Os resultados serão utilizados para construir gráficos, comparar as implementações e identificar aquelas que apresentarem o melhor desempenho.

## 9. Referência

FNV-1a — RFC 9923: The FNV Non-Cryptographic Hash Algorithm.

https://www.rfc-editor.org/rfc/rfc9923.html