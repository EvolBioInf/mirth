#pragma once

#define PROGNAME "mirth"
#define DESCRIPTION "minimal mappable read length calculation"
#define VERSION "0.1"
#define COPYRIGHT "Copyright (C) 2016 Anton Pirogov, Bernhard Haubold"

struct Args {
  void parse(int argc, char *argv[]);

  bool h = false; // help message?
  bool b = false;  // benchmark run
  bool v = false;  // verbose output

  size_t l = 0;  // read masker mode -> read length

  // non-parameter arguments
  size_t num_files = 0;
  char **files = nullptr;
};

extern Args args;
