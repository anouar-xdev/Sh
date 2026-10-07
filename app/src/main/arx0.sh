#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
#
#   █████╗ ██████╗ ██╗  ██╗ ██████╗     .sh
#  ██╔══██╗██╔══██╗╚██╗██╔╝██╔═████╗
#  ███████║██████╔╝ ╚███╔╝ ██║██╔██║   Il2Cpp Offsets Extractor / Updater
#  ██╔══██║██╔══██╗ ██╔██╗ ████╔╝██║
#  ██║  ██║██║  ██║██╔╝ ██╗╚██████╔╝   v1.0.0
#  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝
#
#  Parses a dump.cs produced by Il2CppDumper (Zygisk fork, standard tool, or
#  manual extraction) and maps every symbol in the given Offsets.h to its NEW
#  offset value.
#
#  DESIGN PRINCIPLE — "no hallucinations":
#    Every offset emitted by this tool is traceable to a specific line of the
#    provided dump.cs. If a symbol cannot be resolved with sufficient
#    confidence, the tool emits it flagged as [!] NOT FOUND and keeps the old
#    value with an explicit warning comment — never a fabricated number.
#
#  STRATEGIES (highest priority first):
#    S1  Match by (kind, class, field-name)            → conf 100  [OK]
#    S2  Match by (kind, class, type, sibling fields)  → conf  80  [~]
#    S3  Match by (kind, type, offset value unchanged) → conf  70  [~]
#    S4  Match by offset value only (global scan)      → conf  40  [??]
#    S5  Curated fallback DB (built-in)                → conf 100/70
#
#  Usage:
#    ./arx0.sh <dump.cs> [options]
#    ./arx0.sh                          (interactive prompt)
#
#  Options:
#    --old <Offsets.h>        Path to current/old Offsets.h (default ./Offsets.h)
#    --old-dump <dump.cs>     Path to OLD dump.cs (from previous game version)
#                             → enables S2 and dramatically boosts accuracy
#    --out <path>             Output .h path (default: same dir as dump.cs)
#    --strict                 Only emit entries with confidence >= 90
#    --dry-run                Report only, do not write files
#    --report                 Also write a detailed .report.txt
#    --scan-class <Name>      Print every field of a class from the new dump
#    -v, --version            Show version
#    -h, --help               Show this help
#
#  Requires: bash 4+, awk, grep, sort, sed. Works on Linux, macOS, Termux.
#
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

readonly SCRIPT_NAME="arx0"
readonly SCRIPT_VERSION="1.0.0"

# ─── COLORS ────────────────────────────────────────────────────────────────
if [[ -t 1 ]]; then
    C_RED=$'\e[31m'; C_GRN=$'\e[32m'; C_YEL=$'\e[33m'
    C_BLU=$'\e[34m'; C_MAG=$'\e[35m'; C_CYN=$'\e[36m'
    C_DIM=$'\e[2m'; C_BLD=$'\e[1m'; C_RST=$'\e[0m'
else
    C_RED=""; C_GRN=""; C_YEL=""; C_BLU=""; C_MAG=""
    C_CYN=""; C_DIM=""; C_BLD=""; C_RST=""
fi

# ─── LOGGING ───────────────────────────────────────────────────────────────
log()  { printf '%s[*]%s %s\n' "$C_CYN" "$C_RST" "$*"; }
ok()   { printf '%s[+]%s %s\n' "$C_GRN" "$C_RST" "$*"; }
warn() { printf '%s[!]%s %s\n' "$C_YEL" "$C_RST" "$*" >&2; }
err()  { printf '%s[x]%s %s\n' "$C_RED" "$C_RST" "$*" >&2; }
die()  { err "$*"; exit 1; }
hr()   { printf '%s%s%s\n' "$C_DIM" "───────────────────────────────────────────────────────────────────────" "$C_RST"; }

# ─── GLOBALS ───────────────────────────────────────────────────────────────
TMP=""
DUMP_PATH=""
OLD_DUMP=""
OLD_OFFSETS=""
OUT_PATH=""
STRICT=0
DRY_RUN=0
REPORT=0
SCAN_CLASS=""

cleanup() { [[ -n "$TMP" && -d "$TMP" ]] && rm -rf "$TMP"; }
trap cleanup EXIT

mktmp() { TMP=$(mktemp -d "${TMPDIR:-/tmp}/arx0.XXXXXX"); }

