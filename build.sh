#!/bin/sh
curl -LO https://static.palera.in/artifacts/loader/universal_lite/palera1nLoader.ipa
curl -LO https://static.palera.in/binpack.tar
make all -j 10
