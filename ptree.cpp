/*
*  File:        ptree.cpp
*  Description: Implementation of a partitioning tree class for CPSC 221 PA3
*  Date:        2022-03-03 01:53
*
*               ADD YOUR PRIVATE FUNCTION IMPLEMENTATIONS TO THE BOTTOM OF THIS FILE
*/

#include "ptree.h"
#include "hue_utils.h" // useful functions for calculating hue averages

using namespace cs221util;
using namespace std;

// The following definition may be convenient, but is not necessary to use
typedef pair<unsigned int, unsigned int> pairUI;

/////////////////////////////////
// PTree private member functions
/////////////////////////////////

void PTree::clearTree(Node* root) {
  // if (root->A == NULL && root->B == NULL) {
  //   delete root;
  // } else {
  //   clearTree(root->A);
  //   clearTree(root->B);
  // }
  // root = NULL;
  if (!root) return;
  else {
    Node *tempA = root->A;
    Node *tempB = root->B;
    delete root;
    clearTree(tempA);
    clearTree(tempB);
  }
}

/*
*  Destroys all dynamically allocated memory associated with the current PTree object.
*  You may want to add a recursive helper function for this!
*  POST: all nodes allocated into the heap have been released.
*/
void PTree::Clear() {
  // add your implementation below
  clearTree(root);
}

Node* PTree::copyTree(Node* root) {
  if (!root) return NULL;

  Node *node = new Node();

  node->upperleft = root->upperleft;
  node->width = root->width;
  node->height = root->height;
  node->avg = root->avg;

  node->A = copyTree(root->A);
  node->B = copyTree(root->B);

  return node;
}

/*
*  Copies the parameter other PTree into the current PTree.
*  Does not free any memory. Should be called by copy constructor and operator=.
*  You may want a recursive helper function for this!
*  PARAM: other - the PTree which will be copied
*  PRE:   There is no dynamic memory associated with this PTree.
*  POST:  This PTree is a physically separate copy of the other PTree.
*/
void PTree::Copy(const PTree& other) {
  // add your implementation below
  Node *otherRoot = other.root; // can access root directly
  root = copyTree(otherRoot);
}

HSLAPixel PTree::ComputeAvg(unsigned int w, unsigned int h, pair<unsigned int, unsigned int> ul, PNG& im) {
  double XcomponentHue = 0;
  double YcomponentHue = 0;
  double avgSat = 0;
  // double avgAlp = 0;
  double avgLue = 0;

  for (unsigned int x = ul.first; x < ul.first + w; x++) {
    for (unsigned int y = ul.second; y < ul.second + h; y++) {
      HSLAPixel *pixel = im.getPixel(x,y);
      XcomponentHue += Deg2X(pixel->h);
      YcomponentHue += Deg2Y(pixel->h);
      avgSat += pixel->s;
      // avgAlp += pixel->a;
      avgLue += pixel->l;
    }
  }

  avgSat = avgSat / (w * h);
  // avgAlp = avgAlp / (w * h);
  avgLue = avgLue / (w * h);
  XcomponentHue = XcomponentHue / (w * h);
  YcomponentHue = YcomponentHue / (w * h);
  double avgHue = XY2Deg(XcomponentHue, YcomponentHue);
  HSLAPixel returnpixel(avgHue, avgSat, avgLue);
  return returnpixel;
}

/*
*  Private helper function for the constructor. Recursively builds the tree
*  according to the specification of the constructor.
*  You *may* change this if you like, but we have provided here what we
*  believe will be sufficient to use as-is.
*  PARAM:  im - full reference image used for construction
*  PARAM:  ul - upper-left image coordinate of the currently building Node's image region
*  PARAM:  w - width of the currently building Node's image region
*  PARAM:  h - height of the currently building Node's image region
*  RETURN: pointer to the fully constructed Node
*/
Node* PTree::BuildNode(PNG& im, pair<unsigned int, unsigned int> ul, unsigned int w, unsigned int h) {
  // average helper function
  if (w == 0 || h == 0) return NULL;

  if (w == 1 && h == 1) {

    Node *singleNode = new Node(ul, w, h, *im.getPixel(ul.first, ul.second), NULL, NULL);
    return singleNode;
    
  } else {

  HSLAPixel average = ComputeAvg(w, h, ul, im);  
  Node *node = new Node(ul, w, h, average, NULL, NULL);

  // replace the line below with your implementation
  if (w > h || w == h) {
    if (w % 2 == 0) { // width is even
      pair<unsigned int, unsigned int> newUL = {ul.first + w/2, ul.second};
      node->A = BuildNode(im, ul, w/2, h);
      node->B = BuildNode(im, newUL, w/2, h);
    } else { // width is odd
      pair<unsigned int, unsigned int> newUL = {ul.first + w/2, ul.second};
      node->A = BuildNode(im, ul, w/2, h);
      node->B = BuildNode(im, newUL, w/2 + 1, h);
    }
  } else if (w < h) {
    if (h % 2 == 0) { // height is even
      pair<unsigned int, unsigned int> newUL = {ul.first, ul.second + h/2};
      node->A = BuildNode(im, ul, w, h/2);
      node->B = BuildNode(im, newUL, w, h/2);
    } else { // height is odd
      pair<unsigned int, unsigned int> newUL = {ul.first, ul.second + h/2};
      node->A = BuildNode(im, ul, w, h/2);
      node->B = BuildNode(im, newUL, w, h/2 + 1);
    }
  }
  return node;
  }
}

