#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "BalancedTree.hpp"
#include "Console.hpp"
#include "RBTree.hpp"

namespace {

using console::paint;

constexpr std::size_t kMaxBatch = 1'000'000;
constexpr std::size_t kLargeTree = 256;

std::string trim(const std::string &s) {
  const auto begin = s.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos)
    return "";
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(begin, end - begin + 1);
}

// Lê uma linha; com entrada redirecionada (arquivo/pipe) ecoa o que foi lido
// para que a saída sirva como registro da execução.
bool readLine(const std::string &prompt, std::string &out) {
  std::cout << prompt << std::flush;
  if (!std::getline(std::cin, out))
    return false;
  out = trim(out);
  if (!console::stdinIsTerminal())
    std::cout << out << '\n';
  return true;
}

std::optional<long long> parseInteger(const std::string &token) {
  long long value = 0;
  const char *first = token.data();
  const char *last = first + token.size();
  auto [ptr, ec] = std::from_chars(first, last, value);
  if (ec != std::errc() || ptr != last)
    return std::nullopt;
  return value;
}

std::vector<Key> parseKeys(std::string line,
                           std::vector<std::string> &invalid) {
  std::replace(line.begin(), line.end(), ',', ' ');
  std::replace(line.begin(), line.end(), ';', ' ');
  std::istringstream in(line);
  std::vector<Key> keys;
  std::string token;
  while (in >> token) {
    if (auto value = parseInteger(token))
      keys.push_back(*value);
    else
      invalid.push_back(token);
  }
  return keys;
}

std::optional<std::vector<Key>> askKeys(const std::string &action) {
  std::string line;
  if (!readLine("  IDs para " + action + " (separados por espaço ou vírgula): ",
                line))
    return std::nullopt;
  std::vector<std::string> invalid;
  auto keys = parseKeys(line, invalid);
  for (const auto &token : invalid)
    std::cout << paint("  ! '" + token +
                           "' ignorado: não é um ID inteiro válido\n",
                       console::kYellow);
  if (keys.empty())
    std::cout << paint("  ! Nenhum ID informado.\n", console::kYellow);
  return keys;
}

std::string kindName(TreeKind kind) {
  return kind == TreeKind::AVL ? "AVL" : "Rubro-Negra";
}

std::optional<TreeKind> askKind() {
  std::cout << "\n"
            << paint("Escolha a estrutura de dados:", console::kBold) << "\n"
            << "  1  Árvore AVL\n"
            << "  2  Árvore Rubro-Negra\n";
  std::string line;
  while (readLine("> ", line)) {
    if (line == "1")
      return TreeKind::AVL;
    if (line == "2")
      return TreeKind::RedBlack;
    std::cout << paint("  ! Opção inválida, digite 1 ou 2.\n",
                       console::kYellow);
  }
  return std::nullopt;
}

std::string delta(const Stats &before, const Stats &after) {
  std::ostringstream out;
  const std::size_t rot = after.rotations() - before.rotations();
  const std::size_t rec = after.recolors - before.recolors;
  out << "rotações +" << rot;
  if (after.recolors || before.recolors)
    out << ", recolorações +" << rec;
  return paint("(" + out.str() + ")", console::kDim);
}

void doInsert(BalancedTree &tree) {
  auto keys = askKeys("inserir");
  if (!keys)
    return;
  for (Key key : *keys) {
    const Stats before = tree.stats();
    if (tree.insert(key))
      std::cout << paint("  ✔ ", console::kGreen) << key << " inserido "
                << delta(before, tree.stats()) << '\n';
    else
      std::cout << paint("  ✖ ", console::kYellow) << key
                << " já existe — duplicatas são ignoradas\n";
  }
}

void doRemove(BalancedTree &tree) {
  if (tree.empty()) {
    std::cout << paint("  ! A árvore está vazia, nada a remover.\n",
                       console::kYellow);
    return;
  }
  auto keys = askKeys("remover");
  if (!keys)
    return;
  for (Key key : *keys) {
    const Stats before = tree.stats();
    if (tree.remove(key))
      std::cout << paint("  ✔ ", console::kGreen) << key << " removido "
                << delta(before, tree.stats()) << '\n';
    else
      std::cout << paint("  ✖ ", console::kYellow) << key
                << " não encontrado — árvore inalterada\n";
  }
}

