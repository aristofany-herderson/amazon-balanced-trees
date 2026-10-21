#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "BalancedTree.hpp"
#include "Console.hpp"

namespace {

int gChecks = 0;
int gFailures = 0;
std::string gCurrent;

#define CHECK(cond)                                                            \
  do {                                                                         \
    ++gChecks;                                                                 \
    if (!(cond)) {                                                             \
      ++gFailures;                                                             \
      std::cout << console::paint("    FALHOU ", console::kRed) << gCurrent    \
                << " — " << #cond << " (linha " << __LINE__ << ")\n";          \
    }                                                                          \
  } while (0)

#define CHECK_VALID(tree)                                                      \
  do {                                                                         \
    std::string error_;                                                        \
    ++gChecks;                                                                 \
    if (!(tree).validate(&error_)) {                                           \
      ++gFailures;                                                             \
      std::cout << console::paint("    FALHOU ", console::kRed) << gCurrent    \
                << " — " << error_ << " (linha " << __LINE__ << ")\n";         \
    }                                                                          \
  } while (0)

const TreeKind kKinds[] = {TreeKind::AVL, TreeKind::RedBlack};

bool isSorted(const std::vector<Key> &keys) {
  return std::is_sorted(keys.begin(), keys.end());
}

double heightBound(const BalancedTree &tree) {
  const double n = static_cast<double>(tree.size());
  return tree.name() == "AVL" ? 1.4405 * std::log2(n + 2) - 0.3277
                              : 2 * std::log2(n + 1);
}

std::vector<Key> sequence(int n, int pattern) {
  std::vector<Key> keys(n);
  std::iota(keys.begin(), keys.end(), 1);
  if (pattern == 1)
    std::reverse(keys.begin(), keys.end());
  if (pattern == 2)
    std::shuffle(keys.begin(), keys.end(), std::mt19937(42));
  return keys;
}

void insertAllValidating(BalancedTree &tree, const std::vector<Key> &keys) {
  for (Key k : keys) {
    CHECK(tree.insert(k));
    CHECK_VALID(tree);
  }
}

void testEmptyTree(BalancedTree &t) {
  CHECK(t.empty());
  CHECK(t.height() == 0);
  CHECK(!t.root().has_value());
  CHECK(!t.contains(10));
  CHECK(t.find(10).visited == 0);
  CHECK(!t.remove(10));
  CHECK(t.inorder().empty());
  CHECK(t.childCount(10) == -1);
  CHECK_VALID(t);
}

void testSingleElement(BalancedTree &t) {
  CHECK(t.insert(42));
  CHECK(t.size() == 1);
  CHECK(t.height() == 1);
  CHECK(t.root() == 42);
  CHECK(t.contains(42));
  CHECK(!t.contains(41));
  CHECK(!t.remove(7));
  CHECK(t.size() == 1);
  CHECK_VALID(t);
  CHECK(t.remove(42));
  CHECK(t.empty());
  CHECK(t.height() == 0);
  CHECK_VALID(t);
}

void testDuplicates(BalancedTree &t) {
  for (Key k : {5, 3, 8})
    CHECK(t.insert(k));
  const Stats before = t.stats();
  CHECK(!t.insert(5));
  CHECK(!t.insert(8));
  CHECK(t.size() == 3);
  CHECK(t.stats().rotations() == before.rotations());
  CHECK((t.inorder() == std::vector<Key>{3, 5, 8}));
  CHECK_VALID(t);
}

void testPattern(BalancedTree &t, int pattern) {
  const auto keys = sequence(1000, pattern);
  insertAllValidating(t, keys);
  CHECK(t.size() == 1000);
  CHECK(isSorted(t.inorder()));
  CHECK(t.inorder().size() == 1000);
  CHECK(t.height() <= heightBound(t));
  CHECK(t.height() >= 10);
}

void testSearch(BalancedTree &t) {
  insertAllValidating(t, sequence(200, 2));
  for (Key k = 1; k <= 200; ++k)
    CHECK(t.contains(k));
  for (Key k : {0, -5, 201, 1000, 99999})
    CHECK(!t.contains(k));
  CHECK(t.find(1).visited <= t.height());
  CHECK(t.find(500).visited <= t.height());
}

void testRemovalConfigurations(BalancedTree &t) {
  insertAllValidating(t, {50, 30, 70, 20, 40, 60, 80, 10, 45, 65});

  CHECK(t.childCount(10) == 0);
  CHECK(t.remove(10));
  CHECK(!t.contains(10));
  CHECK_VALID(t);

  const Key oneChild = t.childCount(60) == 1 ? 60 : 40;
  CHECK(t.childCount(oneChild) == 1);
  CHECK(t.remove(oneChild));
  CHECK(!t.contains(oneChild));
  CHECK_VALID(t);

  CHECK(t.childCount(30) == 2 || t.childCount(70) == 2);
  const Key twoChildren = t.childCount(30) == 2 ? 30 : 70;
  CHECK(t.remove(twoChildren));
  CHECK(!t.contains(twoChildren));
  CHECK_VALID(t);

  const Key oldRoot = *t.root();
  CHECK(t.remove(oldRoot));
  CHECK(!t.contains(oldRoot));
  CHECK(t.root() != oldRoot);
  CHECK_VALID(t);

  CHECK(t.size() == 6);
  CHECK(isSorted(t.inorder()));
}

void testRemoveMissing(BalancedTree &t) {
  insertAllValidating(t, {10, 20, 30});
  const auto before = t.inorder();
  const Stats stats = t.stats();
  CHECK(!t.remove(15));
  CHECK(!t.remove(99));
  CHECK(t.inorder() == before);
  CHECK(t.stats().rotations() == stats.rotations());
  CHECK_VALID(t);
}

void testRemoveEverything(BalancedTree &t) {
  insertAllValidating(t, sequence(500, 0));
  auto order = sequence(500, 2);
  for (Key k : order) {
    CHECK(t.remove(k));
    CHECK_VALID(t);
  }
  CHECK(t.empty());
  CHECK(t.height() == 0);
}

void testRandomAgainstStdSet(BalancedTree &t) {
  std::set<Key> oracle;
  std::mt19937 rng(7);
  std::uniform_int_distribution<int> op(0, 2);
  std::uniform_int_distribution<Key> key(1, 400);
  for (int i = 0; i < 6000; ++i) {
    const Key k = key(rng);
    switch (op(rng)) {
    case 0:
      CHECK(t.insert(k) == oracle.insert(k).second);
      break;
    case 1:
      CHECK(t.remove(k) == (oracle.erase(k) == 1));
      break;
    default:
      CHECK(t.contains(k) == (oracle.count(k) == 1));
      break;
    }
    if (i % 25 == 0)
      CHECK_VALID(t);
  }
  CHECK(t.size() == oracle.size());
  CHECK(t.inorder() == std::vector<Key>(oracle.begin(), oracle.end()));
  CHECK_VALID(t);
}

void testClear(BalancedTree &t) {
  insertAllValidating(t, sequence(100, 2));
  t.clear();
  CHECK(t.empty());
  CHECK_VALID(t);
  CHECK(t.insert(1));
  CHECK_VALID(t);
}

void testAvlRotations() {
  gCurrent = "AVL · rotação simples (LL)";
  auto t = makeTree(TreeKind::AVL);
  for (Key k : {30, 20, 10})
    t->insert(k);
  CHECK(t->root() == 20);
  CHECK(t->stats().rightRotations == 1);
  CHECK(t->stats().leftRotations == 0);

  gCurrent = "AVL · rotação dupla (LR)";
  t = makeTree(TreeKind::AVL);
  for (Key k : {30, 10, 20})
    t->insert(k);
  CHECK(t->root() == 20);
  CHECK(t->stats().rotations() == 2);
  CHECK(t->height() == 2);
}

void testRbFixups() {
  gCurrent = "Rubro-Negra · caso linha (rotação + recoloração)";
  auto t = makeTree(TreeKind::RedBlack);
  for (Key k : {10, 20, 30})
    t->insert(k);
  CHECK(t->root() == 20);
  CHECK(t->stats().leftRotations == 1);
  CHECK(t->stats().recolors >= 2);
  CHECK_VALID(*t);

  gCurrent = "Rubro-Negra · caso tio vermelho (só recoloração)";
  const std::size_t rotations = t->stats().rotations();
  t->insert(40);
  CHECK(t->stats().rotations() == rotations);
  CHECK_VALID(*t);
}

struct TestCase {
  const char *name;
  std::function<void(BalancedTree &)> run;
};

} // namespace