////////////////////////////////
// PTree public member functions
////////////////////////////////

/*
*  Constructor that builds the PTree using the provided PNG.
*
*  The PTree represents the sub-image (actually the entire image) from (0,0) to (w-1, h-1) where
*  w-1 and h-1 are the largest valid image coordinates of the original PNG.
*  Each node corresponds to a rectangle of pixels in the original PNG, represented by
*  an (x,y) pair for the upper-left corner of the rectangle, and two unsigned integers for the
*  number of pixels on the width and height dimensions of the rectangular sub-image region the
*  node defines.
*
*  A node's two children correspond to a partition of the node's rectangular region into two
*  equal (or approximately equal) size regions which are either tiled horizontally or vertically.
*
*  If the rectangular region of a node is taller than it is wide, then its two children will divide
*  the region into vertically-tiled sub-regions of equal height:
*  +-------+
*  |   A   |
*  |       |
*  +-------+
*  |   B   |
*  |       |
*  +-------+
*
*  If the rectangular region of a node is wider than it is tall, OR if the region is exactly square,
*  then its two children will divide the region into horizontally-tiled sub-regions of equal width:
*  +-------+-------+
*  |   A   |   B   |
*  |       |       |
*  +-------+-------+
*
*  If any region cannot be divided exactly evenly (e.g. a horizontal division of odd width), then
*  child B will receive the larger half of the two subregions.
*
*  When the tree is fully constructed, each leaf corresponds to a single pixel in the PNG image.
* 
*  For the average colour, this MUST be computed separately over the node's rectangular region.
*  Do NOT simply compute this as a weighted average of the children's averages.
*  The functions defined in hue_utils.h and implemented in hue_utils.cpp will be very useful.
*  Computing the average over many overlapping rectangular regions sounds like it will be
*  inefficient, but as an exercise in theory, think about the asymptotic upper bound on the
*  number of times any given pixel is included in an average calculation.
* 
*  PARAM: im - reference image which will provide pixel data for the constructed tree's leaves
*  POST:  The newly constructed tree contains the PNG's pixel data in each leaf node.
*/
PTree::PTree(PNG& im) {
  // add your implementation below
  // pair<unsigned int, unsigned int> initial(0,0);
  root = BuildNode(im, {0,0}, im.width(), im.height());
}

/*
*  Copy constructor
*  Builds a new tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  This tree is constructed as a physically separate copy of other tree.
*/
PTree::PTree(const PTree& other) {
  // add your implementation below
  Copy(other);
}

/*
*  Assignment operator
*  Rebuilds this tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  If other is a physically different tree in memory, all pre-existing dynamic
*           memory in this tree is deallocated and this tree is reconstructed as a
*           physically separate copy of other tree.
*         Otherwise, there is no change to this tree.
*/
PTree& PTree::operator=(const PTree& other) {
  // add your implementation below
  if (root != other.root) {
  Clear();
  Copy(other);
  }
  return *this;
}

/*
*  Destructor
*  Deallocates all dynamic memory associated with the tree and destroys this PTree object.
*/
PTree::~PTree() {
  // add your implementation below
  Clear();
}

void PTree::renderPNG(Node* root, PNG &image) const {
  if (root == NULL) return;
  else if (root->A == NULL && root->B == NULL) {
    *image.getPixel(root->upperleft.first, root->upperleft.second) = root->avg;
  } else {
    renderPNG(root->A, image);
    renderPNG(root->B, image);
  }
}

/*
*  Traverses the tree and puts the leaf nodes' color data into the nodes'
*  defined image regions on the output PNG.
*  For non-pruned trees, each leaf node corresponds to a single pixel that will be coloured.
*  For pruned trees, each leaf node may cover a larger rectangular region that will be
*  entirely coloured using the node's average colour attribute.
*
*  You may want to add a recursive helper function for this!
*
*  RETURN: A PNG image of appropriate dimensions and coloured using the tree's leaf node colour data
*/
PNG PTree::Render() const {
  // replace the line below with your implementation
  PNG ans(root->width, root->height);
  renderPNG(root, ans);
  return ans;
}


