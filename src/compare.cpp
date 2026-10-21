#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "BalancedTree.hpp"
#include "Console.hpp"

namespace {

using Clock = std::chrono::steady_clock;

constexpr std::size_t kMaxPlainBST = 10000;

// BST sem balanceamento, usada apenas para mostrar a degeneração.
class PlainBST {
public:
  ~PlainBST() {
    std::vector<Node *> stack{root_};
    while (!stack.empty()) {
      Node *n = stack.back();
      stack.pop_back();
      if (!n)
        continue;
      stack.push_back(n->left);
      stack.push_back(n->right);
      delete n;
    }
  }

  void insert(Key key) {
    Node **link = &root_;
    while (*link) {
      if (key == (*link)->key)
        return;
      link = key < (*link)->key ? &(*link)->left : &(*link)->right;
    }
    *link = new Node{key, nullptr, nullptr};
  }

  int height() const {
    int best = 0;
    std::vector<std::pair<const Node *, int>> stack{{root_, 1}};
    while (!stack.empty()) {
      auto [n, depth] = stack.back();
      stack.pop_back();
      if (!n)
        continue;
      best = std::max(best, depth);
      stack.push_back({n->left, depth + 1});
      stack.push_back({n->right, depth + 1});
    }
    return best;
  }

private:
  struct Node {
    Key key;
    Node *left;
    Node *right;
  };
  Node *root_ = nullptr;
};

struct Row {
  std::string scenario;
  std::size_t n = 0;
  std::string tree;
  int heightAfterInsert = 0;
  std::size_t rotationsInsert = 0;
  std::size_t recolorsInsert = 0;
  double avgVisitsHit = 0;
  double avgVisitsMiss = 0;
  int heightAfterRemove = 0;
  std::size_t rotationsRemove = 0;
  std::size_t recolorsRemove = 0;
  double insertMs = 0;
  double searchMs = 0;
  double removeMs = 0;
  bool valid = false;
};

double msSince(Clock::time_point start) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start)
      .count();
}

std::vector<Key> makeKeys(std::size_t n, const std::string &pattern,
                          std::uint32_t seed) {
  std::vector<Key> keys(n);
  for (std::size_t i = 0; i < n; ++i)
    keys[i] = 2 * static_cast<Key>(i + 1);
  if (pattern == "decrescente")
    std::reverse(keys.begin(), keys.end());
  if (pattern == "aleatória")
    std::shuffle(keys.begin(), keys.end(), std::mt19937(seed));
  return keys;
}

Row runScenario(TreeKind kind, const std::string &scenario,
                const std::vector<Key> &keys, const std::vector<Key> &toRemove,
                std::uint32_t seed) {
  auto tree = makeTree(kind);
  Row row;
  row.scenario = scenario;
  row.n = keys.size();
  row.tree = tree->name();

  auto start = Clock::now();
  for (Key k : keys)
    tree->insert(k);
  row.insertMs = msSince(start);
  row.heightAfterInsert = tree->height();
  row.rotationsInsert = tree->stats().rotations();
  row.recolorsInsert = tree->stats().recolors;
  row.valid = tree->validate();

  std::vector<Key> probes = keys;
  std::shuffle(probes.begin(), probes.end(), std::mt19937(seed + 1));
  long long hit = 0;
  long long miss = 0;
  start = Clock::now();
  for (Key k : probes)
    hit += tree->find(k).visited;
  for (Key k : probes)
    miss += tree->find(k - 1).visited;
  row.searchMs = msSince(start);
  row.avgVisitsHit = static_cast<double>(hit) / probes.size();
  row.avgVisitsMiss = static_cast<double>(miss) / probes.size();

  tree->resetStats();
  start = Clock::now();
  for (Key k : toRemove)
    tree->remove(k);
  row.removeMs = msSince(start);
  row.heightAfterRemove = tree->height();
  row.rotationsRemove = tree->stats().rotations();
  row.recolorsRemove = tree->stats().recolors;
  row.valid = row.valid && tree->validate();
  return row;
}

