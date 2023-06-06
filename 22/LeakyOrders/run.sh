#!/usr/bin/env bash

make
make docker
docker run --rm -it haklu:sigs
