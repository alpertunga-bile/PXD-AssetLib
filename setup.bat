@echo off

git submodule init

git submodule update --depth 1

echo Going to PXD-STL directory

cd third-party\\PXD-STL

echo Setting PXD-STL repository

git submodule init

echo Updating PXD-STL repository's third-party repositories

git submodule update --depth 1

cls

echo Setup is finished