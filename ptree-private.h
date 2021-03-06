/*
*  File:        ptree-private.h
*  Description: Private functions for the PTree class
*  Date:        2022-03-06 03:30
*
*               DECLARE YOUR PTREE PRIVATE MEMBER FUNCTIONS HERE
*/

#ifndef _PTREE_PRIVATE_H_
#define _PTREE_PRIVATE_H_

/////////////////////////////////////////////////
// DEFINE YOUR PRIVATE MEMBER FUNCTIONS HERE
//
// Just write the function signatures.
//
// Example:
//
// Node* MyHelperFunction(int arg_a, bool arg_b);
//
/////////////////////////////////////////////////
HSLAPixel ComputeAvg(unsigned int x, unsigned int y, pair<unsigned int, unsigned int> ul, PNG& im);

int getSize(Node* root) const;

int getLeaves(Node* root) const;

void clearTree(Node* root);

void Horizontal(Node* root);

void Vertical(Node* root);

Node* copyTree(Node* root);

void renderPNG(Node* root, PNG &image) const;

void pruneTree(Node *root, double tolerance);

bool isWithinAverage(Node *root, double tolerance, HSLAPixel average);

#endif