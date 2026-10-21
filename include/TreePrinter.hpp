#pragma once

#include <ostream>
#include <string>

// Desenha a árvore
namespace tree_printer {

template <class Node, class Label>
void printChildren(std::ostream &os, const Node *n, const Node *nil,
                   const std::string &prefix, const Label &label) {
  if (n->left == nil && n->right == nil)
    return;
  const Node *kids[2] = {n->left, n->right};
  const char *tags[2] = {"E", "D"};
  for (int i = 0; i < 2; ++i) {
    const bool last = i == 1;
    os << prefix << (last ? "└── " : "├── ") << tags[i] << ": ";
    if (kids[i] == nil) {
      os << "·\n";
      continue;
    }
    os << label(kids[i]) << '\n';
    printChildren(os, kids[i], nil, prefix + (last ? "    " : "│   "), label);
  }
}

template <class Node, class Label>
void print(std::ostream &os, const Node *root, const Node *nil,
           const Label &label) {
  if (root == nil) {
    os << "(árvore vazia)\n";
    return;
  }
  os << label(root) << '\n';
  printChildren(os, root, nil, "", label);
}

} // namespace tree_printer
