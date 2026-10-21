#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Types.hpp"

// Interface comum: o menu, os testes e o comparador usam as duas árvores mas
// com essa interface
class BalancedTree {
public:
  virtual ~BalancedTree() = default;

  virtual std::string name() const = 0;

  // Retorna false quando a chave já existe (duplicatas).
  virtual bool insert(Key key) = 0;
  // Retorna false quando a chave não existe.
  virtual bool remove(Key key) = 0;
  virtual SearchResult find(Key key) const = 0;
  bool contains(Key key) const { return find(key).found; }

  virtual std::vector<Key> inorder() const = 0;
  virtual void print(std::ostream &os) const = 0;

  // Altura em número de nós: vazia = 0, apenas a raiz = 1.
  virtual int height() const = 0;
  virtual std::size_t size() const = 0;
  bool empty() const { return size() == 0; }

  virtual std::optional<Key> root() const = 0;
  // Quantidade de filhos (0, 1 ou 2) do nó com a chave; -1 se não existir.
  virtual int childCount(Key key) const = 0;

  virtual bool validate(std::string *error = nullptr) const = 0;
  virtual void clear() = 0;

  const Stats &stats() const { return stats_; }
  void resetStats() { stats_ = Stats{}; }

protected:
  Stats stats_;
};

enum class TreeKind { AVL, RedBlack };

std::unique_ptr<BalancedTree> makeTree(TreeKind kind);
