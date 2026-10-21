<p id="title" align="center">
  <a href="#title">
    <h1 align="center">Amazon Balanced Trees</h1>
  </a>
</p>

<p align="center">
  <a aria-label="Made By Aristofany" href="https://github.com/aristofany-herderson/">
    <img src="https://img.shields.io/badge/MADE%20BY%20Aristofany-000000.svg?style=for-the-badge&labelColor=000&logo=starship&logoColor=fff&logoWidth=20">
  </a>
  <a aria-label="C++17" href="https://en.cppreference.com/w/cpp/17">
    <img alt="" src="https://img.shields.io/badge/C%2B%2B-17-000000.svg?style=for-the-badge&labelColor=000&logo=cplusplus&logoColor=fff">
  </a>
  <a aria-label="Disciplina" href="#">
    <img alt="" src="https://img.shields.io/badge/IMD0039-EDB%20II-000000.svg?style=for-the-badge&labelColor=000">
  </a>
  <a aria-label="Enjoy My Repos" href="https://github.com/aristofany-herderson?tab=repositories">
    <img alt="" src="https://img.shields.io/badge/Enjoy%20My%20Projects-000000.svg?style=for-the-badge&color=000&logo=github&labelColor=000000&logoColor=fff&logoWidth=20">
  </a>
</p>

<p align="center">📦 Árvores AVL e Rubro-Negra para o índice de encomendas da Black Friday — Amazon Engineering Challenge</p>

<br>

## 🧪&nbsp; Tecnologias

Este projeto foi desenvolvido com as seguintes tecnologias:

