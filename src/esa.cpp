#include <cinttypes>
#include <iostream>
#include <algorithm>
using namespace std;

#include <divsufsort64.h>

#include "bench.h"
#include "esa.h"

// calculate suffix array using divsufsort
uint_vec getSa(char const *seq, size_t n) {
  sauchar_t *t = (sauchar_t *)seq;
  vector<saidx64_t> sa(n + 1);
  if (divsufsort64(t, sa.data(), (saidx64_t)n) != 0) {
    cout << "ERROR[esa]: suffix sorting failed." << endl;
    exit(-1);
  }
  uint_vec ret(n+1);
  for (size_t i=0; i<n+1; i++)
    ret[i] = sa[i];
  return ret;
}

/* calcLcp: compute LCP array using the algorithm in Figure 3
 *   of Kasai et al (2001). Linear-time longest-common-prefix
 *   computation in suffix arrays and its applications. LNCS 2089
 *   p. 191-192.
 */
void calcLcp(Esa &esa) {
  char const *t = esa.str;
  size_t n = esa.n;
  auto &sa = esa.sa;
  auto &rank = esa.isa;

  esa.lcp.resize(n + 1);
  auto &lcp = esa.lcp;
  int64_t h = 0, j = 0;
  lcp[0] = lcp[n] = 0;
  for (size_t i = 0; i < n; i++) {
    if (rank[i] > 0) {
      j = sa[rank[i] - 1];
      while (t[i + h] == t[j + h])
        h++;
      lcp[rank[i]] = h;
      if (h > 0)
        h--;
    }
  }
}

Esa::Esa(char const *seq, size_t len) : str(seq), n(len) {
  tick();
  sa = getSa(seq, n);
  tock("libdivsufsort");

  isa = uint_vec(n+1);
  for (size_t i = 0; i < n; i++)
    isa[sa[i]] = i;

  tick();
  calcLcp(*this);
  tock("calcLCP");
}

void Esa::print() const {
  cout << "i\tSA\tISA\tLCP\tSuffix" << endl;
  for (size_t i = 0; i < this->n; i++)
    cout << i << "\t" << sa[i] << "\t" << isa[i] << "\t" << lcp[i] << "\t" << string(str + sa[i],20)
         << endl;
}

