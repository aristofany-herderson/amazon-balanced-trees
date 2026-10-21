#pragma once

#include "BalancedTree.hpp"

class RBTree final : public BalancedTree {
public:
  RBTree();
  ~RBTree() override;
  RBTree(const RBTree &) = delete;
  RBTree &operator=(const RBTree &) = delete;

  std::string name() const override { return "Rubro-Negra"; }

  bool insert(Key key) override;
  bool remove(Key key) override;
  SearchResult find(Key key) const override;

  std::vector<Key> inorder() const override;
  void print(std::ostream &os) const override;

  int height() const override { return heightOf(root_); }
  std::size_t size() const override { return size_; }
  std::optional<Key> root() const override;
  int childCount(Key key) const override;

  // Quantidade de nós negros da raiz até uma folha (sem contar a sentinela).
  int blackHeight() const;

  bool validate(std::string *error = nullptr) const override;
  void clear() override;

private:
  enum class Color : unsigned char { Red, Black };

  struct Node {
    Key key;
    Color color;
    Node *left;
    Node *right;
    Node *parent;
  };

  int heightOf(const Node *n) const;
  void destroy(Node *n);
  void setColor(Node *n, Color c);

  void rotateLeft(Node *x);
  void rotateRight(Node *y);
  void insertFixup(Node *z);
  void transplant(Node *u, Node *v);
  void removeFixup(Node *x);
  Node *minimum(Node *n) const;
  Node *findNode(Key key) const;

  int check(const Node *n, const Key *lo, const Key *hi, std::size_t &count,
            std::string *error) const;

  Node *nil_; // sentinela negra compartilhada por todas as folhas (CLRS)
  Node *root_;
  std::size_t size_ = 0;
};
