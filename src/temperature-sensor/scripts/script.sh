#!/usr/bin/env bash
set -e

function coresTemp() {
  sensors -j  | jq 'to_entries | map([.key, .value]) | first | .[1] | with_entries(select(.key|contains("Core"))) | .[] | to_entries | .[0].value' > $1/out.script
}

function packageTemp() {
  sensors -j  | jq 'to_entries | map([.key, .value]) | first | .[1] | with_entries(select(.key|contains("Package"))) | .[] | to_entries | .[0].value' > $1/out.script
}

function apci1Temp() {
  sensors -j  | jq 'to_entries | map([.key, .value]) | .[1] | .[1] | with_entries(select(.key|contains("temp1"))) | .[] | .temp1_input' > $1/out.script
}

function apci2Temp() {
  sensors -j  | jq 'to_entries | map([.key, .value]) | .[1] | .[1] | with_entries(select(.key|contains("temp2"))) | .[] | .temp2_input' > $1/out.script
}


function main() {
  CORES=false
  PACKAGE=false
  ACPI_1=false
  ACPI_2=false

  if [[ $# -eq 0 ]] ; then
    echo "Missing arguments ... Use --cores, --package, --acpi-1, --acpi-2 along with --out "
    exit 1
  fi

  while [[ $# -gt 0 ]] ; do
    arg="$1"
    case $arg in
      --cores)
        CORES=true
        ;;
      --package)
        PACKAGE=true
        ;;
      --acpi-1)
        ACPI_1=true
        ;;
      --acpi-2)
        ACPI_2=true
        ;;
      --out)
        shift
        OUT=$1
        ;;
      *)
        echo "Unknown argument: $1. Use --cores, --package, --acpi-1, --acpi-2 along with --out "
        exit 1
        ;;
    esac
    shift
  done

  if [[ -z "${OUT}" ]] ; then
    echo "An output directory is required. Use --out"
    exit 1
  fi

  mkdir -p ${OUT}

  if [[ "${CORES}" == "true" ]] ; then
    coresTemp "${OUT}"
    return 0
  fi

  if [[ "${PACKAGE}" == "true" ]] ; then
    packageTemp "${OUT}"
    return 0
  fi

  if [[ "${ACPI_1}" == "true" ]] ; then
    apci1Temp "${OUT}"
    return 0
  fi

  if [[ "${ACPI_2}" == "true" ]] ; then
    apci2Temp "${OUT}"
    return 0
  fi
}

main $*