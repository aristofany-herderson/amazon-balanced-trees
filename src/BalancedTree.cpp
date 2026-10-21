#include "BalancedTree.hpp"

#include "AVLTree.hpp"
#include "RBTree.hpp"

std::unique_ptr<BalancedTree> makeTree(TreeKind kind) {
  if (kind == TreeKind::AVL)
    return std::make_unique<AVLTree>();
  return std::make_unique<RBTree>();
}
