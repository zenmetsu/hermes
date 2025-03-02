#ifndef LDPC_H
#define LDPC_H

void ldpc_decode(double llcodeword[], int iters, int plain[], int *ok);
void ldpc_encode(int plain[87], int codeword[174]);
int ldpc_check(int codeword[]);
extern int gen[87][87]; // Declaration only

#endif