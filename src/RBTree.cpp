#include "RBTree.hpp"

#include <algorithm>
#include <ostream>

#include "Console.hpp"
#include "TreePrinter.hpp"

namespace {
const std::string kInsertUncleRed =
    "Inserção caso 1 - tio vermelho (recoloração)";
const std::string kInsertTriangle =
    "Inserção caso 2 - triângulo (rotação no pai)";
const std::string kInsertLine = "Inserção caso 3 - linha (rotação no avô)";
const std::string kRemoveSiblingRed = "Remoção caso 1 - irmão vermelho";
const std::string kRemoveNephewsBlack =
    "Remoção caso 2 - irmão e sobrinhos negros";
const std::string kRemoveNearNephew =
    "Remoção caso 3 - sobrinho próximo vermelho";
const std::string kRemoveFarNephew =
    "Remoção caso 4 - sobrinho distante vermelho";

bool fail(std::string *error, const std::string &message) {
  if (error)
    *error = message;
  return false;
}

int failHeight(std::string *error, const std::string &message) {
  fail(error, message);
  return -1;
}
} // namespace

RBTree::RBTree()
    : nil_(new Node{0, Color::Black, nullptr, nullptr, nullptr}), root_(nil_) {
  nil_->left = nil_->right = nil_->parent = nil_;
}

RBTree::~RBTree() {
  destroy(root_);
  delete nil_;
}

void RBTree::destroy(Node *n) {
  if (n == nil_)
    return;
  destroy(n->left);
  destroy(n->right);
  delete n;
}

void RBTree::clear() {
  destroy(root_);
  root_ = nil_;
  nil_->parent = nil_;
  size_ = 0;
}

void RBTree::setColor(Node *n, Color c) {
  if (n->color == c)
    return;
  n->color = c;
  ++stats_.recolors;
}

