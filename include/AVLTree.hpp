#pragma once

#include "BalancedTree.hpp"

class AVLTree final : public BalancedTree {
public:
  AVLTree() = default;
  ~AVLTree() override;
  AVLTree(const AVLTree &) = delete;
  AVLTree &operator=(const AVLTree &) = delete;

  std::string name() const override { return "AVL"; }

  bool insert(Key key) override;
  bool remove(Key key) override;
  SearchResult find(Key key) const override;

  std::vector<Key> inorder() const override;
  void print(std::ostream &os) const override;

  int height() const override { return heightOf(root_); }
  std::size_t size() const override { return size_; }
  std::optional<Key> root() const override;
  int childCount(Key key) const override;

  bool validate(std::string *error = nullptr) const override;
  void clear() override;

private:
  struct Node {
    Key key;
    int height = 1;
    Node *left = nullptr;
    Node *right = nullptr;
    explicit Node(Key k) : key(k) {}
  };

  static int heightOf(const Node *n) { return n ? n->height : 0; }
  static int balanceOf(const Node *n) {
    return n ? heightOf(n->left) - heightOf(n->right) : 0;
  }
  void update(Node *n);
  static void destroy(Node *n);

  Node *rotateLeft(Node *x);
  Node *rotateRight(Node *y);
  Node *rebalance(Node *n);
  Node *insert(Node *n, Key key, bool &inserted);
  Node *remove(Node *n, Key key, bool &removed);
  const Node *findNode(Key key) const;

  bool check(const Node *n, const Key *lo, const Key *hi, int &height,
             std::size_t &count, std::string *error) const;

  Node *root_ = nullptr;
  std::size_t size_ = 0;
};
