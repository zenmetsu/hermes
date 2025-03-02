#include "util.h"
#include <TimeLib.h>
#include <math.h>

double now() {
    return (double)Teensy3Clock.get() + (micros() % 1000000) / 1000000.0;
}

void writewav(const std::vector<double>& samples, const char* filename, int rate) {}

std::vector<double> readwav(const char* filename, int& rate_out) {
    rate_out = 0;
    return std::vector<double>();
}

void writetxt(std::vector<double> v, const char* filename) {}

std::complex<double> goertzel(std::vector<double> v, int rate, int i0, int n, double hz) {
    double bin_hz = rate / (double)n;
    double k = hz / bin_hz;
    double alpha = 2 * M_PI * k / n;
    double beta = 2 * M_PI * k * (n - 1.0) / n;

    double two_cos_alpha = 2 * cos(alpha);
    double a = cos(beta);
    double b = -sin(beta);
    double c = sin(alpha) * sin(beta) - cos(alpha) * cos(beta);
    double d = sin(2 * M_PI * k);

    double w1 = 0, w2 = 0;
    for (int i = 0; i < n; i++) {
        double w0 = v[i0 + i] + two_cos_alpha * w1 - w2;
        w2 = w1;
        w1 = w0;
    }

    double re = w1 * a + w2 * c;
    double im = w1 * b + w2 * d;
    return std::complex<double>(re, im);
}

double vmax(const std::vector<double>& v) {
    double mx = 0;
    int got = 0;
    for (int i = 0; i < (int)v.size(); i++) {
        if (got == 0 || v[i] > mx) {
            got = 1;
            mx = v[i];
        }
    }
    return mx;
}

std::complex<double> cvmax(const std::vector<std::complex<double>>& v) {
    std::complex<double> mx(0, 0);
    int got = 0;
    for (int i = 0; i < (int)v.size(); i++) {
        if (got == 0 || std::abs(v[i]) > std::abs(mx)) {
            got = 1;
            mx = v[i];
        }
    }
    return mx;
}

std::vector<double> vreal(const std::vector<std::complex<double>>& a) {
    std::vector<double> b(a.size());
    for (int i = 0; i < (int)a.size(); i++) {
        b[i] = a[i].real();
    }
    return b;
}

std::vector<double> vimag(const std::vector<std::complex<double>>& a) {
    std::vector<double> b(a.size());
    for (int i = 0; i < (int)a.size(); i++) {
        b[i] = a[i].imag();
    }
    return b;
}

double gfsk_point(double b, double t) {
    double c = M_PI * sqrt(2.0 / log(2.0));
    double x = 0.5 * (erf(c * b * (t + 0.5)) - erf(c * b * (t - 0.5)));
    return x;
}

std::vector<double> gfsk_window(int samples_per_symbol, double b) {
    std::vector<double> v(3 * samples_per_symbol);
    double sum = 0;
    for (int i = 0; i < (int)v.size(); i++) {
        double x = i / (double)samples_per_symbol - 1.5;
        double y = gfsk_point(b, x);
        v[i] = y;
        sum += y;
    }
    for (int i = 0; i < (int)v.size(); i++) {
        v[i] /= sum;
    }
    return v;
}