void RBTree::rotateLeft(Node *x) {
  Node *y = x->right;
  x->right = y->left;
  if (y->left != nil_)
    y->left->parent = x;
  y->parent = x->parent;
  if (x->parent == nil_)
    root_ = y;
  else if (x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;
  y->left = x;
  x->parent = y;
  ++stats_.leftRotations;
}

void RBTree::rotateRight(Node *y) {
  Node *x = y->left;
  y->left = x->right;
  if (x->right != nil_)
    x->right->parent = y;
  x->parent = y->parent;
  if (y->parent == nil_)
    root_ = x;
  else if (y == y->parent->right)
    y->parent->right = x;
  else
    y->parent->left = x;
  x->right = y;
  y->parent = x;
  ++stats_.rightRotations;
}

bool RBTree::insert(Key key) {
  Node *parent = nil_;
  Node *cur = root_;
  while (cur != nil_) {
    parent = cur;
    if (key < cur->key)
      cur = cur->left;
    else if (key > cur->key)
      cur = cur->right;
    else
      return false;
  }
  Node *z = new Node{key, Color::Red, nil_, nil_, parent};
  if (parent == nil_)
    root_ = z;
  else if (key < parent->key)
    parent->left = z;
  else
    parent->right = z;
  ++size_;
  insertFixup(z);
  return true;
}

// Todo nó entra vermelho; o laço corrige a violação "vermelho com pai
// vermelho".
void RBTree::insertFixup(Node *z) {
  while (z->parent->color == Color::Red) {
    Node *p = z->parent;
    Node *g = p->parent;
    const bool parentIsLeft = p == g->left;
    Node *uncle = parentIsLeft ? g->right : g->left;

    if (uncle->color == Color::Red) {
      ++stats_.cases[kInsertUncleRed];
      setColor(p, Color::Black);
      setColor(uncle, Color::Black);
      setColor(g, Color::Red);
      z = g;
      continue;
    }
    if (z == (parentIsLeft ? p->right : p->left)) {
      ++stats_.cases[kInsertTriangle];
      z = p;
      if (parentIsLeft)
        rotateLeft(z);
      else
        rotateRight(z);
      p = z->parent;
    }
    ++stats_.cases[kInsertLine];
    setColor(p, Color::Black);
    setColor(g, Color::Red);
    if (parentIsLeft)
      rotateRight(g);
    else
      rotateLeft(g);
  }
  setColor(root_, Color::Black);
}

void RBTree::transplant(Node *u, Node *v) {
  if (u->parent == nil_)
    root_ = v;
  else if (u == u->parent->left)
    u->parent->left = v;
  else
    u->parent->right = v;
  v->parent = u->parent;
}

RBTree::Node *RBTree::minimum(Node *n) const {
  while (n->left != nil_)
    n = n->left;
  return n;
}

RBTree::Node *RBTree::findNode(Key key) const {
  Node *n = root_;
  while (n != nil_ && n->key != key)
    n = key < n->key ? n->left : n->right;
  return n;
}

// Nó com dois filhos é substituído pelo sucessor; se um nó negro sai da
// árvore, removeFixup redistribui o "negro extra" que ficou em x.
bool RBTree::remove(Key key) {
  Node *z = findNode(key);
  if (z == nil_)
    return false;

  Node *y = z;
  Color removedColor = y->color;
  Node *x = nil_;

  if (z->left == nil_) {
    x = z->right;
    transplant(z, z->right);
  } else if (z->right == nil_) {
    x = z->left;
    transplant(z, z->left);
  } else {
    y = minimum(z->right);
    removedColor = y->color;
    x = y->right;
    if (y->parent == z) {
      x->parent = y;
    } else {
      transplant(y, y->right);
      y->right = z->right;
      y->right->parent = y;
    }
    transplant(z, y);
    y->left = z->left;
    y->left->parent = y;
    setColor(y, z->color);
  }
  delete z;
  --size_;
  if (removedColor == Color::Black)
    removeFixup(x);
  nil_->parent = nil_;
  return true;
}

void RBTree::removeFixup(Node *x) {
  while (x != root_ && x->color == Color::Black) {
    const bool isLeft = x == x->parent->left;
    Node *w = isLeft ? x->parent->right : x->parent->left;

    if (w->color == Color::Red) {
      ++stats_.cases[kRemoveSiblingRed];
      setColor(w, Color::Black);
      setColor(x->parent, Color::Red);
      if (isLeft)
        rotateLeft(x->parent);
      else
        rotateRight(x->parent);
      w = isLeft ? x->parent->right : x->parent->left;
    }
    Node *near = isLeft ? w->left : w->right;
    Node *far = isLeft ? w->right : w->left;

    if (near->color == Color::Black && far->color == Color::Black) {
      ++stats_.cases[kRemoveNephewsBlack];
      setColor(w, Color::Red);
      x = x->parent;
      continue;
    }
    if (far->color == Color::Black) {
      ++stats_.cases[kRemoveNearNephew];
      setColor(near, Color::Black);
      setColor(w, Color::Red);
      if (isLeft)
        rotateRight(w);
      else
        rotateLeft(w);
      w = isLeft ? x->parent->right : x->parent->left;
      far = isLeft ? w->right : w->left;
    }
    ++stats_.cases[kRemoveFarNephew];
    setColor(w, x->parent->color);
    setColor(x->parent, Color::Black);
    setColor(far, Color::Black);
    if (isLeft)
      rotateLeft(x->parent);
    else
      rotateRight(x->parent);
    x = root_;
  }
  setColor(x, Color::Black);
}

SearchResult RBTree::find(Key key) const {
  SearchResult result;
  for (const Node *n = root_; n != nil_;
       n = key < n->key ? n->left : n->right) {
    ++result.visited;
    if (n->key == key) {
      result.found = true;
      break;
    }
  }
  return result;
}

std::vector<Key> RBTree::inorder() const {
  std::vector<Key> keys;
  keys.reserve(size_);
  std::vector<const Node *> stack;
  const Node *n = root_;
  while (n != nil_ || !stack.empty()) {
    while (n != nil_) {
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

void RBTree::print(std::ostream &os) const {
  tree_printer::print(os, static_cast<const Node *>(root_),
                      static_cast<const Node *>(nil_), [](const Node *n) {
                        const bool red = n->color == Color::Red;
                        const std::string text =
                            std::to_string(n->key) + (red ? " [R]" : " [N]");
                        return console::paint(text, red ? console::kRed
                                                        : console::kBold);
                      });
}

int RBTree::heightOf(const Node *n) const {
  if (n == nil_)
    return 0;
  return 1 + std::max(heightOf(n->left), heightOf(n->right));
}

int RBTree::blackHeight() const {
  int bh = 0;
  for (const Node *n = root_; n != nil_; n = n->left)
    if (n->color == Color::Black)
      ++bh;
  return bh;
}

std::optional<Key> RBTree::root() const {
  if (root_ == nil_)
    return std::nullopt;
  return root_->key;
}

int RBTree::childCount(Key key) const {
  const Node *n = findNode(key);
  if (n == nil_)
    return -1;
  return (n->left != nil_ ? 1 : 0) + (n->right != nil_ ? 1 : 0);
}

// Retorna a altura negra da subárvore (contando a sentinela) ou -1 em violação.
int RBTree::check(const Node *n, const Key *lo, const Key *hi,
                  std::size_t &count, std::string *error) const {
  if (n == nil_)
    return 1;
  const std::string id = std::to_string(n->key);
  if ((lo && n->key <= *lo) || (hi && n->key >= *hi))
    return failHeight(error, "ordem de busca violada no nó " + id);
  if ((n->left != nil_ && n->left->parent != n) ||
      (n->right != nil_ && n->right->parent != n))
    return failHeight(error, "ponteiro pai inválido sob o nó " + id);
  if (n->color == Color::Red &&
      (n->left->color == Color::Red || n->right->color == Color::Red))
    return failHeight(error, "nó vermelho " + id + " possui filho vermelho");

  const int bl = check(n->left, lo, &n->key, count, error);
  if (bl < 0)
    return -1;
  const int br = check(n->right, &n->key, hi, count, error);
  if (br < 0)
    return -1;
  if (bl != br)
    return failHeight(error, "alturas negras diferentes sob o nó " + id);
  ++count;
  return bl + (n->color == Color::Black ? 1 : 0);
}

bool RBTree::validate(std::string *error) const {
  if (nil_->color != Color::Black)
    return fail(error, "sentinela deixou de ser negra");
  if (root_->color != Color::Black)
    return fail(error, "a raiz não é negra");
  if (root_ != nil_ && root_->parent != nil_)
    return fail(error, "pai da raiz inválido");
  std::size_t count = 0;
  if (check(root_, nullptr, nullptr, count, error) < 0)
    return false;
  if (count != size_)
    return fail(error, "contador de tamanho inconsistente");
  return true;
}
