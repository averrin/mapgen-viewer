#ifndef _RBTREE_H_
#define _RBTREE_H_

#include "MemoryPool/C-11/MemoryPool.h"
//#include "MemoryPool/C-98/MemoryPool.h" //You will need to use this version instead of the one above if your compiler doesn't handle C++11's noexcept operator
#include <iostream>

template <typename T>
struct treeNode {
	T data;
	treeNode<T>* left;
	treeNode<T>* right;
	treeNode<T>* parent;
	bool red;

	//cached for speed
	treeNode<T>* prev;
	treeNode<T>* next;

	treeNode() : left(NULL), right(NULL), parent(NULL), 
					red(false), prev(NULL), next(NULL) {};
};

template <typename T>
class RBTree {
public:
	RBTree() : root(NULL) {};
	~RBTree() {};

	treeNode<T>* insertSuccessor(treeNode<T>* node, T& successorData);
	void removeNode(treeNode<T>* node);
	inline treeNode<T>* getFirst(treeNode<T>* node);
	inline treeNode<T>* getLast(treeNode<T>* node);

	treeNode<T>* getRoot() { return root; };

	void print();
private:
	treeNode<T>* root;
	MemoryPool<treeNode<T>> nodePool;

	void rotateLeft(treeNode<T>* node);
	void rotateRight(treeNode<T>* node);
};

template <typename T>
treeNode<T>* RBTree<T>::insertSuccessor(treeNode<T>* node, T& successorData) {
	treeNode<T>* successor = nodePool.newElement(treeNode<T>());
	successor->data = successorData;

	treeNode<T>* parent = NULL;

	if (node) {
		successor->prev = node;
		successor->next = node->next;
		if (node->next) {
			node->next->prev = successor;
		}
		node->next = successor;
		if (node->right) {
			node = getFirst(node->right);
			node->left = successor;
		}
		else {
			node->right = successor;
		}
		parent = node;
	}
	// if node is null, successor must be inserted
	// to the left-most part of the tree
	else if (root) {
		node = getFirst(root);
		successor->prev = NULL;
		successor->next = node;
		node->prev = successor;
		node->left = successor;
		parent = node;
	}
	else {
		successor->prev = successor->next = NULL;
		root = successor;
		parent = NULL;
	}
	successor->left = successor->right = NULL;
	successor->parent = parent;
	successor->red = true;

	// Fixup the modified tree by recoloring nodes and performing rotations 
	// (2 at most) so the red-black tree properties are preserved.
	treeNode<T>* grandma;
	treeNode<T>* aunt;
	node = successor;
	while (parent && parent->red) {
		grandma = parent->parent;
		if (parent == grandma->left) {
			aunt = grandma->right;
			if (aunt && aunt->red) {
				parent->red = aunt->red = false;
				grandma->red = true;
				node = grandma;
			}
			else {
				if (node == parent->right) {
					rotateLeft(parent);
					node = parent;
					parent = node->parent;
				}
				parent->red = false;
				grandma->red = true;
				rotateRight(grandma);
			}
		}
		else {
			aunt = grandma->left;
			if (aunt && aunt->red) {
				parent->red = aunt->red = false;
				grandma->red = true;
				node = grandma;
			}
			else {
				if (node == parent->left) {
					rotateRight(parent);
					node = parent;
					parent = node->parent;
				}
				parent->red = false;
				grandma->red = true;
				rotateLeft(grandma);
			}
		}
		parent = node->parent;
	}
	root->red = false;
	return successor;
}

