#ifndef INTERVALTREE_H_
#define INTERVALTREE_H_

#include <cmath>
#include <cstdlib>
#include <cassert>
#include <limits>
#include <algorithm>
#include <sstream>
#include <vector>
#include <string>

//  The interval_tree.h file contains code for 
//  interval trees implemented using red-black-trees as described in 
//  the book _Introduction_To_Algorithms_ by Cormen, Leisserson, 
//  and Rivest.  

// The low should return the lowest point of the interval and
// the high should return the highest point of the interval.  

template<typename T>
class IntervalTree {
public:
  enum color_t {RED, BLACK};

  class Node {
    friend class IntervalTree<T>;
  public:
    std::string str(Node *, Node *) const;
    Node();
    Node(const T&, int, int);
    virtual ~Node();
    int low() const;
    int high() const;
    T value() const;
  protected:
    T value_;
    int key;
    int high_;
    int maxHigh;
    color_t color;
    Node * left;
    Node * right;
    Node * parent;
  };

  struct it_recursion_node {
    /*  this structure stores the information needed when we take the */
    /*  right branch in searching for intervals but possibly come back */
    /*  and check the left branch as well. */
    it_recursion_node(Node *start_node_=NULL, 
        unsigned int parentIndex_=0, 
        int tryRightBranch_=0)
      : start_node (start_node_),
        parentIndex (parentIndex_),
        tryRightBranch (tryRightBranch_) {}

    Node * start_node;
    unsigned int parentIndex;
    int tryRightBranch;
  } ;

  IntervalTree();
  ~IntervalTree();
  std::string str() const;
  T remove(IntervalTree<T>::Node *);
  Node * insert(const T&, int, int);
  std::vector<T> fetch(int low, int high);

protected:
  Node * GetPredecessorOf(Node *) const;
  Node * GetSuccessorOf(Node *) const;
  void check() const;

  /*  A sentinel is used for root and for nil.  These sentinels are */
  /*  created when ITTreeCreate is caled.  root->left should always */
  /*  point to the node which is the root of the tree.  nil points to a */
  /*  node which should always be black but has aribtrary children and */
  /*  parent and no key or info.  The point of using these sentinels is so */
  /*  that the root and nil nodes do not require special cases in the code */
  Node * root;
  Node * nil;

  int Overlap(int a1, int a2, int b1, int b2);
  void LeftRotate(Node *);
  void RightRotate(Node *);
  void TreeInsertHelp(Node *);
  void TreePrintHelper(Node *, std::stringstream&) const;
  void FixUpMaxHigh(Node *);
  void DeleteFixUp(Node *);
  void CheckMaxHighFields(Node *) const;
  int CheckMaxHighFieldsHelper(Node * y, 
      const int currentHigh,
      int match) const;
private:
  std::vector<IntervalTree<T>::it_recursion_node> recursionNodeStack;
  unsigned int currentParent;
};

// If the symbol CHECK_INTERVAL_TREE_ASSUMPTIONS is defined then the
// code does a lot of extra checking to make sure certain assumptions
// are satisfied.  This only needs to be done if you suspect bugs are
// present or if you make significant changes and want to make sure
// your changes didn't mess anything up.
// #define CHECK_INTERVAL_TREE_ASSUMPTIONS 1

template<typename T> IntervalTree<T>::Node::Node(){}

template<typename T>
IntervalTree<T>::Node::Node(const T& value__, int lowPoint, int highPoint) 
  : value_ (value__),
    key(lowPoint), 
    high_(highPoint), 
    maxHigh(highPoint) {}

template<typename T>
IntervalTree<T>::Node::~Node(){}

template<typename T>
IntervalTree<T>::IntervalTree()
{
  nil = new IntervalTree<T>::Node();
  nil->left = nil->right = nil->parent = nil;
  nil->color = BLACK;
  nil->key = nil->high_ = nil->maxHigh = std::numeric_limits<int>::min();
  
  root = new IntervalTree<T>::Node();
  root->parent = root->left = root->right = nil;
  root->key = root->high_ = root->maxHigh = std::numeric_limits<int>::max();
  root->color=BLACK;

  /* the following are used for the fetch function */
  recursionNodeStack.push_back(IntervalTree<T>::it_recursion_node());
}

template<typename T>
int IntervalTree<T>::Node::low() const {
  return key;
}