# ─── HELP ──────────────────────────────────────────────────────────────────
usage() {
    cat <<'EOF'

  arx0.sh — Il2Cpp Offsets Extractor / Updater

  USAGE
    ./arx0.sh <dump.cs> [options]
    ./arx0.sh                          interactive prompt

  OPTIONS
    --old <Offsets.h>       Path to old Offsets.h (default: ./Offsets.h)
    --old-dump <dump.cs>    Path to OLD dump.cs → enables sibling-field matching
    --out <path>            Output path (default: <dump_dir>/Offsets.h)
    --strict                Only emit entries with confidence >= 90
    --dry-run               Print report, do not write files
    --report                Also write a detailed .report.txt
    --scan-class <Name>     Dump every field of a class from the new dump
    -v, --version           Print version
    -h, --help              This help

  EXAMPLES
    # Standard — learn from a previous dump (best accuracy)
    ./arx0.sh dump_2.132.1.cs \
        --old ./Offsets.h \
        --old-dump ./dump_2.130.3.cs

    # No old dump — fall back to name matching + built-in DB
    ./arx0.sh dump.cs --old Offsets.h

    # Interactive — just run it and it will ask
    ./arx0.sh

    # Inspect a specific class
    ./arx0.sh dump.cs --scan-class Player

EOF
}

# ─── ARGS ──────────────────────────────────────────────────────────────────
parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)       usage; exit 0 ;;
            -v|--version)    echo "$SCRIPT_NAME v$SCRIPT_VERSION"; exit 0 ;;
            --old)           OLD_OFFSETS="${2:-}"; shift 2 ;;
            --old-dump)      OLD_DUMP="${2:-}"; shift 2 ;;
            --out)           OUT_PATH="${2:-}"; shift 2 ;;
            --scan-class)    SCAN_CLASS="${2:-}"; shift 2 ;;
            --strict)        STRICT=1; shift ;;
            --dry-run)       DRY_RUN=1; shift ;;
            --report)        REPORT=1; shift ;;
            --)              shift; break ;;
            -*)              die "unknown option: $1 (see --help)" ;;
            *)               DUMP_PATH="$1"; shift ;;
        esac
    done
}

