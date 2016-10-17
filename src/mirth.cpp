#include <iostream>
#include <algorithm>
#include <iomanip>
#include <string>
using namespace std;

#include "args.h"
#include "bench.h"
#include "fastafile.h"
#include "esa.h"
#include "util.h"

bool check_unique_names(FastaFile &ff) {
  vector<string> names;
  for (auto &seq : ff.seqs)
    names.push_back(seq.name);
  sort(names.begin(),names.end());
  for (size_t i=0; i<names.size()-1; i++)
    if (names[i]==names[i+1])
      return false;
  return true;
}

void filterFile(char const *file) {
  with_file_in(file, [&](istream &in){
    string line;
    getline(in, line);
    if (line.substr(0,5)!="track") {
      cerr << "ERROR: This does not look like a bedgraph file!" << endl;
      return false;
    }
    string newtitle("\"MMRL <=");
    newtitle += to_string(args.l) + "\"";
    line.replace(line.find("MMRL"), 4, newtitle);
    cout << line << endl;

    string currseq;
    size_t currfrom, currto;
    string seq;
    size_t from, to, mmrl;
    while (in >> seq >> from >> to >> mmrl) {
      if (mmrl > args.l)
        continue;
      if (seq != currseq || from>currto) {
        if (currseq.size()>0) {
          cout << currseq << "\t" << currfrom << "\t" << currto << "\t1" << endl;
          if (from!=0)
            cout << currseq << "\t" << currto << "\t" << from << "\t0" << endl;
        }
        currseq = seq;
        currfrom = from;
        currto = to;
      } else {
        currto = to;
      }
    }
    cout << currseq << "\t" << currfrom << "\t" << currto << "\t1" << endl;
    return true;
  });
}

//load / extract data, show results
void processFile(char const *file) {
  tick();
  FastaFile ff(file);
  tock("readFastaFromFile");
  if (ff.failed) {
    cerr << "Invalid FASTA file!" << endl;
    return;
  }
  if (!check_unique_names(ff)) {
    cerr << "Headers of the FASTA sequences must be unique before the first whitespace!" << endl;
    return;
  }

  //construct concatenated sequence
  string s = "";
  size_t offset=0;
  vector<size_t> seqstarts; //positions where new sequences start
  for (auto &it : ff.seqs) { //extract region list from file
    seqstarts.push_back(offset);
    offset += it.seq.size();
    s += it.seq;
    it.seq = ""; //free memory of separate sequences
  }
  seqstarts.push_back(offset);

  // get list of bad intervals
  vector<pair<size_t,size_t>> bad;
  tick();
  bool insidebad = false;
  size_t start = 0;
  string validchars = "ACGT$";
  for (size_t i = 0; i < s.size(); i++) {
    bool valid = validchars.find(s[i]) != string::npos;
    if (insidebad && valid) {
      insidebad = false;
      bad.push_back(make_pair(start, i - 1));
    } else if (!insidebad && !valid) {
      start = i;
      insidebad = true;
    }
  }
  if (insidebad) // push last one, if we are inside
    bad.push_back(make_pair(start, s.size() - 1));
  tock("find bad intervals");

  //calculate ESA of both strands
  s = s + "$" + revComp(s) + "$";
  tick();
  Esa esa(s.c_str(), s.size()); // esa for seq+$+revseq+$
  tock("getEsa");

  //calculate shustring lengths
  tick();
  vector<int64_t> shus(esa.n,-1);
  string valid="ACGT";
  for (size_t i=0; i<esa.n; i++) {
    if (valid.find(esa.str[esa.sa[i]])!=string::npos) {
      shus[esa.sa[i]] = max(esa.lcp[i], i==esa.n-1 ? -1 : esa.lcp[i+1])+1;
    }
  }

  //remove results going into N regions
  for (auto &it : bad) {
    auto i = it.first;
    if (i==0)
      continue;
    size_t j=i-1;
    while (j+shus[j]>=i) {
      shus[j] = -1;
      j--;
    }
  }
  //remove results from borders
  for (auto &i : seqstarts) {
    if (i==0)
      continue;
    size_t j=i-1;
    while (j+shus[j]>=i) {
      shus[j] = -1;
      j--;
    }
  }
  tock("get shulens");

  if (args.v) {
    esa.print();
    cout << "----" << endl;
  }

  //output result
  cout << "track type=bedGraph name=MMRL description=\"Minimal mappable read length\" visibility=full" << endl;
  if (args.v) {
    size_t currseq=0;
    for (size_t i=0; i<esa.n/2-1; i++) {
      if (currseq<seqstarts.size()-1 && i>=seqstarts[currseq+1])
        currseq++;
      if (shus[i]>0) {
        string &name = ff.seqs[currseq].name;
        size_t pos = i-seqstarts[currseq];
        cout << name << "\t" << pos << "\t" << pos+1 << "\t" << shus[i] << endl;
      }
    }
  } else {
    size_t currseq = 0;
    size_t maxi = esa.n/2-1;
    size_t ivstart = 0;
    size_t ivend = 0;
    for (size_t i=0; i<maxi; i++) {
      if (currseq<seqstarts.size()-1 && i>=seqstarts[currseq+1])
        currseq++;
      if (shus[i]>0) {
        if (i!=0 && shus[i-1] != shus[i])
          ivstart = i;
        if (i==maxi-1 || shus[i+1] != shus[i]) {
          ivend = i;
          string &name = ff.seqs[currseq].name;
          size_t posstart = ivstart-seqstarts[currseq];
          size_t posend = ivend-seqstarts[currseq];
          cout << name << "\t" << posstart << "\t" << posend+1 << "\t" << shus[i] << endl;
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  args.parse(argc, argv);

  tick();
  char const *file = args.num_files == 0 ? nullptr : args.files[0];
  if (args.l==0)
    processFile(file);
  else
    filterFile(file);
  tock("total time");
}
