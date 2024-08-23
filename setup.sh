#!/bin/bash

echo "Setting third party Github repositories"

git submodule init

git submodule update --depth 1

echo "Going to PXD-STL directory"

cd third-party/PXD-STL

echo "Setting PXD-STL repository"

git submodule init

echo "Updating PXD-STL repository's third-party repositories"

git submodule update --depth 1

clear

echo "Setup is finished"