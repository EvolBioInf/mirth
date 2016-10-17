# mirth

*mi*nimal mappable *r*ead leng*th* calculation

## Why mirth?

mirth can be used as a blazingly fast, easy-to-use alternative for GEM
mappability to obtain uniquely mappable genomic regions, if for the use case at
hand finding only perfect matches is sufficient (like `gem-mappability`
with `-m 0`).

## What is MMRL?

MMRL means Minimal Mappable Read Length and is a value calculated for every
nucleotide position. It says how long a read that starts at this position must be,
to be uniquely mappable to this position.

A small value indicates regions that are less repetitive and can be uniquely mapped
with shorter reads. Conversely, given some some read size N, regions that are
uniquely mappable with reads of this length are exactly the regions with MMRL
values less or equal than N.

## Build

This program depends on [libdivsufsort](https://github.com/y-256/libdivsufsort).

To build mirth, first obtain the source:

```
git clone https://github.com/EvolBioInf/mirth.git
cd mirth
```

If libdivsufsort is not installed, build it first:

```
make libdivsufsort
```

Finally, build mirth:

```
make
```

The `mirth` binary is now located in the `build` directory.

## Usage

First, the MMRL values have to be calculated. All sequences considered for the
mapping must be supplied in one FASTA file:

```
mirth sequences.fa > sequences_mmrl.bedgraph
```

If the sequences are in different files, it is possible to concatenate them on
the fly and supply them via standard input:

```
cat seq1.fa seq2.fa seq3.fa | mirth > sequences_mmrl.bedgraph
```

From this track the unique mappability tracks for any read lengths can be easily
extracted, e.g. to see uniquely mappable regions for read length of 36bp, the
following command can be used:

```
mirth sequences_mmrl.bedgraph -l 36 > sequences_mappability_36.bedgraph
```

The resulting annotation track reports a value of 1 for uniquely mappable
regions given the supplied read length and a value of 0 otherwise and is almost
identical to a GEM mappability track where all values <1 are truncated to 0.

This track can be used to mask sequences and viewed in a genome browser of the
users choice, e.g. UCSC genome browser or the Integrative Genome Viewer.
use [kentUtils](https://github.com/ENCODE-DCC/kentUtils) can be used to convert
the BedGraph data into other formats.

