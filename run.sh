#!/bin/bash
#sudo rm -fr CMakeFiles
sudo cmake .
sudo make
sudo rm -fr CMakeFiles
sudo rm -f CMakeCache.txt
sudo rm -f cmake_install.cmake
sudo rm -f Makefile