template<typename T>
void RBTree<T>::removeNode(treeNode<T>* node) {
	if (node->next) {
		node->next->prev = node->prev;
	}
	if (node->prev) {
		node->prev->next = node->next;
	}

	treeNode<T>* original = node;
	treeNode<T>* parent = node->parent;
	treeNode<T>* left = node->left;
	treeNode<T>* right = node->right;
	treeNode<T>* next = NULL;

	if (!left) {
		next = right;
	}
	else if (!right) {
		next = left;
	}
	else {
		next = getFirst(right);
	}
	if (parent) {
		if (parent->left == node) {
			parent->left = next;
		}
		else {
			parent->right = next;
		}
	}
	else {
		root = next;
	}
	//enforce red-black rules
	bool red;
	if (left && right) {
		red = next->red;
		next->red = node->red;
		next->left = left;
		left->parent = next;
		if (next != right) {
			parent = next->parent;
			next->parent = node->parent;
			node = next->right;
			parent->left = node;
			next->right = right;
			right->parent = next;
		}
		else {
			next->parent = parent;
			parent = next;
			node = next->right;
		}
	}
	else {
		red = node->red;
		node = next;
	}
	// 'node' is now the sole successor's child and 'parent' its
	// new parent (since the successor can have been moved)
	if (node) {
		node->parent = parent;
	}
	// the 'easy' cases
	if (red) {
		nodePool.deleteElement(original);
		return;
	}
	if (node && node->red) {
		node->red = false;
		nodePool.deleteElement(original);
		return;
	}
	// the other cases
	treeNode<T>* sibling = NULL;
	do {
		if (node == root) {
			break;
		}
		if (node == parent->left) {
			sibling = parent->right;
			if (sibling->red) {
				sibling->red = false;
				parent->red = true;
				rotateLeft(parent);
				sibling = parent->right;
			}
			if ((sibling->left && sibling->left->red) || (sibling->right && sibling->right->red)) {
				if (!sibling->right || !sibling->right->red) {
					sibling->left->red = false;
					sibling->red = true;
					rotateRight(sibling);
					sibling = parent->right;
				}
				sibling->red = parent->red;
				parent->red = sibling->right->red = false;
				rotateLeft(parent);
				node = root;
				break;
			}
		}
		else {
			sibling = parent->left;
			if (sibling->red) {
				sibling->red = false;
				parent->red = true;
				rotateRight(parent);
				sibling = parent->left;
			}
			if ((sibling->left && sibling->left->red) || (sibling->right && sibling->right->red)) {
				if (!sibling->left || !sibling->left->red) {
					sibling->right->red = false;
					sibling->red = true;
					rotateLeft(sibling);
					sibling = parent->left;
				}
				sibling->red = parent->red;
				parent->red = sibling->left->red = false;
				rotateRight(parent);
				node = root;
				break;
			}
		}
		sibling->red = true;
		node = parent;
		parent = parent->parent;
	} while (!node->red);
	if (node) {
		node->red = false;
	}

	nodePool.deleteElement(original);
}

template<typename T>
void RBTree<T>::rotateLeft(treeNode<T>* node) {
	treeNode<T>* p = node;
	treeNode<T>* q = node->right; //can't be null
	treeNode<T>* parent = p->parent;

	if (parent) {
		if (parent->left == p) {
			parent->left = q;
		}
		else {
			parent->right = q;
		}
	}
	else {
		root = q;
	}
	q->parent = parent;
	p->parent = q;
	p->right = q->left;
	if (p->right) {
		p->right->parent = p;
	}
	q->left = p;
}

template<typename T>
void RBTree<T>::rotateRight(treeNode<T>* node) {
	treeNode<T>* p = node;
	treeNode<T>* q = node->left; //can't be null
	treeNode<T>* parent = p->parent;

	if (parent) {
		if (parent->left == p) {
			parent->left = q;
		}
		else {
			parent->right = q;
		}
	}
	else {
		root = q;
	}
	q->parent = parent;
	p->parent = q;
	p->left = q->right;
	if (p->left) {
		p->left->parent = p;
	}
	q->right = p;
}

template<typename T>
inline treeNode<T>* RBTree<T>::getFirst(treeNode<T>* node) {
	while (node->left) {
		node = node->left;
	}
	return node;
}

template<typename T>
inline treeNode<T>* RBTree<T>::getLast(treeNode<T>* node) {
	while (node->right) {
		node = node->right;
	}
	return node;
}

template<typename T>
void RBTree<T>::print() {
	treeNode<T>* node = getFirst(root);

	while (node) {
		std::cout << node->data << std::endl;
		node = node->next;
	}
}

#endif