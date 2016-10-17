#include <iostream>
using namespace std;

#include "args.h"
#include <getopt.h>

// globally accessible arguments for convenience
Args args;

static char const opts_short[] = "hbvl:";
static struct option const opts[] = {
    {"help", no_argument, nullptr, 'h'},
    {"verbose", no_argument, nullptr, 'v'},
    {"length", required_argument, nullptr, 'l'},
    {"benchmark", no_argument, nullptr, 'b'},
    {0, 0, 0, 0} // <- required
};

static char const usage[] = PROGNAME " " VERSION "\n" DESCRIPTION "\n" COPYRIGHT "\n"
    "Usage: " PROGNAME " [OPTIONS] FILE\n"
    "OPTIONS:\n"
    "\t-h: print this help message and exit\n"
    "\t-l NUM: use filtering mode for given read length\n"
    "\t-b: print benchmarking information\n"
    "\t-v: verbose output (for debugging)\n";

void Args::parse(int argc, char *argv[]) {
  int c = 0;       // getopt stores value returned (last struct component) here
  int opt_idx = 0; // getopt stores the option index here.
  while ((c = getopt_long(argc, argv, opts_short, opts, &opt_idx)) != -1) {
    switch (c) {
    case 0: // long option without a short name
      /* printf("Option: %s\n", opts[opt_idx].name); */
      break;
    case 'h':
      cout << usage;
      exit(0);
      break;

    case 'b':
      args.b = true;
      break;
    case 'v':
      args.v = true;
      break;
    case 'l':
      args.l = atoi(optarg);
      break;

    case '?': // automatic error message from getopt
      exit(1);
      break;
    default:
      cerr << "Error while parsing command line arguments. "
              "Please file a bug report."
           << endl;
      abort();
    }
  }

  // remaining arguments are not options and flags -> files
  if (optind < argc) {
    args.num_files = argc - optind;
    args.files = &argv[optind];
  }

  if (args.num_files > 1)
    cerr << "WARNING: processing only first file: " << args.files[0] << endl;
}