int main() {
  console::setup();
  const std::vector<TestCase> cases = {
      {"árvore vazia", testEmptyTree},
      {"um único elemento", testSingleElement},
      {"valores repetidos", testDuplicates},
      {"inserção crescente (1..1000)",
       [](BalancedTree &t) { testPattern(t, 0); }},
      {"inserção decrescente (1000..1)",
       [](BalancedTree &t) { testPattern(t, 1); }},
      {"inserção não ordenada (1000)",
       [](BalancedTree &t) { testPattern(t, 2); }},
      {"busca existente e inexistente", testSearch},
      {"remoção: folha, 1 filho, 2 filhos, raiz", testRemovalConfigurations},
      {"remoção de inexistentes", testRemoveMissing},
      {"remoção de todos os elementos", testRemoveEverything},
      {"6000 operações aleatórias × std::set", testRandomAgainstStdSet},
      {"esvaziar e reutilizar", testClear},
  };

  std::cout << console::paint("Testes automatizados — AVL e Rubro-Negra\n",
                              console::kBold);
  for (const auto &test : cases) {
    for (TreeKind kind : kKinds) {
      auto tree = makeTree(kind);
      gCurrent = tree->name() + " · " + test.name;
      const int failuresBefore = gFailures;
      test.run(*tree);
      std::cout << (gFailures == failuresBefore
                        ? console::paint("  ✔ ", console::kGreen)
                        : console::paint("  ✖ ", console::kRed))
                << gCurrent << '\n';
    }
  }
  for (auto test : {testAvlRotations, testRbFixups}) {
    const int failuresBefore = gFailures;
    test();
    std::cout << (gFailures == failuresBefore
                      ? console::paint("  ✔ ", console::kGreen)
                      : console::paint("  ✖ ", console::kRed))
              << gCurrent << '\n';
  }

  std::cout << '\n'
            << gChecks << " verificações, " << gFailures << " falha(s)\n";
  return gFailures == 0 ? 0 : 1;
}