void doSearch(const BalancedTree &tree) {
  auto keys = askKeys("buscar");
  if (!keys)
    return;
  for (Key key : *keys) {
    const SearchResult r = tree.find(key);
    if (r.found)
      std::cout << paint("  ✔ ", console::kGreen) << key << " encontrado";
    else
      std::cout << paint("  ✖ ", console::kYellow) << key << " não encontrado";
    std::cout << paint(" (" + std::to_string(r.visited) + " nó(s) visitado(s))",
                       console::kDim)
              << '\n';
  }
}

void doInorder(const BalancedTree &tree) {
  const auto keys = tree.inorder();
  if (keys.empty()) {
    std::cout << "  (árvore vazia)\n";
    return;
  }
  std::cout << "  Em ordem (" << keys.size() << " elementos):\n  ";
  for (std::size_t i = 0; i < keys.size(); ++i) {
    std::cout << keys[i] << (i + 1 < keys.size() ? " " : "\n");
    if ((i + 1) % 15 == 0 && i + 1 < keys.size())
      std::cout << "\n  ";
  }
}

void doPrint(const BalancedTree &tree) {
  if (tree.size() > kLargeTree) {
    std::string line;
    if (!readLine("  A árvore tem " + std::to_string(tree.size()) +
                      " nós. Exibir mesmo assim? (s/n): ",
                  line) ||
        (line != "s" && line != "S"))
      return;
  }
  if (dynamic_cast<const RBTree *>(&tree))
    std::cout << paint("  Legenda: [R] = rubro (vermelho), [N] = negro (preto) · "
                       "E/D = filho esquerdo/direito\n\n",
                       console::kDim);
  else
    std::cout << paint("  Legenda: h = altura do nó, fb = h(esq) - h(dir) · "
                       "E/D = filho esquerdo/direito\n\n",
                       console::kDim);
  tree.print(std::cout);
}

void doBatch(BalancedTree &tree) {
  std::string line;
  if (!readLine("  Quantidade de IDs: ", line))
    return;
  const auto n = parseInteger(line);
  if (!n || *n <= 0 || static_cast<std::size_t>(*n) > kMaxBatch) {
    std::cout << paint("  ! Informe um número entre 1 e " +
                           std::to_string(kMaxBatch) + ".\n",
                       console::kYellow);
    return;
  }
  if (!readLine("  Ordem (1 crescente, 2 decrescente, 3 aleatória): ", line))
    return;
  const std::string order = line;
  if (order != "1" && order != "2" && order != "3") {
    std::cout << paint("  ! Ordem inválida.\n", console::kYellow);
    return;
  }
  if (!readLine("  ID inicial [1]: ", line))
    return;
  const Key start = line.empty() ? 1 : parseInteger(line).value_or(1);

  std::vector<Key> keys(static_cast<std::size_t>(*n));
  std::iota(keys.begin(), keys.end(), start);
  if (order == "2")
    std::reverse(keys.begin(), keys.end());
  if (order == "3")
    std::shuffle(keys.begin(), keys.end(), std::mt19937_64(2026));

  const Stats before = tree.stats();
  std::size_t inserted = 0;
  for (Key key : keys)
    inserted += tree.insert(key) ? 1 : 0;
  std::cout << paint("  ✔ ", console::kGreen) << inserted << " inserido(s), "
            << keys.size() - inserted << " duplicado(s) ignorado(s) "
            << delta(before, tree.stats()) << '\n';
}

void doStats(const BalancedTree &tree) {
  const Stats &s = tree.stats();
  const double n = static_cast<double>(tree.size());
  std::cout << std::fixed << std::setprecision(2) << "  Estrutura ............ "
            << tree.name() << '\n'
            << "  Elementos ............ " << tree.size() << '\n'
            << "  Altura ............... " << tree.height() << '\n'
            << "  Altura mínima teórica  "
            << (n > 0 ? static_cast<int>(std::ceil(std::log2(n + 1))) : 0)
            << '\n';
  if (const auto *rb = dynamic_cast<const RBTree *>(&tree))
    std::cout << "  Altura negra ......... " << rb->blackHeight() << '\n'
              << "  Limite teórico ....... " << 2 * std::log2(n + 1)
              << "  (2·log2(n+1))\n";
  else
    std::cout << "  Limite teórico ....... " << 1.44 * std::log2(n + 2)
              << "  (1,44·log2(n+2))\n";
  std::cout << "  Rotações ............. " << s.rotations() << " (esquerda "
            << s.leftRotations << ", direita " << s.rightRotations << ")\n";
  if (dynamic_cast<const RBTree *>(&tree))
    std::cout << "  Recolorações ......... " << s.recolors << '\n';
  if (!s.cases.empty()) {
    std::cout << "  Casos de balanceamento:\n";
    for (const auto &[name, count] : s.cases)
      std::cout << "    · " << name << ": " << count << '\n';
  }
}

