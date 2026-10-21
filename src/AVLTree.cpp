#include "AVLTree.hpp"

#include <algorithm>
#include <cstdlib>
#include <ostream>
#include <sstream>

#include "Console.hpp"
#include "TreePrinter.hpp"

namespace {
const std::string kCaseLL = "LL - rotação simples à direita";
const std::string kCaseRR = "RR - rotação simples à esquerda";
const std::string kCaseLR = "LR - rotação dupla à direita";
const std::string kCaseRL = "RL - rotação dupla à esquerda";

bool fail(std::string *error, const std::string &message) {
  if (error)
    *error = message;
  return false;
}
} // namespace

AVLTree::~AVLTree() { destroy(root_); }

void AVLTree::destroy(Node *n) {
  if (!n)
    return;
  destroy(n->left);
  destroy(n->right);
  delete n;
}

void AVLTree::clear() {
  destroy(root_);
  root_ = nullptr;
  size_ = 0;
}

void AVLTree::update(Node *n) {
  ++stats_.heightUpdates;
  n->height = 1 + std::max(heightOf(n->left), heightOf(n->right));
}

// x com filho direito y: y sobe, x vira filho esquerdo de y e herda y->left.
AVLTree::Node *AVLTree::rotateLeft(Node *x) {
  Node *y = x->right;
  x->right = y->left;
  y->left = x;
  update(x);
  update(y);
  ++stats_.leftRotations;
  return y;
}

AVLTree::Node *AVLTree::rotateRight(Node *y) {
  Node *x = y->left;
  y->left = x->right;
  x->right = y;
  update(y);
  update(x);
  ++stats_.rightRotations;
  return x;
}

// Fator de balanceamento = h(esq) - h(dir); fora de [-1, 1] exige rotação.
AVLTree::Node *AVLTree::rebalance(Node *n) {
  update(n);
  const int bf = balanceOf(n);
  if (bf > 1) {
    if (balanceOf(n->left) < 0) {
      ++stats_.cases[kCaseLR];
      n->left = rotateLeft(n->left);
    } else {
      ++stats_.cases[kCaseLL];
    }
    return rotateRight(n);
  }
  if (bf < -1) {
    if (balanceOf(n->right) > 0) {
      ++stats_.cases[kCaseRL];
      n->right = rotateRight(n->right);
    } else {
      ++stats_.cases[kCaseRR];
    }
    return rotateLeft(n);
  }
  return n;
}

bool AVLTree::insert(Key key) {
  bool inserted = false;
  root_ = insert(root_, key, inserted);
  if (inserted)
    ++size_;
  return inserted;
}

AVLTree::Node *AVLTree::insert(Node *n, Key key, bool &inserted) {
  if (!n) {
    inserted = true;
    return new Node(key);
  }
  if (key < n->key) {
    n->left = insert(n->left, key, inserted);
  } else if (key > n->key) {
    n->right = insert(n->right, key, inserted);
  } else {
    return n;
  }
  return inserted ? rebalance(n) : n;
}

bool AVLTree::remove(Key key) {
  bool removed = false;
  root_ = remove(root_, key, removed);
  if (removed)
    --size_;
  return removed;
}

// Nó com dois filhos: copia o sucessor (mínimo da subárvore direita) e o
// remove.
AVLTree::Node *AVLTree::remove(Node *n, Key key, bool &removed) {
  if (!n)
    return nullptr;
  if (key < n->key) {
    n->left = remove(n->left, key, removed);
  } else if (key > n->key) {
    n->right = remove(n->right, key, removed);
  } else {
    removed = true;
    if (!n->left || !n->right) {
      Node *child = n->left ? n->left : n->right;
      delete n;
      return child;
    }
    const Node *successor = n->right;
    while (successor->left)
      successor = successor->left;
    n->key = successor->key;
    bool ignored = false;
    n->right = remove(n->right, successor->key, ignored);
  }
  return removed ? rebalance(n) : n;
}

const AVLTree::Node *AVLTree::findNode(Key key) const {
  const Node *n = root_;
  while (n && n->key != key)
    n = key < n->key ? n->left : n->right;
  return n;
}

SearchResult AVLTree::find(Key key) const {
  SearchResult result;
  for (const Node *n = root_; n; n = key < n->key ? n->left : n->right) {
    ++result.visited;
    if (n->key == key) {
      result.found = true;
      break;
    }
  }
  return result;
}

std::vector<Key> AVLTree::inorder() const {
  std::vector<Key> keys;
  keys.reserve(size_);
  std::vector<const Node *> stack;
  const Node *n = root_;
  while (n || !stack.empty()) {
    while (n) {
      stack.push_back(n);
      n = n->left;
    }
    n = stack.back();
    stack.pop_back();
    keys.push_back(n->key);
    n = n->right;
  }
  return keys;
}

void AVLTree::print(std::ostream &os) const {
  tree_printer::print(
      os, static_cast<const Node *>(root_), static_cast<const Node *>(nullptr),
      [](const Node *n) {
        std::ostringstream label;
        const int bf = balanceOf(n);
        label << console::paint(std::to_string(n->key), console::kBold)
              << console::paint(" (h=" + std::to_string(n->height) +
                                    ", fb=" + (bf > 0 ? "+" : "") +
                                    std::to_string(bf) + ")",
                                console::kDim);
        return label.str();
      });
}

std::optional<Key> AVLTree::root() const {
  if (!root_)
    return std::nullopt;
  return root_->key;
}

int AVLTree::childCount(Key key) const {
  const Node *n = findNode(key);
  if (!n)
    return -1;
  return (n->left ? 1 : 0) + (n->right ? 1 : 0);
}

bool AVLTree::check(const Node *n, const Key *lo, const Key *hi, int &height,
                    std::size_t &count, std::string *error) const {
  if (!n) {
    height = 0;
    return true;
  }
  if ((lo && n->key <= *lo) || (hi && n->key >= *hi))
    return fail(error,
                "ordem de busca violada no nó " + std::to_string(n->key));
  int hl = 0;
  int hr = 0;
  if (!check(n->left, lo, &n->key, hl, count, error))
    return false;
  if (!check(n->right, &n->key, hi, hr, count, error))
    return false;
  ++count;
  height = 1 + std::max(hl, hr);
  if (n->height != height)
    return fail(error,
                "altura armazenada incorreta no nó " + std::to_string(n->key));
  if (std::abs(hl - hr) > 1)
    return fail(error, "fator de balanceamento fora de [-1, 1] no nó " +
                           std::to_string(n->key));
  return true;
}

bool AVLTree::validate(std::string *error) const {
  int h = 0;
  std::size_t count = 0;
  if (!check(root_, nullptr, nullptr, h, count, error))
    return false;
  if (count != size_)
    return fail(error, "contador de tamanho inconsistente");
  return true;
}