bool PTree::isWithinAverage(Node *root, double tolerance, HSLAPixel average) {
  if (!root) return false;
  if (root->A == NULL && root->B == NULL) {
    return average.dist(root->avg) <= tolerance;
  } 
  else {
    return isWithinAverage(root->A, tolerance, average) && isWithinAverage(root->B, tolerance, average);
  }
}

void PTree::pruneTree(Node *root, double tolerance) {
  if (!root) return;
  HSLAPixel average = root->avg;
  // HSLAPixel averageA = root->A->avg;
  // HSLAPixel averageB = root->B->avg;

  // if (average.dist(averageA) <= tolerance && average.dist(averageB) <= tolerance) {
  //   clearTree(root->A);
  //   clearTree(root->B);
  // }
  // else if (average.dist(averageA) > tolerance || average.dist(averageB) > tolerance) {
  // pruneTree(root->A, tolerance);
  // pruneTree(root->B, tolerance);
  // }

  if (isWithinAverage(root, tolerance, average)) {
    clearTree(root->A);
    clearTree(root->B);
    root->A = NULL;
    root->B = NULL;
  }
  else {
    pruneTree(root->A, tolerance);
    pruneTree(root->B, tolerance);
  }

}

/*
*  Trims subtrees as high as possible in the tree. A subtree is pruned
*  (its children are cleared/deallocated) if ALL of its leaves have colour
*  within tolerance of the subtree root's average colour.
*  Pruning criteria should be evaluated on the original tree, and never on a pruned
*  tree (i.e. we expect that Prune would be called on any tree at most once).
*  When processing a subtree, you should determine if the subtree should be pruned,
*  and prune it if possible before determining if it has subtrees that can be pruned.
* 
*  You may want to add (a) recursive helper function(s) for this!
*
*  PRE:  This tree has not been previously pruned (and is not copied/assigned from a tree that has been pruned)
*  POST: Any subtrees (as close to the root as possible) whose leaves all have colour
*        within tolerance from the subtree's root colour will have their children deallocated;
*        Each pruned subtree's root becomes a leaf node.
*/
void PTree::Prune(double tolerance) {
  // add your implementation below
  pruneTree(root, tolerance);
}

int PTree::getSize(Node* root) const {
  if (!root) return 0;
  else {
    return 1 + getSize(root->A) + getSize(root->B);
  }
}

/*
*  Returns the total number of nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::Size() const {
  // replace the line below with your implementation
  return getSize(root);
}

int PTree::getLeaves(Node* root) const {
  if (!root) return 0;
  else if (root->A == NULL && root->B == NULL) {
    return 1;
  } else {
    return getLeaves(root->A) + getLeaves(root->B);
  }
}

/*
*  Returns the total number of leaf nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::NumLeaves() const {
  // replace the line below with your implementation
  return getLeaves(root);
}

void PTree::Horizontal(Node* root) {
  if (!root) return;
  else if (root->A == NULL && root->B == NULL) return;
  if (root->A->A == NULL || root->B->B == NULL) { // more than swapping colors
         HSLAPixel tempA = root->A->avg; // if leaf then swap
         HSLAPixel tempB = root->B->avg; // another case
         root->A->avg = tempB; // don't only check leaves
         root->B->avg = tempA;
  }
  else {
    // Node* temp = root->A;
    // root->A = root->B;
    // root->B = temp;

    Horizontal(root->A);
    Horizontal(root->B);
  }
}

/*
*  Rearranges the nodes in the tree, such that a rendered PNG will be flipped horizontally
*  (i.e. mirrored over a vertical axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped horizontally.
*/
void PTree::FlipHorizontal() {
  // add your implementation below
  Horizontal(root);
}



void PTree::Vertical(Node* root) {
  
}

/*
*  Like the function above, rearranges the nodes in the tree, such that a rendered PNG
*  will be flipped vertically (i.e. mirrored over a horizontal axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped vertically.
*/
void PTree::FlipVertical() {
  // add your implementation below
  Vertical(root);
}

/*
    *  Provides access to the root of the tree.
    *  Dangerous in practice! This is only used for testing.
    */
Node* PTree::GetRoot() {
  return root;
}

//////////////////////////////////////////////
// PERSONALLY DEFINED PRIVATE MEMBER FUNCTIONS
//////////////////////////////////////////////