void doValidate(const BalancedTree &tree) {
  std::string error;
  if (tree.validate(&error)) {
    std::cout << paint("  ✔ Todas as propriedades da árvore " + tree.name() +
                           " foram verificadas.\n",
                       console::kGreen);
    if (dynamic_cast<const RBTree *>(&tree))
      std::cout << paint("    raiz negra · sem vermelho com filho vermelho · "
                         "mesma altura negra · ordem BST\n",
                         console::kDim);
    else
      std::cout << paint(
          "    |fb| <= 1 em todos os nós · alturas consistentes · ordem BST\n",
          console::kDim);
  } else {
    std::cout << paint("  ✖ Violação encontrada: " + error + '\n',
                       console::kRed);
  }
}

void printMenu(const BalancedTree &tree) {
  std::cout << '\n'
            << paint("══ Amazon Logistics · Árvore " + tree.name() + " ══",
                     console::kBold)
            << paint("  n=" + std::to_string(tree.size()) +
                         "  h=" + std::to_string(tree.height()),
                     console::kDim)
            << "\n"
            << "  1  Inserir ID(s)              6  Gerar lote de IDs\n"
            << "  2  Remover ID(s)              7  Estatísticas\n"
            << "  3  Buscar ID(s)               8  Verificar propriedades\n"
            << "  4  Percurso em ordem          9  Esvaziar árvore\n"
            << "  5  Visualizar estrutura      10  Trocar tipo de árvore\n"
            << "  0  Sair\n";
}

void printUsage() {
  std::cout << "Uso: arvores [--avl | --rb] [--no-color]\n"
            << "  --avl       inicia com a árvore AVL\n"
            << "  --rb        inicia com a árvore Rubro-Negra\n"
            << "  --no-color  desativa as cores ANSI\n";
}

} // namespace

int main(int argc, char **argv) {
  console::setup();
  std::optional<TreeKind> kind;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--avl")
      kind = TreeKind::AVL;
    else if (arg == "--rb")
      kind = TreeKind::RedBlack;
    else if (arg == "--no-color")
      console::setColors(false);
    else if (arg == "--help" || arg == "-h")
      return printUsage(), 0;
    else
      return printUsage(), 1;
  }

  std::cout << paint("Amazon Engineering Challenge — Árvores Balanceadas (AVL "
                     "× Rubro-Negra)",
                     console::kBold)
            << '\n';
  if (!kind)
    kind = askKind();
  if (!kind)
    return 0;
  auto tree = makeTree(*kind);

  std::string option;
  while (true) {
    printMenu(*tree);
    if (!readLine("> ", option))
      break;
    if (option == "0")
      break;
    if (option == "1")
      doInsert(*tree);
    else if (option == "2")
      doRemove(*tree);
    else if (option == "3")
      doSearch(*tree);
    else if (option == "4")
      doInorder(*tree);
    else if (option == "5")
      doPrint(*tree);
    else if (option == "6")
      doBatch(*tree);
    else if (option == "7")
      doStats(*tree);
    else if (option == "8")
      doValidate(*tree);
    else if (option == "9") {
      tree->clear();
      tree->resetStats();
      std::cout << paint("  ✔ Árvore esvaziada e estatísticas zeradas.\n",
                         console::kGreen);
    } else if (option == "10") {
      const auto next = askKind();
      if (!next)
        break;
      tree = makeTree(*next);
      std::cout << paint("  ✔ Nova árvore " + kindName(*next) +
                             " criada (vazia).\n",
                         console::kGreen);
    } else {
      std::cout << paint("  ! Opção inválida.\n", console::kYellow);
    }
  }
  std::cout << "Até logo!\n";
  return 0;
}