struct WorkloadRow {
  std::string tree;
  std::size_t operations = 0;
  std::size_t finalSize = 0;
  int height = 0;
  std::size_t rotations = 0;
  std::size_t recolors = 0;
  std::size_t heightUpdates = 0;
  double avgVisits = 0;
  double ms = 0;
  bool valid = false;
};

enum class Op { Insert, Search, Remove };

struct Profile {
  std::string name;
  std::string file;
  std::size_t prefill;
  double insertShare;
  double searchShare;
};

// IDs chegam em ordem crescente (gerados sequencialmente), clientes rastreiam
// pedidos e entregas concluídas removem preferencialmente pedidos antigos.
std::vector<std::pair<Op, Key>>
makeWorkload(const Profile &profile, std::size_t ops, std::uint32_t seed) {
  std::mt19937_64 rng(seed);
  std::uniform_real_distribution<double> coin(0.0, 1.0);
  std::vector<std::pair<Op, Key>> plan;
  plan.reserve(profile.prefill + ops);
  std::vector<Key> alive;
  Key nextId = 100000000;
  for (std::size_t i = 0; i < profile.prefill; ++i) {
    nextId += 1 + static_cast<Key>(rng() % 3);
    alive.push_back(nextId);
    plan.emplace_back(Op::Insert, nextId);
  }
  for (std::size_t i = 0; i < ops; ++i) {
    const double r = coin(rng);
    if (alive.empty() || r < profile.insertShare) {
      nextId += 1 + static_cast<Key>(rng() % 3);
      alive.push_back(nextId);
      plan.emplace_back(Op::Insert, nextId);
    } else if (r < profile.insertShare + profile.searchShare) {
      const bool existing = coin(rng) < 0.9;
      const Key k = existing ? alive[rng() % alive.size()]
                             : nextId + 1 + static_cast<Key>(rng() % 1000);
      plan.emplace_back(Op::Search, k);
    } else {
      const std::size_t window = std::max<std::size_t>(1, alive.size() / 4);
      const std::size_t idx = rng() % window;
      plan.emplace_back(Op::Remove, alive[idx]);
      alive[idx] = alive.back();
      alive.pop_back();
    }
  }
  return plan;
}

WorkloadRow runWorkload(TreeKind kind,
                        const std::vector<std::pair<Op, Key>> &plan) {
  auto tree = makeTree(kind);
  WorkloadRow row;
  row.tree = tree->name();
  row.operations = plan.size();
  long long visits = 0;
  long long searches = 0;
  const auto start = Clock::now();
  for (const auto &[op, key] : plan) {
    if (op == Op::Insert) {
      tree->insert(key);
    } else if (op == Op::Remove) {
      tree->remove(key);
    } else {
      visits += tree->find(key).visited;
      ++searches;
    }
  }
  row.ms = msSince(start);
  row.finalSize = tree->size();
  row.height = tree->height();
  row.rotations = tree->stats().rotations();
  row.recolors = tree->stats().recolors;
  row.heightUpdates = tree->stats().heightUpdates;
  row.avgVisits = searches ? static_cast<double>(visits) / searches : 0;
  row.valid = tree->validate();
  return row;
}

constexpr int kRepetitions = 5;

double median(std::vector<double> v) {
  std::sort(v.begin(), v.end());
  return v[v.size() / 2];
}

Row runScenarioMedian(TreeKind kind, const std::string &scenario,
                      const std::vector<Key> &keys,
                      const std::vector<Key> &toRemove, std::uint32_t seed) {
  std::vector<double> ins, search, rem;
  Row row;
  for (int i = 0; i < kRepetitions; ++i) {
    row = runScenario(kind, scenario, keys, toRemove, seed);
    ins.push_back(row.insertMs);
    search.push_back(row.searchMs);
    rem.push_back(row.removeMs);
  }
  row.insertMs = median(ins);
  row.searchMs = median(search);
  row.removeMs = median(rem);
  return row;
}