- [C++17](https://en.cppreference.com/w/cpp/17)
- [GCC / MinGW-w64](https://www.msys2.org/) (testado com g++ 16.2, MSYS2 UCRT64)
- [CMake](https://cmake.org/) (opcional)

Dependências do projeto:

- Nenhuma. Apenas a biblioteca padrão do C++. Os algoritmos de balanceamento (rotações, recolorações e casos de correção) foram implementados do zero; `std::set` é usado **somente** como oráculo nos testes.

<br>

## 🧑🏻‍💻&nbsp; Começando

Clone o projeto e acesse a pasta

```bash
$ git clone https://github.com/aristofany-herderson/amazon-balanced-trees
$ cd amazon-balanced-trees
```

Os executáveis para Windows já estão compilados em `bin/` (linkagem estática, não precisam de DLLs). Para compilar novamente:

```bash
# Linux, macOS ou Git Bash/MSYS2 no Windows
$ ./build.sh

# Prompt de comando do Windows
> build.bat

# Ou com CMake
$ cmake -S . -B build && cmake --build build
```

Para executar:

```bash
# Programa interativo (pergunta qual árvore usar)
$ ./bin/arvores

# Já escolhendo a árvore
$ ./bin/arvores --avl
$ ./bin/arvores --rb

# Testes automatizados
$ ./bin/testes

# Análise comparativa (gera CSV em results/)
$ ./bin/comparar
```

Opções extras: `--no-color` desativa as cores ANSI do `arvores`; `comparar` aceita `--out <pasta>`, `--seed <n>` e `--quick`.

<br>

## 💻&nbsp; Projeto

**Resumo:** 📦 Protótipo de índice de identificadores de encomendas usando duas árvores balanceadas, **AVL** e **Rubro-Negra**, com a mesma interface. O programa permite escolher a estrutura em tempo de execução, inserir, remover, buscar, listar em ordem e visualizar a árvore (com as cores dos nós na Rubro-Negra), e conta rotações, recolorações e casos de balanceamento para a análise comparativa.

#### ❔&nbsp; Por que fazer?

Trabalho da Unidade II de **IMD0039 – Estruturas de Dados Básicas II (T05, 2026.2)**. Na Black Friday os IDs de pedidos chegam praticamente em ordem crescente; uma árvore de busca binária comum degenera em lista (altura 10.000 para 10.000 IDs crescentes, medido pelo `comparar`). O objetivo é comparar as duas estratégias de balanceamento com os mesmos dados e recomendar uma delas com base em evidências.

<br />

#### 🚀&nbsp; Funcionalidades

- [x] Escolha da árvore (AVL ou Rubro-Negra) ao iniciar ou pelo menu
- [x] Inserção de um ou vários IDs (separados por espaço ou vírgula)
- [x] Remoção de um ou vários IDs
- [x] Busca, informando quantos nós foram visitados
- [x] Percurso em ordem (IDs em ordem crescente)
- [x] Visualização da estrutura: `[R]` rubro / `[N]` negro (com cor no terminal) na RB; altura `h` e fator `fb` na AVL
- [x] Geração de lotes de IDs em ordem crescente, decrescente ou aleatória
- [x] Estatísticas: tamanho, altura, limites teóricos, altura negra, rotações (esquerda/direita), recolorações e contagem de cada caso de balanceamento
- [x] Verificação das propriedades da árvore a qualquer momento
- [x] Cada inserção/remoção mostra o custo da operação (`rotações +N, recolorações +M`)

<br />

#### 🧭&nbsp; Exemplo de uso

```text
$ ./bin/arvores --rb
> 1
  IDs para inserir (separados por espaço ou vírgula): 10 20 30 40 50 25
  ✔ 10 inserido (rotações +0, recolorações +1)
  ✔ 20 inserido (rotações +0, recolorações +0)
  ✔ 30 inserido (rotações +1, recolorações +2)
  ✔ 40 inserido (rotações +0, recolorações +4)
  ✔ 50 inserido (rotações +1, recolorações +2)
  ✔ 25 inserido (rotações +0, recolorações +3)
> 5
20 [N]
├── E: 10 [N]
└── D: 40 [R]
    ├── E: 30 [N]
    │   ├── E: 25 [R]
    │   └── D: ·
    └── D: 50 [N]
> 2
  IDs para remover (separados por espaço ou vírgula): 40 99
  ✔ 40 removido (rotações +1, recolorações +4)
  ✖ 99 não encontrado — árvore inalterada
```

A entrada também pode vir de um arquivo (`./bin/arvores --rb < entrada.txt`); nesse caso cada comando lido é ecoado, o que gera um registro completo da sessão. As sessões usadas na apresentação estão em `results/demo/`.

<br />

#### 🧠&nbsp; Decisões de implementação

| Situação | Decisão |
| --- | --- |
| ID repetido | Rejeitado: `insert` retorna `false`, a árvore e os contadores não mudam e o menu avisa |
| Remover ID inexistente | `remove` retorna `false`, a árvore e os contadores não mudam |
| Buscar ID inexistente | Informa "não encontrado" e o número de nós visitados |
| Nó com dois filhos | Substituído pelo sucessor em ordem (mínimo da subárvore direita) nas duas árvores |
| Altura | Contada em nós: árvore vazia = 0, só a raiz = 1 |
| Variante da Rubro-Negra | Convencional (CLRS), não a LLRB: ligações rubras podem ficar dos dois lados; no máximo 2 rotações na inserção e 3 na remoção |
| Folhas da Rubro-Negra | Sentinela `NIL` negra (preta) compartilhada |
| Entrada inválida no menu | O token é ignorado com aviso; o programa nunca encerra por erro de digitação |
| Contagem de rotações | Rotação dupla conta como duas rotações simples |

**AVL** (`src/AVLTree.cpp`): recursiva; cada nó guarda sua altura. Na volta da recursão, `rebalance()` recalcula a altura e, se `|fb| = 2`, aplica um dos casos com a nomenclatura da disciplina: LL (rotação simples à direita), RR (simples à esquerda), LR (dupla à direita: esquerda no filho e direita no nó) ou RL (dupla à esquerda: direita no filho e esquerda no nó). Convenções iguais às da aula: `fb = h(esq) − h(dir)` e `h = max(h_esq, h_dir) + 1`, com folha de altura 1.

**Rubro-Negra** (`src/RBTree.cpp`): iterativa, com ponteiro para o pai e sentinela. `insertFixup()` trata os casos tio rubro, triângulo e linha; `removeFixup()` trata os quatro casos clássicos da remoção. A variante LLRB (caída para a esquerda) também foi vista em aula; a convencional foi escolhida por ser a usada em `std::map`, `java.util.TreeMap` e no kernel Linux, e porque, sem a regra de inclinação à esquerda, dispensa as rotações extras que a LLRB faz só para manter as ligações rubras à esquerda.

Os exemplos das aulas foram reproduzidos no programa com o mesmo resultado: a inserção de `{1,2,3,10,4,5,9,7,8,6}` na AVL e o `RemoverAVL(8,T)`, que gera uma rotação LL seguida de uma RR.

**Validação**: `validate()` confere ordem de busca, `|fb| ≤ 1` e alturas armazenadas (AVL); raiz negra, ausência de rubro com filho rubro, mesma altura negra em todos os caminhos e consistência dos ponteiros pai (Rubro-Negra).

<br />

#### 🗂️&nbsp; Estrutura

```text
include/   BalancedTree.hpp (interface comum), AVLTree.hpp, RBTree.hpp, TreePrinter.hpp, Console.hpp, Types.hpp
src/       AVLTree.cpp, RBTree.cpp, BalancedTree.cpp, Console.cpp, main.cpp (menu), compare.cpp (análise)
tests/     tests.cpp (suíte automatizada)
bin/       executáveis compilados: arvores, testes, comparar
results/   CSV e saídas usados na apresentação
```

<br />

#### ✅&nbsp; Testes

`bin/testes` executa 12 cenários para cada árvore, com a mesma entrada, validando as propriedades após cada operação — **30.314 verificações, 0 falhas** (`results/saida_testes.txt`):

- árvore vazia e com um único elemento;
- valores repetidos;
- inserção crescente, decrescente e não ordenada (1.000 IDs);
- busca de existentes e inexistentes;
- remoção de folha, de nó com 1 filho, com 2 filhos e da raiz;
- remoção de inexistentes e de todos os elementos;
- 6.000 operações aleatórias comparadas com `std::set`;
- casos dirigidos: rotação simples e dupla na AVL, caso linha e caso tio rubro na Rubro-Negra.

<br />

#### 📊&nbsp; Resultados da análise (`bin/comparar`, semente 2026)

Altura após inserir 100.000 IDs (mínimo possível: 17):

| Ordem | AVL | Rubro-Negra |
| --- | ---: | ---: |
| Crescente | 17 | 31 |
| Decrescente | 17 | 31 |
| Aleatória | 20 | 20 |

Simulação de cargas com IDs sequenciais (mesma sequência de operações nas duas):

| Carga | Métrica | AVL | Rubro-Negra |
| --- | --- | ---: | ---: |
| Black Friday (45% ins., 40% busca, 15% rem.) | altura · nós por busca | 18 · 14,49 | 27 · 15,34 |
| | rotações · escritas de metadados | 143.683 · 3,08 mi | 146.115 · 0,79 mi |
| Rastreamento (8% ins., 90% busca, 2% rem.) | altura · nós por busca | 17 · 15,92 | 30 · 17,59 |
| | rotações · escritas de metadados | 124.254 · 2,32 mi | 124.332 · 0,64 mi |

Em 7 execuções, a Rubro-Negra foi de 4% a 30% mais rápida na carga de escrita e a AVL de 7% a 20% mais rápida na carga de leitura. Os tempos variam entre máquinas; as demais métricas são determinísticas. Saída completa em `results/saida_comparacao.txt`.

**Recomendação:** Rubro-Negra para o registro e a baixa de encomendas durante a Black Friday (carga dominada por escrita); AVL para um índice de rastreamento dominado por consultas. Os dados confirmam a orientação vista em aula: quando a busca predomina, AVL; quando inserção e remoção predominam, Rubro-Negra.

<br />

#### 🎨&nbsp; Apresentação

A apresentação técnica (18 slides, exportada em PDF) usa a paleta da Amazon: laranja `#FF9900`, azul-marinho `#232F3E`, azul `#146EB4`, dourado-claro `#FEBD69`, branco e preto.

<br>

## 🧑🏻&nbsp; Autor

<p align="center">
  <img width="20%" src="https://github.com/aristofany-herderson.png" alt="aristofany-herderson">
  <p align="center">
    Aristofany Herderson
  </p>
  <p align="center">
    <a  href="https://www.linkedin.com/in/aristofany-herderson/" target="_blank">
    <img align="center" src="https://img.shields.io/badge/LINKEDIN-000000.svg?style=for-the-badge&labelColor=0a66c2&logo=inspire&logoColor=fff&logoWidth=20" alt="linkedin"/>
    </a>
    <a href="https://www.instagram.com/aristo.dev/" target="_blank">
      <img align="center" src="https://img.shields.io/badge/INSTAGRAM-000000.svg?style=for-the-badge&labelColor=dd326f&logo=instagram&logoColor=fff&logoWidth=20" alt="instagram"/>
    </a>
  </p>
</p>
