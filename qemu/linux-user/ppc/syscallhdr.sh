#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only

in="$1"
out="$2"
my_abis=`echo "($3)" | tr ',' '|'`
prefix="$4"
offset="$5"

fileguard=LINUX_USER_PPC_`basename "$out" | sed \
    -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' \
    -e 's/[^A-Z0-9_]/_/g' -e 's/__/_/g'`
grep -E "^[0-9A-Fa-fXx]+[[:space:]]+${my_abis}" "$in" | sort -n | (
    printf "#ifndef %s\n" "${fileguard}"
    printf "#define %s\n" "${fileguard}"
    printf "\n"

    while read nr abi name entry compat ; do
        if [ "$entry" = "sys_ni_syscall" ] ; then
            continue
        fi
        if [ -z "$offset" ]; then
            printf "#define TARGET_NR_%s%s\t%s\n" \
                "${prefix}" "${name}" "${nr}"
        else
            printf "#define TARGET_NR_%s%s\t(%s + %s)\n" \
                "${prefix}" "${name}" "${offset}" "${nr}"
        fi
    done

    printf "\n"
    printf "#endif /* %s */" "${fileguard}"
    printf "\n"
) > "$out"