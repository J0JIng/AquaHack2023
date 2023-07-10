# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/licong/esp/esp-idf/components/bootloader/subproject"
  "/home/licong/project/AquaHack2023/src/build/bootloader"
  "/home/licong/project/AquaHack2023/src/build/bootloader-prefix"
  "/home/licong/project/AquaHack2023/src/build/bootloader-prefix/tmp"
  "/home/licong/project/AquaHack2023/src/build/bootloader-prefix/src/bootloader-stamp"
  "/home/licong/project/AquaHack2023/src/build/bootloader-prefix/src"
  "/home/licong/project/AquaHack2023/src/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/licong/project/AquaHack2023/src/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/licong/project/AquaHack2023/src/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
