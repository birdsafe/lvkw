#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 3 ]; then
  echo "usage: $0 <c-compiler> <cxx-compiler> <source-dir>" >&2
  exit 2
fi

CC_BIN="$1"
CXX_BIN="$2"
SOURCE_DIR="$3"

C_HEADERS=(
  "lvkw/c/core.h"
  "lvkw/c/instrumentation.h"
  "lvkw/c/context.h"
  "lvkw/c/input.h"
  "lvkw/c/display.h"
  "lvkw/c/data.h"
  "lvkw/c/events.h"
  "lvkw/c/ext/controller.h"
  "lvkw/c/shortcuts.h"
  "lvkw/lvkw.h"
)

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

write_config() {
  local cfg_root="$1"
  local enable_controller="$2"
  local use_float="$3"

  mkdir -p "${cfg_root}/lvkw/details"
  {
    echo "#ifndef LVKW_CONFIG_H_INCLUDED"
    echo "#define LVKW_CONFIG_H_INCLUDED"
    echo "#define LVKW_VERSION_MAJOR 0"
    echo "#define LVKW_VERSION_MINOR 1"
    echo "#define LVKW_VERSION_PATCH 0"
    if [ "${enable_controller}" = "1" ]; then
      echo "#define LVKW_ENABLE_CONTROLLER"
    fi
    if [ "${use_float}" = "1" ]; then
      echo "#define LVKW_USE_FLOAT"
    fi
    echo "#define LVKW_API_VALIDATION 0"
    echo "#endif"
  } > "${cfg_root}/lvkw/details/lvkw_config.h"
}

run_variant() {
  local lang="$1"
  local compiler="$2"
  local std_flag="$3"
  local enable_controller="$4"
  local use_float="$5"
  local headers_name="$6"

  local variant_dir="${TMP_DIR}/${lang}_ctrl${enable_controller}_flt${use_float}"
  local cfg_root="${variant_dir}/cfg"
  mkdir -p "${variant_dir}"
  write_config "${cfg_root}" "${enable_controller}" "${use_float}"

  local -n headers_ref="${headers_name}"
  for header in "${headers_ref[@]}"; do
    local stem
    stem="$(echo "${header}" | tr '/.' '__')"
    local unit="${variant_dir}/${stem}.${lang}"
    local obj="${variant_dir}/${stem}.o"
    cat > "${unit}" <<EOF
#include "${header}"
int main(void) { return 0; }
EOF
    "${compiler}" "${std_flag}" -I"${cfg_root}" -I"${SOURCE_DIR}/include" -c "${unit}" -o "${obj}"
  done
}

CPP_HEADERS=(
  "${C_HEADERS[@]}"
  "lvkw/lvkw.hpp"
  "lvkw/cpp/fwd.hpp"
  "lvkw/cpp/error.hpp"
  "lvkw/cpp/context.hpp"
  "lvkw/cpp/window.hpp"
  "lvkw/cpp/cursor.hpp"
  "lvkw/cpp/monitor.hpp"
  "lvkw/cpp/controller.hpp"
  "lvkw/cpp/events.hpp"
  "lvkw/cpp/cxx20.hpp"
)

run_variant "c" "${CC_BIN}" "-std=c11" "0" "0" "C_HEADERS"
run_variant "c" "${CC_BIN}" "-std=c11" "1" "0" "C_HEADERS"
run_variant "c" "${CC_BIN}" "-std=c11" "0" "1" "C_HEADERS"
run_variant "c" "${CC_BIN}" "-std=c11" "1" "1" "C_HEADERS"

run_variant "cpp" "${CXX_BIN}" "-std=c++20" "0" "0" "CPP_HEADERS"
run_variant "cpp" "${CXX_BIN}" "-std=c++20" "1" "0" "CPP_HEADERS"
run_variant "cpp" "${CXX_BIN}" "-std=c++20" "0" "1" "CPP_HEADERS"
run_variant "cpp" "${CXX_BIN}" "-std=c++20" "1" "1" "CPP_HEADERS"

echo "taxonomy header self-containment checks passed"
