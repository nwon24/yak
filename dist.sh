#!/bin/bash

DIR=$(basename $(pwd))
cd ..
tar --exclude=*.tar --exclude=.git -czf $DIR/$1 $DIR