template<typename T>
int IntervalTree<T>::Node::high() const {
  return high_;
}

template<typename T>
T IntervalTree<T>::Node::value() const {
  return value_;
}

/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  the node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: this, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. Also updates the maxHigh fields of x and y */
/*            after rotation. */
/***********************************************************************/

template<typename T>
void IntervalTree<T>::LeftRotate(IntervalTree<T>::Node* x) {
  IntervalTree<T>::Node* y;
 
  /*  I originally wrote this function to use the sentinel for */
  /*  nil to avoid checking for nil.  However this introduces a */
  /*  very subtle bug because sometimes this function modifies */
  /*  the parent pointer of nil.  This can be a problem if a */
  /*  function which calls LeftRotate also uses the nil sentinel */
  /*  and expects the nil sentinel's parent pointer to be unchanged */
  /*  after calling this function.  For example, when DeleteFixUP */
  /*  calls LeftRotate it expects the parent pointer of nil to be */
  /*  unchanged. */

  y=x->right;
  x->right=y->left;

  if (y->left != nil) y->left->parent=x; /* used to use sentinel here */
  /* and do an unconditional assignment instead of testing for nil */
  
  y->parent=x->parent;   

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  if( x == x->parent->left) {
    x->parent->left=y;
  } else {
    x->parent->right=y;
  }
  y->left=x;
  x->parent=y;

  x->maxHigh=std::max(x->left->maxHigh,std::max(x->right->maxHigh,x->high_));
  y->maxHigh=std::max(x->maxHigh,std::max(y->right->maxHigh,y->high_));
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
  check();
#elif defined(DEBUG_ASSERT)
  assert(nil->color == RED || !"nil not red in ITLeftRotate");
  assert((nil->maxHigh=std::numeric_limits<int>::min())
         || !"nil->maxHigh != std::numeric_limits<int>::min() in ITLeftRotate");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RighttRotate */
/**/
/*  INPUTS:  node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: this, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. Also updates the maxHigh fields of x and y */
/*            after rotation. */
/***********************************************************************/


template<typename T>
void IntervalTree<T>::RightRotate(IntervalTree<T>::Node* y) {
  IntervalTree<T>::Node* x;

  /*  I originally wrote this function to use the sentinel for */
  /*  nil to avoid checking for nil.  However this introduces a */
  /*  very subtle bug because sometimes this function modifies */
  /*  the parent pointer of nil.  This can be a problem if a */
  /*  function which calls LeftRotate also uses the nil sentinel */
  /*  and expects the nil sentinel's parent pointer to be unchanged */
  /*  after calling this function.  For example, when DeleteFixUP */
  /*  calls LeftRotate it expects the parent pointer of nil to be */
  /*  unchanged. */

  x=y->left;
  y->left=x->right;

  if (nil != x->right)  x->right->parent=y; /*used to use sentinel here */
  /* and do an unconditional assignment instead of testing for nil */

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  x->parent=y->parent;
  if( y == y->parent->left) {
    y->parent->left=x;
  } else {
    y->parent->right=x;
  }
  x->right=y;
  y->parent=x;

  y->maxHigh=std::max(y->left->maxHigh,std::max(y->right->maxHigh,y->high_));
  x->maxHigh=std::max(x->left->maxHigh,std::max(y->maxHigh,x->high_));
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
  check();
#elif defined(DEBUG_ASSERT)
  assert(nil->color == RED || !"nil not red in ITRightRotate");
  assert((nil->maxHigh=std::numeric_limits<int>::min())
        || !"nil->maxHigh != std::numeric_limits<int>::min() in ITRightRotate");
#endif
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the InsertTree function and not by the user */
/***********************************************************************/

template<typename T>
void IntervalTree<T>::TreeInsertHelp(IntervalTree<T>::Node* z) {
  /*  This function should only be called by InsertITTree (see above) */
  IntervalTree<T>::Node* x;
  IntervalTree<T>::Node* y;
    
  z->left=z->right=nil;
  y=root;
  x=root->left;
  while( x != nil) {
    y=x;
    if ( x->key > z->key) { 
      x=x->left;
    } else { /* x->key <= z->key */
      x=x->right;
    }
  }
  z->parent=y;
  if ( (y == root) ||
       (y->key > z->key) ) { 
    y->left=z;
  } else {
    y->right=z;
  }


#if defined(DEBUG_ASSERT)
  assert(nil->color == RED || !"nil not red in ITTreeInsertHelp");
  assert((nil->maxHigh=std::numeric_limits<int>::min())
        || !"nil->maxHigh != std::numeric_limits<int>::min() in ITTreeInsertHelp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  FixUpMaxHigh  */
/**/
/*  INPUTS:  x is the node to start from*/
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this */
/**/
/*  EFFECTS:  Travels up to the root fixing the maxHigh fields after */
/*            an insertion or deletion */
/***********************************************************************/

template<typename T>
void IntervalTree<T>::FixUpMaxHigh(IntervalTree<T>::Node * x) {
  while(x != root) {
    x->maxHigh=std::max(x->high_,std::max(x->left->maxHigh,x->right->maxHigh));
    x=x->parent;
  }
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
  check();
#endif
}

/*  Before calling InsertNode  the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  InsertNode */
/**/
/*  INPUTS:  newInterval is the interval to insert*/
/**/
/*  OUTPUT:  This function returns a pointer to the newly inserted node */
/*           which is guarunteed to be valid until this node is deleted. */
/*           What this means is if another data structure stores this */
/*           pointer then the tree does not need to be searched when this */
/*           is to be deleted. */
/**/
/*  Modifies Input: tree */
/**/
/*  EFFECTS:  Creates a node node which contains the appropriate key and */
/*            info pointers and inserts it into the tree. */
/***********************************************************************/

template <typename T>
typename IntervalTree<T>::Node* IntervalTree<T>::insert(const T& newInterval, int low, int high)
{
  IntervalTree<T>::Node * y;
  IntervalTree<T>::Node * x;
  IntervalTree<T>::Node * newNode;

  x = new IntervalTree<T>::Node(newInterval, low, high);
  TreeInsertHelp(x);
  FixUpMaxHigh(x->parent);
  newNode = x;
  x->color=RED;
  while(x->parent->color == RED) { /* use sentinel instead of checking for root */
    if (x->parent == x->parent->parent->left) {
      y=x->parent->parent->right;
      if (y->color == RED) {
        x->parent->color=BLACK;
        y->color=BLACK;
        x->parent->parent->color=RED;
        x=x->parent->parent;
      } else {
        if (x == x->parent->right) {
          x=x->parent;
          LeftRotate(x);
        }
        x->parent->color=BLACK;
        x->parent->parent->color=RED;
        RightRotate(x->parent->parent);
      } 
    } else { /* case for x->parent == x->parent->parent->right */
             /* this part is just like the section above with */
             /* left and right interchanged */
      y=x->parent->parent->left;
      if (y->color == RED) {
        x->parent->color=BLACK;
        y->color=BLACK;
        x->parent->parent->color=RED;
        x=x->parent->parent;
      } else {
        if (x == x->parent->left) {
          x=x->parent;
          RightRotate(x);
        }
        x->parent->color=BLACK;
        x->parent->parent->color=RED;
        LeftRotate(x->parent->parent);
      } 
    }
  }
  root->left->color=BLACK;
  return(newNode);

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
  check();
#elif defined(DEBUG_ASSERT)
  assert(nil->color == RED || !"nil not red in ITTreeInsert");
  assert(root->color == RED || !"root not red in ITTreeInsert");
  assert((nil->maxHigh=std::numeric_limits<int>::min())
         || !"nil->maxHigh != std::numeric_limits<int>::min() in ITTreeInsert");
#endif
}

/***********************************************************************/
/*  FUNCTION:  GetSuccessorOf  */
/**/
/*    INPUTS:  x is the node we want the succesor of */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/
  
template<typename T> 
typename IntervalTree<T>::Node * IntervalTree<T>::GetSuccessorOf(IntervalTree<T>::Node * x) const
{ 
  IntervalTree<T>::Node* y;

  if (nil != (y = x->right)) { /* assignment to y is intentional */
    while(y->left != nil) { /* returns the minium of the right subtree of x */
      y=y->left;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->right) { /* sentinel used instead of checking for nil */
      x=y;
      y=y->parent;
    }
    if (y == root) return(nil);
    return(y);
  }
}

/***********************************************************************/
/*  FUNCTION:  GetPredecessorOf  */
/**/
/*    INPUTS:  x is the node to get predecessor of */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

template<typename T>
typename IntervalTree<T>::Node * IntervalTree<T>::GetPredecessorOf(IntervalTree<T>::Node * x) const {
  IntervalTree<T>::Node* y;

  if (nil != (y = x->left)) { /* assignment to y is intentional */
    while(y->right != nil) { /* returns the maximum of the left subtree of x */
      y=y->right;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->left) { 
      if (y == root) return(nil); 
      x=y;
      y=y->parent;
    }
    return(y);
  }
}

/***********************************************************************/
/*  FUNCTION:  str */
/**/
/*    INPUTS:  none */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively prints the nodes of the tree */
/*              inorder. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:    This function should only be called from ITTreePrint */
/***********************************************************************/

template<typename T>
std::string IntervalTree<T>::Node::str(IntervalTree<T>::Node * nil,
                             IntervalTree<T>::Node * root) const {
  std::stringstream s;
  s << value;
  s << ", k=" << key << ", h=" << high_ << ", mH=" << maxHigh;
  s << "  l->key=";
  if( left == nil) s << "NULL"; else s << left->key;
  s << "  r->key=";
  if( right == nil) s << "NULL"; else s << right->key;
  s << "  p->key=";
  if( parent == root) s << "NULL"; else s << parent->key;
  s << "  color=" << (color == RED ? "RED" : "BLACK") << std::endl;
  return s.str();
}

template<typename T>
void IntervalTree<T>::TreePrintHelper(IntervalTree<T>::Node* x, std::stringstream &s) const {
  if (x != nil) {
    TreePrintHelper(x->left, s);
    s << x->str(nil,root);
    TreePrintHelper(x->right, s);
  }
}

template<typename T>
IntervalTree<T>::~IntervalTree() {
  IntervalTree<T>::Node * x = root->left;
  std::vector<IntervalTree<T>::Node *> stuffToFree;

  if (x != nil) {
    if (x->left != nil) {
      stuffToFree.push_back(x->left);
    }
    if (x->right != nil) {
      stuffToFree.push_back(x->right);
    }
    delete x;
    while( !stuffToFree.empty() ) {
      x = stuffToFree.back();
      stuffToFree.pop_back();
      if (x->left != nil) {
        stuffToFree.push_back(x->left);
      }
      if (x->right != nil) {
        stuffToFree.push_back(x->right);
      }
      delete x;
    }
  }
  delete nil;
  delete root;
}


/***********************************************************************/
/*  FUNCTION:  str */
/**/
/*    INPUTS:  none */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  This function recursively prints the nodes of the tree */
/*             inorder. */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/

template<typename T>
std::string IntervalTree<T>::str() const {
  std::stringstream s;
  return TreePrintHelper(root->left, s).str();
}

/***********************************************************************/
/*  FUNCTION:  DeleteFixUp */
/**/
/*    INPUTS:  x is the child of the spliced */
/*             out node in remove. */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Performs rotations and changes colors to restore red-black */
/*             properties after a node is deleted */
/**/
/*    Modifies Input: this, x */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

template<typename T>
void IntervalTree<T>::DeleteFixUp(IntervalTree<T>::Node* x) {
  IntervalTree<T>::Node * w;
  IntervalTree<T>::Node * rootLeft = root->left;

  while( (x->color == BLACK) && (rootLeft != x)) {
    if (x == x->parent->left) {
      w=x->parent->right;
      if (w->color == RED) {
        w->color=BLACK;
        x->parent->color=RED;
        LeftRotate(x->parent);
        w=x->parent->right;
      }
      if ( (w->right->color == BLACK) && (w->left->color == BLACK) ) { 
        w->color=RED;
        x=x->parent;
      } else {
        if (w->right->color == BLACK) {
          w->left->color=BLACK;
          w->color=RED;
          RightRotate(w);
          w=x->parent->right;
        }
        w->color=x->parent->color;
        x->parent->color=BLACK;
        w->right->color=BLACK;
        LeftRotate(x->parent);
        x=rootLeft; /* this is to exit while loop */
      }
    } else { /* the code below is has left and right switched from above */
      w=x->parent->left;
      if (w->color == RED) {
        w->color=BLACK;
        x->parent->color=RED;
        RightRotate(x->parent);
        w=x->parent->left;
      }
      if ( (w->right->color == BLACK) && (w->left->color == BLACK) ) { 
        w->color=RED;
        x=x->parent;
      } else {
        if (w->left->color == BLACK) {
          w->right->color=BLACK;
          w->color=RED;
          LeftRotate(w);
          w=x->parent->left;
        }
        w->color=x->parent->color;
        x->parent->color=BLACK;
        w->left->color=BLACK;
        RightRotate(x->parent);
        x=rootLeft; /* this is to exit while loop */
      }
    }
  }
  x->color=BLACK;

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
  check();
#elif defined(DEBUG_ASSERT)
  assert(nil->color == BLACK || !"nil not black in ITDeleteFixUp");
  assert((nil->maxHigh=std::numeric_limits<int>::min())
         || !"nil->maxHigh != std::numeric_limits<int>::min() in ITDeleteFixUp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  remove */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  returns the Interval stored at deleted node */
/**/
/*    EFFECT:  Deletes z from tree and but don't call destructor */
/*             Then calls FixUpMaxHigh to fix maxHigh fields then calls */
/*             ITDeleteFixUp to restore red-black properties */
/**/
/*    Modifies Input:  z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

template<typename T>
T IntervalTree<T>::remove(IntervalTree<T>::Node * z){
  IntervalTree<T>::Node* y;
  IntervalTree<T>::Node* x;
  const T& returnValue = z->value;

  y= ((z->left == nil) || (z->right == nil)) ? z : GetSuccessorOf(z);
  x= (y->left == nil) ? y->right : y->left;
  if (root == (x->parent = y->parent)) { /* assignment of y->p to x->p is intentional */
    root->left=x;
  } else {
    if (y == y->parent->left) {
      y->parent->left=x;
    } else {
      y->parent->right=x;
    }
  }
  if (y != z) { /* y should not be nil in this case */

#ifdef DEBUG_ASSERT
    assert( (y!=nil) || !"y is nil in remove");
#endif
    /* y is the node to splice out and x is its child */
  
    y->maxHigh = std::numeric_limits<int>::min();
    y->left=z->left;
    y->right=z->right;
    y->parent=z->parent;
    z->left->parent=z->right->parent=y;
    if (z == z->parent->left) {
      z->parent->left=y; 
    } else {
      z->parent->right=y;
    }
    FixUpMaxHigh(x->parent); 
    if (y->color == BLACK) {
      y->color = z->color;
      DeleteFixUp(x);
    } else
      y->color = z->color; 
    delete z;
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    check();
#elif defined(DEBUG_ASSERT)
    assert(nil->color == BLACK || !"nil not black in ITDelete");
    assert((nil->maxHigh=std::numeric_limits<int>::min())
        && !"nil->maxHigh != std::numeric_limits<int>::min() in ITDelete");
#endif
  } else {
    FixUpMaxHigh(x->parent);
    if (y->color == BLACK) DeleteFixUp(x);
    delete y;
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
    check();
#elif defined(DEBUG_ASSERT)
    assert(nil->color == BLACK || !"nil not black in ITDelete");
    assert((nil->maxHigh=std::numeric_limits<int>::min())
        || !"nil->maxHigh != std::numeric_limits<int>::min() in ITDelete");
#endif
  }
  return returnValue;
}


/***********************************************************************/
/*  FUNCTION:  Overlap */
/**/
/*    INPUTS:  [a1,a2] and [b1,b2] are the low and high endpoints of two */
/*             closed intervals.  */
/**/
/*    OUTPUT:  stack containing pointers to the nodes between [low,high] */
/**/
/*    Modifies Input: none */
/**/
/*    EFFECT:  returns 1 if the intervals overlap, and 0 otherwise */
/***********************************************************************/

template<typename T>
int IntervalTree<T>::Overlap(int a1, int a2, int b1, int b2) {
  if (a1 <= b1) {
    return( (b1 <= a2) );
  } else {
    return( (a1 <= b2) );
  }
}


/***********************************************************************/
/*  FUNCTION:  fetch */
/**/
/*    INPUTS:  tree is the tree to look for intervals overlapping the */
/*             closed interval [low,high]  */
/**/
/*    OUTPUT:  stack containing pointers to the nodes overlapping */
/*             [low,high] */
/**/
/*    Modifies Input: none */
/**/
/*    EFFECT:  Returns a stack containing pointers to nodes containing */
/*             intervals which overlap [low,high] in O(max(N,k*log(N))) */
/*             where N is the number of intervals in the tree and k is  */
/*             the number of overlapping intervals                      */
/**/
/*    Note:    This basic idea for this function comes from the  */
/*              _Introduction_To_Algorithms_ book by Cormen et al, but */
/*             modifications were made to return all overlapping intervals */
/*             instead of just the first overlapping interval as in the */
/*             book.  The natural way to do this would require recursive */
/*             calls of a basic search function.  I translated the */
/*             recursive version into an interative version with a stack */
/*             as described below. */
/***********************************************************************/



/*  The basic idea for the function below is to take the IntervalSearch */
/*  function from the book and modify to find all overlapping intervals */
/*  instead of just one.  This means that any time we take the left */
/*  branch down the tree we must also check the right branch if and only if */
/*  we find an overlapping interval in that left branch.  Note this is a */
/*  recursive condition because if we go left at the root then go left */
/*  again at the first left child and find an overlap in the left subtree */
/*  of the left child of root we must recursively check the right subtree */
/*  of the left child of root as well as the right child of root. */

template<typename T>
std::vector<T> IntervalTree<T>::fetch(int low, int high)  {
  std::vector<T> enumResultStack;
  IntervalTree<T>::Node* x=root->left;
  int stuffToDo = (x != nil);
  
  // Possible speed up: add min field to prune right searches //

#ifdef DEBUG_ASSERT
  assert((recursionNodeStack.size() == 0)
         || !"recursionStack not empty when entering IntervalTree::fetch");
#endif
  currentParent = 0;
  recursionNodeStack.push_back(IntervalTree<T>::it_recursion_node());

  while(stuffToDo) {
    if (Overlap(low,high,x->key,x->high_) ) {
      enumResultStack.push_back(x->value());
      recursionNodeStack[currentParent].tryRightBranch=1;
    }
    if(x->left->maxHigh >= low) { // implies x != nil 
      recursionNodeStack.back().start_node = x;
      recursionNodeStack.back().tryRightBranch = 0;
      recursionNodeStack.back().parentIndex = currentParent;
      currentParent = recursionNodeStack.size();
      recursionNodeStack.push_back(IntervalTree<T>::it_recursion_node());
      x = x->left;
    } else {
      x = x->right;
    }
    stuffToDo = (x != nil);
    while( (!stuffToDo) && (recursionNodeStack.size() > 0) ) {
        IntervalTree<T>::it_recursion_node back = recursionNodeStack.back();
        recursionNodeStack.pop_back();

        if(back.tryRightBranch) {
          x=recursionNodeStack.back().start_node->right;
          currentParent=recursionNodeStack.back().parentIndex;
          recursionNodeStack[currentParent].tryRightBranch=1;
          stuffToDo = ( x != nil);
        }
    }
  }
#ifdef DEBUG_ASSERT
  assert((recursionNodeStack.size() == 0)
         || !"recursionStack not empty when exiting IntervalTree::fetch");
#endif
  return(enumResultStack);   
}
        


template<typename T>
int IntervalTree<T>::CheckMaxHighFieldsHelper(IntervalTree<T>::Node * y, 
                                    const int currentHigh,
                                    int match) const
{
  if (y != nil) {
    match = CheckMaxHighFieldsHelper(y->left,currentHigh,match) ?
      1 : match;
    if (y->high_ == currentHigh)
      match = 1;
    match = CheckMaxHighFieldsHelper(y->right,currentHigh,match) ?
      1 : match;
  }
  return match;
}

          

/* Make sure the maxHigh fields for everything makes sense. *
 * If something is wrong, print a warning and exit */
template<typename T>
void IntervalTree<T>::CheckMaxHighFields(IntervalTree<T>::Node * x) const {
  if (x != nil) {
    CheckMaxHighFields(x->left);
    if(!(CheckMaxHighFieldsHelper(x,x->maxHigh,0) > 0)) {
      assert(0);
    }
    CheckMaxHighFields(x->right);
  }
}

template<typename T>
void IntervalTree<T>::check() const {
 CheckMaxHighFields(root->left);
}

#endif