std::vector<std::complex<double>> gfsk_c(const std::vector<int>& symbols,
                                        double hz0, double hz1,
                                        double spacing, int rate, int symsamples,
                                        double phase0,
                                        const std::vector<double>& gwin) {
    assert((gwin.size() % 2) == 0);
    std::vector<double> hzv(symsamples * (symbols.size() + 2), 0.0);
    for (int bi = 0; bi < (int)symbols.size(); bi++) {
        double base_hz = hz0 + (hz1 - hz0) * (bi / (double)symbols.size());
        double fr = base_hz + (symbols[bi] * spacing);
        int mid = symsamples * (bi + 1) + symsamples / 2;
        hzv[mid] = fr * symsamples / 2.0;
        hzv[mid - 1] = fr * symsamples / 2.0;
    }
    for (int i = 0; i < symsamples; i++) {
        hzv[i] = hzv[i + symsamples];
        hzv[symsamples * (symbols.size() + 1) + i] = hzv[symsamples * symbols.size() + i];
    }
    int half = gwin.size() / 2;
    std::vector<double> o(hzv.size());
    for (int i = 0; i < (int)o.size(); i++) {
        double sum = 0;
        for (int j = 0; j < (int)gwin.size(); j++) {
            int k = i - half + j;
            if (k >= 0 && k < (int)hzv.size()) {
                sum += hzv[k] * gwin[j];
            }
        }
        o[i] = sum;
    }
    std::vector<double> oo(symsamples * symbols.size());
    for (int i = 0; i < (int)oo.size(); i++) {
        oo[i] = o[i + symsamples];
    }
    std::vector<std::complex<double>> v(symsamples * symbols.size());
    double theta = phase0;
    for (int i = 0; i < (int)v.size(); i++) {
        v[i] = std::complex(cos(theta), sin(theta));
        double hz = oo[i];
        theta += 2 * M_PI / (rate / hz);
    }
    return v;
}

std::vector<double> gfsk_r(const std::vector<int>& symbols,
                          double hz0, double hz1,
                          double spacing, int rate, int symsamples,
                          double phase0,
                          const std::vector<double>& gwin) {
    assert((gwin.size() % 2) == 0);
    std::vector<double> hzv(symsamples * (symbols.size() + 2), 0.0);
    for (int bi = 0; bi < (int)symbols.size(); bi++) {
        double base_hz = hz0 + (hz1 - hz0) * (bi / (double)symbols.size());
        double fr = base_hz + (symbols[bi] * spacing);
        int mid = symsamples * (bi + 1) + symsamples / 2;
        hzv[mid] = fr * symsamples / 2.0;
        hzv[mid - 1] = fr * symsamples / 2.0;
    }
    for (int i = 0; i < symsamples; i++) {
        hzv[i] = hzv[i + symsamples];
        hzv[symsamples * (symbols.size() + 1) + i] = hzv[symsamples * symbols.size() + i];
    }
    int half = gwin.size() / 2;
    std::vector<double> o(hzv.size());
    for (int i = 0; i < (int)o.size(); i++) {
        double sum = 0;
        for (int j = 0; j < (int)gwin.size(); j++) {
            int k = i - half + j;
            if (k >= 0 && k < (int)hzv.size()) {
                sum += hzv[k] * gwin[j];
            }
        }
        o[i] = sum;
    }
    std::vector<double> oo(symsamples * symbols.size());
    for (int i = 0; i < (int)oo.size(); i++) {
        oo[i] = o[i + symsamples];
    }
    std::vector<double> v(symsamples * symbols.size());
    double theta = phase0;
    for (int i = 0; i < (int)v.size(); i++) {
        v[i] = cos(theta);
        double hz = oo[i];
        theta += 2 * M_PI / (rate / hz);
    }
    return v;
}

double cycle_second() {
    double tt = now();
    double st = trunc(tt / 15.0) * 15;
    return tt - st;
}

void ft8_crc(int msg1[], int msglen, int out[12]) {
    int div[] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0};
    int* msg = (int*)malloc(sizeof(int) * (msglen + 12));
    for (int i = 0; i < msglen + 12; i++) {
        msg[i] = (i < msglen) ? msg1[i] : 0;
    }
    for (int i = 0; i < msglen; i++) {
        if (msg[i]) {
            for (int j = 0; j < 13; j++) {
                msg[i + j] = (msg[i + j] + div[j]) % 2;
            }
        }
    }
    for (int i = 0; i < 12; i++) {
        out[i] = msg[msglen + i];
    }
    out[10] ^= 1;
    out[8] ^= 1;
    out[6] ^= 1;
    free(msg);
}