WorkloadRow runWorkloadMedian(TreeKind kind,
                              const std::vector<std::pair<Op, Key>> &plan) {
  std::vector<double> times;
  WorkloadRow row;
  for (int i = 0; i < kRepetitions; ++i) {
    row = runWorkload(kind, plan);
    times.push_back(row.ms);
  }
  row.ms = median(times);
  return row;
}

// std::setw conta bytes; nomes com acento (UTF-8) precisam de ajuste manual.
std::string pad(const std::string &text, std::size_t width) {
  std::size_t glyphs = 0;
  for (unsigned char c : text)
    glyphs += (c & 0xC0) != 0x80;
  return text + std::string(width > glyphs ? width - glyphs : 0, ' ');
}

std::string fmt(double v, int precision = 2) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(precision) << v;
  return out.str();
}

} // namespace

int main(int argc, char **argv) {
  console::setup();
  std::filesystem::path outDir = "results";
  std::uint32_t seed = 2026;
  std::vector<std::size_t> sizes = {1000, 10000, 100000};
  std::size_t workloadOps = 300000;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc)
      outDir = argv[++i];
    else if (arg == "--seed" && i + 1 < argc)
      seed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
    else if (arg == "--quick")
      sizes = {1000, 10000}, workloadOps = 50000;
    else {
      std::cout << "Uso: comparar [--out pasta] [--seed n] [--quick]\n";
      return 1;
    }
  }
  std::filesystem::create_directories(outDir);

  std::cout << console::paint("Análise comparativa — mesmos dados e mesma "
                              "sequência para as duas árvores\n",
                              console::kBold)
            << "semente = " << seed << " · tempos = mediana de " << kRepetitions
            << " execuções\n\n";

  std::vector<Row> rows;
  std::cout << console::paint(
      "Altura de uma BST sem balanceamento (referência do problema):\n",
      console::kBold);
  const std::vector<std::string> patterns = {"crescente", "decrescente",
                                             "aleatória"};
  for (std::size_t n : sizes) {
    for (const auto &pattern : patterns) {
      const auto keys = makeKeys(n, pattern, seed);
      std::cout << "  n=" << std::setw(6) << n << "  " << pad(pattern, 12);
      if (n <= kMaxPlainBST) {
        PlainBST bst;
        for (Key k : keys)
          bst.insert(k);
        std::cout << " altura BST = " << std::setw(6) << bst.height();
      } else {
        std::cout << " altura BST = (omitida: inserção O(n²))";
      }
      std::cout << "   mínimo possível = "
                << static_cast<int>(std::ceil(std::log2(n + 1.0))) << '\n';

      std::vector<Key> toRemove = keys;
      std::shuffle(toRemove.begin(), toRemove.end(), std::mt19937(seed + 7));
      toRemove.resize(n / 2);
      for (TreeKind kind : {TreeKind::AVL, TreeKind::RedBlack})
        rows.push_back(runScenarioMedian(kind, pattern, keys, toRemove, seed));
    }
  }

  std::cout << '\n'
            << console::paint(
                   "Inserção de n IDs pares, busca de todos (+ n ímpares "
                   "inexistentes) e remoção de 50% aleatórios:\n",
                   console::kBold)
            << "  cenário      n       árvore       h_ins  rot_ins  recol_ins  "
               "visitas(hit/miss)  h_rem  rot_rem  "
               "ins(ms)  válida\n";
  for (const auto &r : rows) {
    std::cout << "  " << pad(r.scenario, 12) << std::setw(6) << r.n << "  "
              << pad(r.tree, 12) << std::setw(6) << r.heightAfterInsert
              << std::setw(9) << r.rotationsInsert << std::setw(11)
              << r.recolorsInsert << std::setw(11) << fmt(r.avgVisitsHit)
              << " / " << std::setw(5) << fmt(r.avgVisitsMiss) << std::setw(7)
              << r.heightAfterRemove << std::setw(9) << r.rotationsRemove
              << std::setw(9) << fmt(r.insertMs, 1) << "  "
              << (r.valid ? "sim" : "NÃO") << '\n';
  }

  std::ofstream csv(outDir / "comparacao.csv");
  csv << "cenario,n,arvore,altura_insercao,rotacoes_insercao,recoloracoes_"
         "insercao,visitas_media_existente,"
         "visitas_media_inexistente,altura_apos_remocao,rotacoes_remocao,"
         "recoloracoes_remocao,tempo_insercao_ms,"
         "tempo_busca_ms,tempo_remocao_ms,valida\n";
  for (const auto &r : rows)
    csv << r.scenario << ',' << r.n << ',' << r.tree << ','
        << r.heightAfterInsert << ',' << r.rotationsInsert << ','
        << r.recolorsInsert << ',' << fmt(r.avgVisitsHit, 3) << ','
        << fmt(r.avgVisitsMiss, 3) << ',' << r.heightAfterRemove << ','
        << r.rotationsRemove << ',' << r.recolorsRemove << ','
        << fmt(r.insertMs, 3) << ',' << fmt(r.searchMs, 3) << ','
        << fmt(r.removeMs, 3) << ',' << (r.valid ? "sim" : "nao") << '\n';

  const std::vector<Profile> profiles = {
      {"Black Friday — pico de escrita", "black_friday.csv", 0, 0.45, 0.40},
      {"Rastreamento — pico de leitura", "rastreamento.csv", 100000, 0.08,
       0.90},
  };
  for (const auto &profile : profiles) {
    const auto plan = makeWorkload(profile, workloadOps, seed);
    std::size_t inserts = 0;
    std::size_t searches = 0;
    for (const auto &step : plan) {
      if (step.first == Op::Insert)
        ++inserts;
      else if (step.first == Op::Search)
        ++searches;
    }
    std::cout << '\n'
              << console::paint(
                     profile.name + " (" + std::to_string(plan.size()) +
                         " operações: " + std::to_string(inserts) +
                         " inserções, " + std::to_string(searches) +
                         " buscas, " +
                         std::to_string(plan.size() - inserts - searches) +
                         " remoções):\n",
                     console::kBold)
              << "  árvore        n_final  altura  rotações  recolorações  "
                 "alturas_recalc  visitas/busca  tempo(ms)"
                 "  válida\n";
    std::ofstream csvWorkload(outDir / profile.file);
    csvWorkload << "arvore,operacoes,tamanho_final,altura,rotacoes,"
                   "recoloracoes,alturas_recalculadas,"
                   "visitas_media_busca,tempo_ms,valida\n";
    for (TreeKind kind : {TreeKind::AVL, TreeKind::RedBlack}) {
      const auto w = runWorkloadMedian(kind, plan);
      std::cout << "  " << pad(w.tree, 12) << std::setw(9) << w.finalSize
                << std::setw(8) << w.height << std::setw(10) << w.rotations
                << std::setw(14) << w.recolors << std::setw(16)
                << w.heightUpdates << std::setw(15) << fmt(w.avgVisits)
                << std::setw(11) << fmt(w.ms, 1) << "  "
                << (w.valid ? "sim" : "NÃO") << '\n';
      csvWorkload << w.tree << ',' << w.operations << ',' << w.finalSize << ','
                  << w.height << ',' << w.rotations << ',' << w.recolors << ','
                  << w.heightUpdates << ',' << fmt(w.avgVisits, 3) << ','
                  << fmt(w.ms, 3) << ',' << (w.valid ? "sim" : "nao") << '\n';
    }
  }

  std::cout << "\nCSV gerados em " << outDir.string() << '\n';
  return 0;
}
