#include "ldpc.h"
#include "arrays.h"
#include "ldpc_gen.h"
#include <math.h>

float fast_tanh(float x) {
    if (x < -4.97) return -1.0;
    if (x > 4.97) return 1.0;
    float x2 = x * x;
    float a = x * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2)));
    float b = 135135.0f + x2 * (62370.0f + x2 * (3150.0f + x2 * 28.0f));
    return a / b;
}

void ldpc_decode(double codeword[], int iters, int plain[], int *ok) {
    double m[87][174];
    double e[87][174];
    int best_score = -1;
    int best_cw[174];

    for (int i = 0; i < 174; i++)
        for (int j = 0; j < 87; j++)
            m[j][i] = codeword[i];

    for (int i = 0; i < 174; i++)
        for (int j = 0; j < 87; j++)
            e[j][i] = 0.0;

    for (int iter = 0; iter < iters; iter++) {
        for (int j = 0; j < 87; j++) {
            for (int ii1 = 0; ii1 < 7; ii1++) {
                int i1 = Nm[j][ii1] - 1;
                if (i1 < 0) continue;
                double a = 1.0;
                for (int ii2 = 0; ii2 < 7; ii2++) {
                    int i2 = Nm[j][ii2] - 1;
                    if (i2 >= 0 && i2 != i1) {
                        a *= fast_tanh(m[j][i2] / 2.0);
                    }
                }
                e[j][i1] = log((1 + a) / (1 - a));
            }
        }

        int cw[174];
        for (int i = 0; i < 174; i++) {
            double l = codeword[i];
            for (int j = 0; j < 3; j++)
                l += e[Mn[i][j] - 1][i];
            cw[i] = (l <= 0.0);
        }
        int score = ldpc_check(cw);
        if (score == 87) {
            for (int i = 0; i < 174; i++)
                plain[i] = cw[colorder[i]];
            *ok = 87;
            return;
        }

        if (score > best_score) {
            for (int i = 0; i < 174; i++)
                best_cw[i] = cw[i];
            best_score = score;
        }

        for (int i = 0; i < 174; i++) {
            for (int ji1 = 0; ji1 < 3; ji1++) {
                int j1 = Mn[i][ji1] - 1;
                double l = codeword[i];
                for (int ji2 = 0; ji2 < 3; ji2++) {
                    if (ji1 != ji2) {
                        int j2 = Mn[i][ji2] - 1;
                        l += e[j2][i];
                    }
                }
                m[j1][i] = l;
            }
        }
    }

    for (int i = 0; i < 174; i++)
        plain[i] = best_cw[colorder[i]];
    *ok = best_score;
}

int ldpc_check(int codeword[]) {
    int score = 0;
    for (int j = 0; j < 87; j++) {
        int x = 0;
        for (int ii1 = 0; ii1 < 7; ii1++) {
            int i1 = Nm[j][ii1] - 1;
            if (i1 >= 0) {
                x ^= codeword[i1];
            }
        }
        if (x == 0) score++;
    }
    return score;
}

void ldpc_encode(int plain[87], int codeword[174]) {
    int cw[174];
    for (int i = 0; i < 174; i++) cw[i] = 0;
    for (int i = 0; i < 87; i++) {
        int sum = 0;
        for (int j = 0; j < 87; j++) sum += plain[j] * gen[i][j];
        cw[i] = sum % 2;
    }
    for (int i = 0; i < 87; i++) cw[87 + i] = plain[i];
    for (int i = 0; i < 174; i++) codeword[i] = cw[colorder[i]];
}