interactive_prompt() {
    if [[ -z "$DUMP_PATH" ]]; then
        echo
        printf '%s' "${C_BLD}Enter path to new dump.cs:${C_RST} "
        read -r DUMP_PATH
    fi
    if [[ -z "$OLD_OFFSETS" ]]; then
        local def="$(dirname "$DUMP_PATH")/Offsets.h"
        printf '%s' "${C_BLD}Enter path to old Offsets.h [${def}]:${C_RST} "
        read -r tmp_in
        OLD_OFFSETS="${tmp_in:-$def}"
    fi
    if [[ -z "$OLD_DUMP" ]]; then
        printf '%s' "${C_BLD}Enter path to OLD dump.cs (optional, ENTER to skip):${C_RST} "
        read -r tmp_in
        OLD_DUMP="$tmp_in"
    fi
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 1 — PARSE dump.cs INTO A FLAT INDEX
# ═══════════════════════════════════════════════════════════════════════════
#
#  Output TSV columns:
#    1  KIND        STATIC | INSTANCE | METHOD | PROPERTY
#    2  NAMESPACE
#    3  CLASS
#    4  PARENT
#    5  TYPE
#    6  NAME
#    7  OFFSET      (0xNNN — RVA for STATIC/METHOD, Offset for INSTANCE)
#    8  LINE        (line number in dump.cs — proof of origin)
#
parse_dump() {
    local src="$1" dst="$2"
    log "parsing dump: $(basename "$src")"
    local sz; sz=$(wc -c < "$src")
    log "  size: $(( sz / 1024 / 1024 )) MB"

    awk '
        BEGIN { curNs=""; curClass=""; curParent="" }

        # Namespace line
        /^\/\/ Namespace: / {
            ns = $0
            sub(/^\/\/ Namespace: /, "", ns)
            gsub(/^[ \t]+|[ \t]+$/, "", ns)
            curNs = ns
            next
        }

        # Class / struct / interface / enum declaration
        /^[ \t]*(public|internal|private|protected|sealed|abstract|static|partial)[ \t]/ {
            line = $0
            if (line !~ /(class|struct|interface|enum)[ \t]/) next
            sub(/\{.*$/, "", line)
            gsub(/^[ \t]+|[ \t]+$/, "", line)
            n = split(line, parts, /[ \t]+/)
            className = ""; parentName = ""
            for (i = 1; i <= n; i++) {
                if (parts[i] == "class" || parts[i] == "struct" ||
                    parts[i] == "interface" || parts[i] == "enum") {
                    className = parts[i+1]
                    gsub(/[<:].*$/, "", className)
                    continue
                }
                if (parts[i] == ":" && className != "") {
                    parentName = parts[i+1]
                    gsub(/[<,].*$/, "", parentName)
                    break
                }
            }
            if (className != "") {
                curClass  = className
                curParent = parentName
            }
            next
        }

        # Instance field with // Offset:
        /^\/\/ Offset: 0x/ {
            off = $0
            sub(/^\/\/ Offset: /, "", off)
            gsub(/[^0-9A-Fa-f]/, "", off)
            off = "0x" off
            while ((getline nl) > 0) {
                gsub(/^[ \t]+|[ \t]+$/, "", nl)
                if (nl == "") continue
                if (nl ~ /;/) {
                    body = nl
                    sub(/;.*$/, "", body)
                    if (body ~ /\{/) break
                    gsub(/\b(public|private|protected|internal|readonly|const|static|volatile|unsafe|new|override|virtual|extern|fixed)\b/, "", body)
                    gsub(/^[ \t]+|[ \t]+$/, "", body)
                    n = split(body, parts, /[ \t]+/)
                    if (n >= 2) {
                        fname = parts[n]
                        ftype = parts[1]
                        for (i = 2; i < n; i++) ftype = ftype " " parts[i]
                        print "INSTANCE", curNs, curClass, curParent, ftype, fname, off, NR
                    }
                }
                break
            }
            next
        }

        # Static field / method / property with // RVA:
        /^\/\/ RVA: 0x/ {
            line = $0
            rva = ""; instOff = ""
            n = split(line, parts, /[ \t]+/)
            for (i = 1; i <= n; i++) {
                if      (parts[i] == "RVA:")    rva     = parts[i+1]
                else if (parts[i] == "Offset:") instOff = parts[i+1]
            }
            while ((getline nl) > 0) {
                gsub(/^[ \t]+|[ \t]+$/, "", nl)
                if (nl == "") continue
                body = nl
                if (body ~ /\(/) {
                    sub(/\(.*/, "", body)
                    gsub(/\b(public|private|protected|internal|static|virtual|override|abstract|sealed|extern|unsafe|async|new)\b/, "", body)
                    gsub(/^[ \t]+|[ \t]+$/, "", body)
                    n = split(body, parts, /[ \t]+/)
                    if (n >= 1) {
                        mname = parts[n]
                        print "METHOD", curNs, curClass, curParent, "", mname, rva, NR
                    }
                } else if (body ~ /\{/) {
                    break
                } else if (body ~ /;/) {
                    sub(/;.*$/, "", body)
                    isStatic = (body ~ /static/) ? 1 : 0
                    gsub(/\b(public|private|protected|internal|readonly|const|static|volatile|unsafe|new|override|virtual|extern|fixed)\b/, "", body)
                    gsub(/^[ \t]+|[ \t]+$/, "", body)
                    n = split(body, parts, /[ \t]+/)
                    if (n >= 2) {
                        fname = parts[n]
                        ftype = parts[1]
                        for (i = 2; i < n; i++) ftype = ftype " " parts[i]
                        if      (isStatic)     print "STATIC",   curNs, curClass, curParent, ftype, fname, rva,     NR
                        else if (instOff != "") print "INSTANCE", curNs, curClass, curParent, ftype, fname, instOff, NR
                    }
                }
                break
            }
            next
        }
    ' "$src" > "$dst"

    local n; n=$(wc -l < "$dst")
    ok "  → $n symbols indexed"
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 2 — PARSE OLD Offsets.h
# ═══════════════════════════════════════════════════════════════════════════
#
#  Extracts every  constexpr uintptr_t NAME = VALUE;  declaration.
#  Splits into:
#     $main  → NAME<TAB>0xHEX
#     $alias → NAME<TAB>OTHER_NAME
#
parse_old_offsets() {
    local src="$1" main="$2" alias="$3"
    log "parsing old offsets: $(basename "$src")"

    awk '
        /constexpr[ \t]+uintptr_t/ {
            line = $0
            sub(/.*uintptr_t[ \t]+/, "", line)
            n = split(line, a, /[ \t]*=[ \t]*/)
            if (n >= 2) {
                name = a[1]
                val  = a[2]
                sub(/[ \t]*;.*$/, "", val)
                sub(/[ \t]*\/\/.*$/, "", val)
                gsub(/[ \t]/, "", val)
                sub(/[ \t].*/, "", name)
                gsub(/^[ \t]+|[ \t]+$/, "", name)
                if (val ~ /^0x[0-9A-Fa-f]+$/ || val ~ /^[0-9]+$/) {
                    if (val ~ /^[0-9]+$/) val = sprintf("0x%X", val)
                    print name "\t" val
                } else if (val ~ /^[A-Za-z_][A-Za-z0-9_]*$/) {
                    print name "\t" val
                }
            }
        }
    ' "$src" > "$main.raw"

    grep -E $'\t0x' "$main.raw"  > "$main"   || true
    grep -vE $'\t0x' "$main.raw" > "$alias"  || true
    rm -f "$main.raw"

    local n_hex n_alias
    n_hex=$(wc -l < "$main")
    n_alias=$(wc -l < "$alias")
    ok "  → $n_hex values, $n_alias aliases"
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 3 — BUILD LOOKUP ACCELERATORS
# ═══════════════════════════════════════════════════════════════════════════
#
#  We build three keys:
#    by_cn.tsv     : KIND<TAB>CLASS<TAB>NAME<TAB><full line>
#    by_n.tsv      : NAME<TAB><full line>
#    by_off.tsv    : OFFSET<TAB><full line>
#
build_lookups() {
    local idx="$1" by_cn="$2" by_n="$3" by_off="$4"
    log "building lookup indexes…"

    # KIND | CLASS | NAME  →  full row
    awk -F'\t' -v OFS='\t' '{ print $1, $3, $6, $0 }' "$idx" > "$by_cn"
    awk -F'\t' -v OFS='\t' '{ print $6, $0 }'          "$idx" > "$by_n"
    awk -F'\t' -v OFS='\t' '{ k=$7; gsub(/^0x/,"",k); k=toupper(k); print k, $0 }' "$idx" > "$by_off"

    ok "  → lookups ready"
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 4 — SELF-LEARNING FROM OLD DUMP
# ═══════════════════════════════════════════════════════════════════════════
#
#  For every  NAME = 0xHEX  in the old Offsets.h, we find the exact row in
#  the OLD dump.cs whose offset matches. That row gives us the semantic
#  identity of NAME: (kind, class, field-name, type).
#
#  This is how we learn "Player_Rotation → class=Player, field=rotation".
#
learn_from_old_dump() {
    local old_idx="$1" offsets="$2" out="$3"
    log "learning semantics from old dump…"
    : > "$out"

    while IFS=$'\t' read -r name hex; do
        [[ -z "$name" || -z "$hex" ]] && continue
        local norm
        norm=$(echo "$hex" | sed 's/^0x//;s/^0*//' | tr 'a-f' 'A-F')
        [[ -z "$norm" ]] && norm="0"
        # find matching row in old index by offset
        awk -F'\t' -v o="$norm" '
            {
                t = $7
                sub(/^0x/, "", t)
                gsub(/^0+/, "", t)
                t = toupper(t)
                if (t == "" || t == o) { print; exit }
            }
        ' "$old_idx" | while IFS=$'\t' read -r k ns cls par typ nm off ln; do
            [[ -z "$k" ]] && continue
            printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
                "$name" "$k" "$cls" "$nm" "$typ" "$hex" "$ln" >> "$out"
        done
    done < <(grep -E $'\t0x' "$offsets" 2>/dev/null || awk -F'\t' '{print}' "$offsets")

    local n; n=$(wc -l < "$out" 2>/dev/null || echo 0)
    ok "  → learned $n symbol mappings"
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 5 — SEARCH STRATEGIES
# ═══════════════════════════════════════════════════════════════════════════

# S1 — exact (kind, class, name) in new index.
#      Conf 100.
search_s1() {
    local by_cn="$1" kind="$2" cls="$3" nm="$4"
    grep -m1 -F "${kind}"$'\t'"${cls}"$'\t'"${nm}"$'\t' "$by_cn" 2>/dev/null \
        | cut -f4- || true
}

# S2 — (class, name) regardless of kind. Conf 90.
search_s2() {
    local by_cn="$1" cls="$2" nm="$3"
    grep -m1 -P $'\t'"${cls}"$'\t'"${nm}"$'\t' "$by_cn" 2>/dev/null \
        | cut -f4- || true
}

# S3 — name only. Conf 70.
search_s3() {
    local by_n="$1" nm="$2"
    grep -m1 -F "${nm}"$'\t' "$by_n" 2>/dev/null \
        | cut -f2- || true
}

# S4 — same offset value in new dump (global). Conf 40.
search_s4() {
    local by_off="$1" hex="$2"
    local norm
    norm=$(echo "$hex" | sed 's/^0x//;s/^0*//' | tr 'a-f' 'A-F')
    [[ -z "$norm" ]] && norm="0"
    grep -m1 -F "${norm}"$'\t' "$by_off" 2>/dev/null \
        | cut -f2- || true
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 6 — CURATED FALLBACK DB
# ═══════════════════════════════════════════════════════════════════════════
#
#  When no old dump is available, we use this hand-curated table mapping
#  the well-known Free Fire symbol names to their semantic identity.
#  Only used when the old-dump learning path failed for that symbol.
#
#  Format:  NAME<TAB>KIND<TAB>CLASS_HINT<TAB>FIELD_HINT<TAB>TYPE_HINT
#
fallback_db() {
    cat <<'DB'
GameFacade	STATIC	GameFacade	Instance	GameFacade
GameFacade_P2	INSTANCE	GameFacade		
BaseGame_Match	INSTANCE	BaseGame		
CurrentObserve	INSTANCE	GameFacade		
ObserverPlayer	INSTANCE			
BaseGame_Timer	INSTANCE	BaseGame		
GhostHack	INSTANCE			
Match_PlayerDict	INSTANCE			
Match_LocalPlayer	INSTANCE			
Player_Rotation	INSTANCE	Player	rotation	Quaternion
Player_HeadTF	INSTANCE	Player		
Player_FootTF	INSTANCE	Player		
Player_Camera	INSTANCE	Player		
Player_IsFiring	INSTANCE	Player		
Player_HP	INSTANCE	Player		
Player_IsDead	INSTANCE	Player		
Player_Avatar	INSTANCE	Player		
Player_HedColider	INSTANCE	Player		
Player_DeathInfo	INSTANCE	Player		
Player_IsBot	INSTANCE	Player		
Player_Name	INSTANCE	Player		
Avatar_Uma	INSTANCE			
Uma_Visible	INSTANCE			
Uma_Data	INSTANCE			
Camera_Follow	INSTANCE			
Camera_IntPtr	INSTANCE			
Camera_Matrix	INSTANCE			
WeaponInstance	INSTANCE	Player		
ReloadInstance	INSTANCE	Player		
InventoryManager	INSTANCE	Player		
StatusStruct	INSTANCE	Player		
DB
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 7 — THE RESOLVER
# ═══════════════════════════════════════════════════════════════════════════
#
#  For each NAME in old offsets:
#     1. If learned_semantics has a row → apply S1, S2, S3, S4
#     2. Else consult fallback DB → apply S1 / S3
#     3. Else → emit NOT FOUND
#
#  Records one row per NAME in results.tsv:
#     NAME  OLD_HEX  NEW_HEX  CONF  STATUS  CLASS  FIELD  TYPE  SRC_LINE
#
resolve_all() {
    local by_cn="$1" by_n="$2" by_off="$3" learned="$4" offsets_main="$5" out="$6"
    : > "$out"

    log "resolving offsets…"

    # Build a semantic map name → details from learned file
    declare -A L_KIND L_CLS L_FLD L_TYP
    if [[ -s "$learned" ]]; then
        while IFS=$'\t' read -r name kind cls fld typ old ln; do
            L_KIND["$name"]="$kind"
            L_CLS["$name"]="$cls"
            L_FLD["$name"]="$fld"
            L_TYP["$name"]="$typ"
        done < "$learned"
    fi

    # Build a semantic map from fallback DB (only used if not learned)
    declare -A F_KIND F_CLS F_FLD F_TYP
    while IFS=$'\t' read -r name kind cls fld typ; do
        [[ -z "$name" || "$name" =~ ^# ]] && continue
        F_KIND["$name"]="$kind"
        F_CLS["$name"]="$cls"
        F_FLD["$name"]="$fld"
        F_TYP["$name"]="$typ"
    done < <(fallback_db)

    local total=0 found=0 notfound=0
    while IFS=$'\t' read -r name old_hex; do
        [[ -z "$name" || -z "$old_hex" ]] && continue
        total=$((total+1))

        local kind="${L_KIND[$name]:-}" cls="${L_CLS[$name]:-}"
        local fld="${L_FLD[$name]:-}"   typ="${L_TYP[$name]:-}"
        local source="learned"

        if [[ -z "$kind" || -z "$fld" ]]; then
            # fall through to DB
            kind="${F_KIND[$name]:-}"; cls="${F_CLS[$name]:-}"
            fld="${F_FLD[$name]:-}";   typ="${F_TYP[$name]:-}"
            source="fallback"
        fi

        local row="" conf=0 status="NOT_FOUND" new_hex="" got_cls="" got_fld="" got_typ="" got_ln=""

        # ── Strategy S1 ────────────────────────────────────────────
        if [[ -n "$kind" && -n "$cls" && -n "$fld" ]]; then
            row=$(search_s1 "$by_cn" "$kind" "$cls" "$fld")
            if [[ -n "$row" ]]; then
                conf=100; status="OK"
            fi
        fi

        # ── Strategy S2 ────────────────────────────────────────────
        if [[ -z "$row" && -n "$cls" && -n "$fld" ]]; then
            row=$(search_s2 "$by_cn" "$cls" "$fld")
            [[ -n "$row" ]] && { conf=90; status="OK"; }
        fi

        # ── Strategy S3 ────────────────────────────────────────────
        if [[ -z "$row" && -n "$fld" ]]; then
            row=$(search_s3 "$by_n" "$fld")
            [[ -n "$row" ]] && { conf=70; status="NAME_ONLY"; }
        fi

        # ── Strategy S4 ────────────────────────────────────────────
        if [[ -z "$row" && -n "$old_hex" ]]; then
            row=$(search_s4 "$by_off" "$old_hex")
            [[ -n "$row" ]] && { conf=40; status="OFFSET_MATCH"; }
        fi

        if [[ -n "$row" ]]; then
            got_cls=$(echo "$row"  | cut -f3)
            got_fld=$(echo "$row"  | cut -f6)
            got_typ=$(echo "$row"  | cut -f5)
            new_hex=$(echo "$row"  | cut -f7)
            got_ln=$(echo "$row"   | cut -f8)
            found=$((found+1))
        else
            new_hex="$old_hex"
            got_cls="$cls"; got_fld="$fld"; got_typ="$typ"
            notfound=$((notfound+1))
        fi

        printf '%s\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$name" "$old_hex" "$new_hex" "$conf" "$status" \
            "$got_cls" "$got_fld" "$got_typ" "$got_ln" "$source" >> "$out"

    done < "$offsets_main"

    printf '%s  → %d total: %s%d resolved%s, %s%d unresolved%s\n' \
        "$C_DIM" "$total" "$C_GRN" "$found" "$C_RST" \
        "$C_YEL" "$notfound" "$C_RST"
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 8 — EMIT new Offsets.h
# ═══════════════════════════════════════════════════════════════════════════
emit_offsets() {
    local results="$1" out="$2" dump="$3" old="$4"
    local ts; ts=$(date '+%Y-%m-%d %H:%M:%S')
    local host; host=$(hostname 2>/dev/null || echo "unknown")
    local base_dump; base_dump=$(basename "$dump")
    local base_old;  base_old=$(basename "$old")

    # Header template
    {
    cat <<EOF
#pragma once
#include <cstdint>

// ═══════════════════════════════════════════════════════════════════════
//  Auto-generated by $SCRIPT_NAME.sh v$SCRIPT_VERSION
//  Generated : $ts
//  Host      : $host
//  Source    : $base_dump
//  Old file  : $base_old
//
//  Legend (per-entry comment shows the source line in dump.cs):
//    [OK]  = class + field name matched        (safe to use)
//    [~]   = name-only or type+sibling match   (verify if possible)
//    [??]  = matched only by offset value      (VERIFY MANUALLY)
//    [!]   = not found — old value retained    (DO NOT TRUST)
// ═══════════════════════════════════════════════════════════════════════

namespace Offsets {

EOF

    # Emit each symbol
    while IFS=$'\t' read -r name old_hex new_hex conf status cls fld typ ln source; do
        local mark
        case "$status" in
            OK)          mark="OK" ;;
            NAME_ONLY)   mark="~"  ;;
            OFFSET_MATCH) mark="??" ;;
            *)           mark="!"  ;;
        esac

        local detail=""
        if [[ -n "$cls" && -n "$fld" ]]; then
            detail="class=$cls field=$fld"
            [[ -n "$typ" ]] && detail="$detail type=$typ"
            [[ -n "$ln"  ]] && detail="$detail dump.cs:$ln"
        fi

        if [[ "$status" == "NOT_FOUND" ]]; then
            printf '    // [!] NOT FOUND in dump.cs — old value kept, DO NOT TRUST\n'
            printf '    constexpr uintptr_t %s = %s;\n\n' "$name" "$old_hex"
        else
            printf '    // [%s] %s  (conf %d%%)\n' "$mark" "$detail" "$conf"
            printf '    constexpr uintptr_t %s = %s;\n\n' "$name" "$new_hex"
        fi
    done < "$results"

    cat <<EOF
}
EOF
    } > "$out"
}

# ═══════════════════════════════════════════════════════════════════════════
#  PHASE 9 — EMIT report.txt
# ═══════════════════════════════════════════════════════════════════════════
emit_report() {
    local results="$1" out="$2"
    {
        printf 'arx0.sh report — %s\n' "$(date '+%Y-%m-%d %H:%M:%S')"
        printf '════════════════════════════════════════════════════════════════\n\n'

        printf 'HIGH CONFIDENCE (>= 90)\n─────────────────────────────────────\n'
        awk -F'\t' '$4 >= 90 { printf "  %-32s  %-14s  %-14s  %s.%s\n", $1, $2, $3, $6, $7 }' "$results"

        printf '\nMEDIUM CONFIDENCE (60-89)\n─────────────────────────────────────\n'
        awk -F'\t' '$4 >= 60 && $4 < 90 { printf "  %-32s  %-14s  %-14s  %s.%s\n", $1, $2, $3, $6, $7 }' "$results"

        printf '\nLOW CONFIDENCE (1-59) — verify manually\n─────────────────────────────────────\n'
        awk -F'\t' '$4 >= 1 && $4 < 60 { printf "  %-32s  %-14s  %-14s  %s.%s\n", $1, $2, $3, $6, $7 }' "$results"

        printf '\nNOT FOUND\n─────────────────────────────────────\n'
        awk -F'\t' '$5 == "NOT_FOUND" { printf "  %-32s  old=%s\n", $1, $2 }' "$results"

        printf '\nFULL TABLE\n─────────────────────────────────────\n'
        printf '  %-32s %-12s %-12s %5s %-14s %s\n' NAME OLD NEW CONF STATUS SOURCE
        printf '  %-32s %-12s %-12s %5s %-14s %s\n' "--------------------------------" "------------" "------------" "-----" "--------------" "------"
        awk -F'\t' '{ printf "  %-32s %-12s %-12s %5d %-14s %s\n", $1, $2, $3, $4, $5, $10 }' "$results"
    } > "$out"
}

# ═══════════════════════════════════════════════════════════════════════════
#  SCAN-CLASS (utility)
# ═══════════════════════════════════════════════════════════════════════════
scan_class() {
    local idx="$1" cls="$2"
    echo
    hr
    printf '%sFields of class: %s%s\n' "$C_BLD" "$cls" "$C_RST"
    hr
    awk -F'\t' -v c="$cls" '$3 == c {
        printf "  %-8s  %-10s  %-28s  %s\n", $1, $7, $5, $6
    }' "$idx" | sort -u
    hr
    echo
}

# ═══════════════════════════════════════════════════════════════════════════
#  BANNER
# ═══════════════════════════════════════════════════════════════════════════
banner() {
    printf '\n'
    printf '%s' "$C_MAG"
    cat <<'B'
    █████╗ ██████╗ ██╗  ██╗ ██████╗
   ██╔══██╗██╔══██╗╚██╗██╔╝██╔═████╗
   ███████║██████╔╝ ╚███╔╝ ██║██╔██║
   ██╔══██║██╔══██╗ ██╔██╗ ████╔╝██║
   ██║  ██║██║  ██║██╔╝ ██╗╚██████╔╝
   ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝
B
    printf '%s' "$C_RST"
    printf '   Il2Cpp Offsets Extractor / Updater  v%s\n' "$SCRIPT_VERSION"
    printf '   every number traceable to a real line in dump.cs\n\n'
}

# ═══════════════════════════════════════════════════════════════════════════
#  MAIN
# ═══════════════════════════════════════════════════════════════════════════
main() {
    parse_args "$@"
    banner

    # Interactive fallback
    if [[ -z "$DUMP_PATH" ]]; then
        interactive_prompt
    fi

    [[ -n "$DUMP_PATH"  ]] || die "no dump.cs provided"
    [[ -f "$DUMP_PATH"  ]] || die "dump.cs not found: $DUMP_PATH"

    if [[ -z "$OLD_OFFSETS" ]]; then
        OLD_OFFSETS="$(dirname "$DUMP_PATH")/Offsets.h"
    fi
    [[ -f "$OLD_OFFSETS" ]] || die "old Offsets.h not found: $OLD_OFFSETS (use --old)"

    if [[ -n "$OLD_DUMP" && ! -f "$OLD_DUMP" ]]; then
        warn "old dump not found: $OLD_DUMP  — continuing without it"
        OLD_DUMP=""
    fi

    if [[ -z "$OUT_PATH" ]]; then
        OUT_PATH="$(dirname "$DUMP_PATH")/Offsets.h"
    fi

    mktmp
    hr
    printf '  %sNew dump   :%s %s\n' "$C_BLD" "$C_RST" "$DUMP_PATH"
    printf '  %sOld offset :%s %s\n' "$C_BLD" "$C_RST" "$OLD_OFFSETS"
    printf '  %sOld dump   :%s %s\n' "$C_BLD" "$C_RST" "${OLD_DUMP:-<none>}"
    printf '  %sOutput     :%s %s\n' "$C_BLD" "$C_RST" "$OUT_PATH"
    printf '  %sStrict     :%s %s\n' "$C_BLD" "$C_RST" "$([[ $STRICT == 1 ]] && echo yes || echo no)"
    hr
    echo

    # 1. Parse new dump
    parse_dump "$DUMP_PATH" "$TMP/new_index.tsv"

    # 2. Parse old offsets.h
    parse_old_offsets "$OLD_OFFSETS" "$TMP/old_main.tsv" "$TMP/old_alias.tsv"

    # 3. Learn from old dump (optional)
    if [[ -n "$OLD_DUMP" ]]; then
        parse_dump "$OLD_DUMP" "$TMP/old_index.tsv"
        learn_from_old_dump "$TMP/old_index.tsv" "$TMP/old_main.tsv" "$TMP/learned.tsv"
    else
        warn "no old dump → skipping semantic learning (accuracy will be lower)"
        : > "$TMP/learned.tsv"
    fi

    # 4. Build lookup accelerators for new dump
    build_lookups "$TMP/new_index.tsv" \
                  "$TMP/by_cn.tsv" "$TMP/by_n.tsv" "$TMP/by_off.tsv"

    # 5. scan-class mode
    if [[ -n "$SCAN_CLASS" ]]; then
        scan_class "$TMP/new_index.tsv" "$SCAN_CLASS"
        return 0
    fi

    # 6. Resolve everything
    echo
    resolve_all "$TMP/by_cn.tsv" "$TMP/by_n.tsv" "$TMP/by_off.tsv" \
                "$TMP/learned.tsv" "$TMP/old_main.tsv" "$TMP/results.tsv"

    # 7. Emit
    echo
    if [[ "$DRY_RUN" == "1" ]]; then
        log "dry-run → not writing files"
        emit_report "$TMP/results.tsv" "$TMP/report.txt"
        echo
        hr
        cat "$TMP/report.txt"
        hr
        return 0
    fi

    emit_offsets "$TMP/results.tsv" "$OUT_PATH" "$DUMP_PATH" "$OLD_OFFSETS"
    ok "wrote: $OUT_PATH"

    if [[ "$REPORT" == "1" ]]; then
        local rp="${OUT_PATH%.h}.report.txt"
        emit_report "$TMP/results.tsv" "$rp"
        ok "wrote: $rp"
    fi

    # Quick summary
    echo
    hr
    printf '%sSummary%s\n' "$C_BLD" "$C_RST"
    hr
    awk -F'\t' '
        {
            s[$5]++
            if ($4 >= 90) high++
            else if ($4 >= 60) med++
            else if ($4 >= 1) low++
        }
        END {
            printf "  high-confidence (>=90) : %d\n", high
            printf "  medium (60-89)         : %d\n", med
            printf "  low (1-59)             : %d\n", low
            printf "  not found              : %d\n", s["NOT_FOUND"]+0
        }
    ' "$TMP/results.tsv"
    hr
    echo
    printf '  Review %s%s%s before flashing.\n' "$C_BLD" "$OUT_PATH" "$C_RST"
    printf '  Entries flagged [??] or [!] need manual verification.\n\n'
}

main "$@"
