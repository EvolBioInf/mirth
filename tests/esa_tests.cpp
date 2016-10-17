#include "minunit.h"
#include "util.h"
#include <cstring>
#include <ctime>
#include <algorithm>
#include <string>
using namespace std;

#include "fastafile.h"
#include "esa.h"

// some basic tests
void test_getEsa() {
  FastaFile ff("Data/example.fasta");
  ff.seqs[0].seq += "$";

  char const *s = ff.seqs[0].seq.c_str();
  size_t n = ff.seqs[0].seq.size();
  Esa esa(s, n); // calculate esa, including $

  mu_assert(esa.str == s, "ESA does not point to original sequence");
  mu_assert(esa.n == n, "ESA size not correct");
  mu_assert(esa.str[esa.sa[0]] == '$', "first ESA entry not $");
  mu_assert(esa.sa[0] == (uint64_t)(n - 1), "wrong SA index");
  mu_assert(esa.isa[esa.sa[0]] == 0, "isa incorrect");
  mu_assert(esa.isa[esa.sa[n - 1]] == (uint64_t)(n - 1), "isa incorrect");
  mu_assert(esa.lcp[0] == 0, "first LCP not 0");
  mu_assert(esa.lcp[esa.n] == 0, "last LCP not 0");
}

void all_tests() {
  srand(time(NULL));
  mu_run_test(test_getEsa);
}
RUN_TESTS(all_tests)
