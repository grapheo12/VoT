#!/bin/sh

rm -rf tallying/results/*
OMP_NUM_THREADS=4 ./tallying/bin/main tallying/dump keygen/keys/pk.key keygen/keys/bk.key tallying/results