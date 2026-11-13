#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define GNUGER_VERSION "0.3.0"
#define MAX_DETAIL 200
#define MAX_LINE 256

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#define BSD_BASE 1
#endif

#ifdef BSD_BASE
#define GNU_TOOL(name) "g" name
#else
#define GNU_TOOL(name) name
#endif

struct entry {
    const char *name;
    const char *category;
    int weight;
    int antibody;
    const char *replace;
    const char *const *probes;
};

static const char *color_gray = "";
static const char *color_white = "";
static const char *color_reset = "";

#define PROBES(...) ((const char *const[]){ __VA_ARGS__, NULL })

static const char *const bmake_probes[] = {
    "bmake",
    "pmake",
#ifdef BSD_BASE
    "make",
#endif
    NULL
};

static const char *const byacc_probes[] = {
    "byacc",
#ifdef BSD_BASE
    "yacc",
#endif
    NULL
};

static const char *const elftoolchain_probes[] = {
#ifdef BSD_BASE
    "objdump", "objcopy", "readelf", "ranlib", "strip", "nm", "ar", "size", "addr2line",
#endif
    NULL
};

static const char *const bsdtar_probes[] = {
    "bsdtar",
    "bsdcpio",
#ifdef BSD_BASE
    "tar",
#endif
    NULL
};

static const char *const libc_probes[] = {
#ifdef BSD_BASE
    "/usr/lib/libc.a",
#endif
    NULL
};

static const char *const bsd_loader_probes[] = {
#ifdef BSD_BASE
    "/boot/loader.conf",
#endif
    NULL
};

static const struct entry entries[] = {
    { "glibc", "library", 20, 0, "musl (MIT), libc (BSD)",
      PROBES("/lib/x86_64-linux-gnu/libc.so.6", "/lib/i386-linux-gnu/libc.so.6",
             "/lib64/libc.so.6", "/lib/libc.so.6", "/usr/lib64/libc.so.6", "/usr/lib/libc.so.6") },
    { "gcc", "compiler", 15, 0, "clang (Apache2.0), pcc (BSD), cproc (ISC), tcc (LGPL2.1)",
      PROBES("gcc", "gcc-14", "gcc-13", "gcc-12", "gcc14", "gcc13", "gcc12", "gcc11", "gcc10") },
    { "binutils", "toolchain", 12, 0, "elftoolchain (BSD), llvm (Apache2.0)",
      PROBES(GNU_TOOL("readelf"), GNU_TOOL("objcopy"), GNU_TOOL("objdump"), GNU_TOOL("ranlib"),
             GNU_TOOL("strip"), GNU_TOOL("nm"), GNU_TOOL("ar"), GNU_TOOL("as")) },
    { "coreutils", "utils", 12, 0, "toybox (BSD0), busybox (GPL2.0), sbase (ISC)",
      PROBES(GNU_TOOL("stdbuf"), GNU_TOOL("chcon"), GNU_TOOL("csplit"), GNU_TOOL("shred"),
             GNU_TOOL("ptx"), GNU_TOOL("numfmt"), GNU_TOOL("tsort"), GNU_TOOL("truncate"),
             GNU_TOOL("fmt"), GNU_TOOL("join")) },
    { "busybox", "utils", 12, 1, NULL, PROBES("busybox") },
    { "bash", "shell", 8, 0, "mksh (MirOS), oksh (BSD), zsh", PROBES("bash") },
    { "grub", "bootloader", 8, 0, "limine (ISC), bsd loader (BSD)",
      PROBES("grub-install", "grub-mkconfig", "grub2-install", "grub2-mkconfig", "grub-mkrescue",
             "/etc/default/grub", "/etc/grub.d", "/boot/grub2", "/boot/grub") },
    { "emacs", "editor", 8, 0, "atto (BSD3), nvi (BSD3), vis (ISC), mg (BSD), vim (Vim)",
      PROBES("emacs", "emacsclient") },
    { "libstdc++", "library", 6, 0, "libc++ (Apache2.0)",
      PROBES("/usr/include/c++", "/lib64/libstdc++.so.6", "/usr/lib64/libstdc++.so.6",
             "/usr/lib/libstdc++.so.6", "/usr/local/lib/libstdc++.so") },
    { "readline", "library", 6, 0, "libedit (BSD), editline (BSD)",
      PROBES("/usr/include/readline/readline.h", "/usr/local/include/readline/readline.h",
             "/lib/x86_64-linux-gnu/libreadline.so.8", "/lib64/libreadline.so.8",
             "/usr/lib64/libreadline.so", "/usr/lib/libreadline.so", "/usr/local/lib/libreadline.so") },
    { "gdb", "debugger", 6, 0, "lldb (Apache2.0)", PROBES("gdb", "gdbserver") },
    { "make", "build", 6, 0, "bmake (BSD), samurai (ISC), just (CC0)",
      PROBES(GNU_TOOL("make"), "gmake") },
    { "gawk", "scripting", 5, 0, "nawk (BSD), goawk (MIT)", PROBES("gawk") },
    { "grep", "utils", 5, 0, "ugrep (BSD3), ripgrep (MIT), BSD grep",
      PROBES(GNU_TOOL("grep"), GNU_TOOL("egrep"), GNU_TOOL("fgrep")) },
    { "sed", "utils", 5, 0, "BSD sed, sd (Unlicense)", PROBES(GNU_TOOL("sed")) },
    { "gettext", "i18n", 5, 0, "tinygettext (zlib), gettext-tiny (MIT)",
      PROBES("gettext", "xgettext", "msgfmt", "msgmerge") },
    { "autoconf", "build", 5, 0, "muon (MIT)",
      PROBES("autoconf", "automake", "autoreconf") },
    { "fish", "shell", 5, 1, NULL, PROBES("fish") },
    { "gnutls", "library", 4, 0, "openssl (Apache2.0), libressl (BSD)",
      PROBES("/usr/include/gnutls/gnutls.h", "/usr/local/include/gnutls/gnutls.h",
             "/lib/x86_64-linux-gnu/libgnutls.so.30", "/lib64/libgnutls.so.30",
             "/usr/lib/libgnutls.so", "/usr/local/lib/libgnutls.so") },
    { "tar", "archiving", 4, 0, "bsdtar (BSD), libarchive (BSD)", PROBES(GNU_TOOL("tar")) },
    { "diffutils", "utils", 4, 0, "BSD diff",
      PROBES(GNU_TOOL("diff"), GNU_TOOL("diff3"), GNU_TOOL("cmp"), GNU_TOOL("sdiff")) },
    { "findutils", "utils", 4, 0, "BSD find, fd (Apache2.0)", PROBES(GNU_TOOL("find"), GNU_TOOL("xargs")) },
    { "gimp", "graphics", 4, 0, "Pinta (MIT), PhotoDemon (BSD)",
      PROBES("gimp", "gimp-2.10", "gimp-2.99") },
    { "u-boot", "bootloader", 4, 1, NULL,
      PROBES("mkimage", "dumpimage") },
    { "libtool", "build", 3, 0, "cmake (BSD)", PROBES("libtool", "libtoolize") },
    { "nano", "editor", 3, 0, "micro (MIT)", PROBES("nano") },
    { "gzip", "compression", 3, 0, "xz (public domain), zstd (BSD), bzip2",
      PROBES(GNU_TOOL("gzip"), GNU_TOOL("gunzip"), GNU_TOOL("zcat")) },
    { "info", "docs", 3, 0, "mandoc (BSD)", PROBES("info", "texi2any", "makeinfo", "install-info") },
    { "m4", "toolchain", 3, 0, "GPP (BSD), Gema (BSD), BSD m4", PROBES(GNU_TOOL("m4")) },
    { "bison", "parser", 3, 0, "byacc (BSD)", PROBES("bison") },
    { "patch", "utils", 3, 0, "BSD patch", PROBES(GNU_TOOL("patch")) },
    { "screen", "terminal", 3, 0, "tmux (ISC)", PROBES("screen") },
    { "guile", "scheme", 3, 0, "lua (MIT), gauche (BSD)", PROBES("guile", "guile3.0", "guile2.2") },
    { "octave", "numerics", 3, 0, "julia (MIT)", PROBES("octave", "octave-cli") },
    { "wget", "network", 3, 0, "curl (MIT)", PROBES("wget") },
    { "cvs", "vcs", 3, 0, "fossil (BSD2)", PROBES("cvs") },
    { "parted", "disk", 3, 0, "gpart (BSD)", PROBES("parted", "gparted") },
    { "udev", "device", 3, 0, "mdevd (ISC)", PROBES("udevadm", "systemd-udevd", "udevd") },
    { "mawk", "scripting", 3, 1, NULL, PROBES("mawk") },
    { "aria2", "network", 3, 1, NULL, PROBES("aria2c", "aria2") },
    { "indent", "utils", 2, 0, "clang-format (Apache2.0)", PROBES(GNU_TOOL("indent")) },
    { "gperf", "utils", 2, 0, "nbperf (BSD), frozen (Apache2.0)", PROBES("gperf") },
    { "gnucash", "finance", 2, 0, "Actual Budget (MIT)", PROBES("gnucash") },
    { "gmp", "library", 2, 0, "libtommath (public domain), OpenSSL BIGNUM",
      PROBES("/usr/include/gmp.h", "/usr/local/include/gmp.h", "/usr/lib/libgmp.so",
             "/usr/local/lib/libgmp.so") },
    { "mpfr", "library", 2, 0, "LibBF (MIT), TTMath (BSD)",
      PROBES("/usr/include/mpfr.h", "/usr/local/include/mpfr.h", "/usr/lib/libmpfr.so",
             "/usr/local/lib/libmpfr.so") },
    { "yash", "shell", 2, 1, NULL, PROBES("yash") },

    { "musl", "library", 20, 1, NULL,
      PROBES("/lib/ld-musl-x86_64.so.1", "/lib/ld-musl-aarch64.so.1", "/lib/ld-musl-i386.so.1",
             "/lib/ld-musl-armv7.so.1", "/lib/ld-musl-riscv64.so.1",
             "/lib/libc.musl-x86_64.so.1", "/usr/lib/libc.musl-x86_64.so.1",
             "/usr/local/musl/lib/libc.so") },
    { "libc", "library", 20, 1, NULL, libc_probes },
    { "clang", "compiler", 15, 1, NULL, PROBES("clang", "clang++") },
    { "elftoolchain", "toolchain", 12, 1, NULL, elftoolchain_probes },
    { "lld", "toolchain", 12, 1, NULL,
      PROBES("ld.lld", "lld", "llvm-readelf", "llvm-objcopy", "llvm-strip", "llvm-ar",
             "llvm-ranlib", "llvm-nm", "llvm-objdump", "llvm-size") },
    { "toybox", "utils", 12, 1, NULL, PROBES("toybox") },
    { "limine", "bootloader", 8, 1, NULL,
      PROBES("limine", "limine-bios", "limine-uefi", "/boot/limine.cfg",
             "/boot/limine/limine.cfg", "/usr/share/limine", "/usr/local/share/limine") },
    { "mksh", "shell", 8, 1, NULL, PROBES("mksh", "oksh", "pdksh", "ksh") },
    { "bsd-loader", "bootloader", 6, 1, NULL, bsd_loader_probes },
    { "libedit", "library", 6, 1, NULL,
      PROBES("/usr/include/histedit.h", "/usr/local/include/histedit.h",
             "/usr/include/editline/readline.h", "/usr/lib/libedit.so.0",
             "/usr/local/lib/libedit.so") },
    { "libc++", "library", 6, 1, NULL,
      PROBES("/usr/include/c++/v1/iostream", "/usr/lib/libc++.so.1", "/usr/lib/libc++.so",
             "/usr/local/lib/libc++.so") },
    { "lldb", "debugger", 6, 1, NULL, PROBES("lldb") },
    { "llvm", "compiler", 6, 1, NULL, PROBES("llvm-config", "opt", "llc") },
    { "bmake", "build", 6, 1, NULL, bmake_probes },
    { "muon", "build", 6, 1, NULL, PROBES("muon") },
    { "neovim", "editor", 5, 1, NULL, PROBES("nvim", "neovim") },
    { "zsh", "shell", 5, 1, NULL, PROBES("zsh") },
    { "dash", "shell", 5, 1, NULL, PROBES("dash") },
    { "curl", "network", 5, 1, NULL, PROBES("curl") },
    { "vim", "editor", 4, 1, NULL, PROBES("vim", "vim.basic", "vim.tiny", "gvim") },
    { "samurai", "build", 4, 1, NULL, PROBES("samu", "samurai") },
    { "cmake", "build", 4, 1, NULL, PROBES("cmake", "ccmake", "cpack") },
    { "bsdtar", "archiving", 4, 1, NULL, bsdtar_probes },
    { "openssl", "library", 4, 1, NULL,
      PROBES("/usr/include/openssl/ssl.h", "/usr/local/include/openssl/ssl.h",
             "/usr/include/tls.h", "/usr/local/include/tls.h", "/usr/lib/libssl.so",
             "/usr/local/lib/libssl.so") },
    { "zig", "compiler", 3, 1, NULL, PROBES("zig") },
    { "tcc", "compiler", 3, 1, NULL, PROBES("tcc") },
    { "pcc", "compiler", 3, 1, NULL, PROBES("pcc") },
    { "byacc", "parser", 3, 1, NULL, byacc_probes },
    { "mandoc", "docs", 3, 1, NULL, PROBES("mandoc") },
    { "micro", "editor", 3, 1, NULL, PROBES("micro") },
    { "ripgrep", "utils", 3, 1, NULL, PROBES("rg", "ripgrep") },
    { "xz", "compression", 3, 1, NULL, PROBES("xz") },
    { "zstd", "compression", 3, 1, NULL, PROBES("zstd") },
    { "nawk", "scripting", 3, 1, NULL, PROBES("nawk") },
    { "just", "build", 3, 1, NULL, PROBES("just") },
    { "ugrep", "utils", 2, 1, NULL, PROBES("ugrep") },
    { "bzip2", "compression", 2, 1, NULL, PROBES("bzip2") },
    { "lz4", "compression", 2, 1, NULL, PROBES("lz4") },
    { "mg", "editor", 2, 1, NULL, PROBES("mg") },
    { "fd", "utils", 2, 1, NULL, PROBES("fd", "fdfind") },
    { "plocate", "utils", 2, 1, NULL, PROBES("plocate") },
    { "cproc", "compiler", 2, 1, NULL, PROBES("cproc") }
};

#define ENTRY_COUNT ((int)(sizeof entries / sizeof entries[0]))

struct scan {
    int found[ENTRY_COUNT];
    char detail[ENTRY_COUNT][MAX_DETAIL];
};

struct tally {
    int infection_points;
    int antibody_points;
    int infection_found;
    int antibody_found;
};

static int copy_detail(char *detail, size_t detail_sz, const char *text)
{
    size_t len = strlen(text);

    if (len >= detail_sz)
        return 0;

    memcpy(detail, text, len + 1);
    return 1;
}

static int is_foreign_symlink(const char *path)
{
    static const char *const foreign[] = { "busybox", "toybox", "llvm", "lld", "clang", NULL };
    char target[MAX_LINE];
    ssize_t len = readlink(path, target, sizeof target - 1);
    size_t i;

    if (len <= 0)
        return 0;

    target[len] = '\0';

    for (i = 0; foreign[i] != NULL; i++) {
        if (strstr(target, foreign[i]) != NULL)
            return 1;
    }

    return 0;
}

static int lookup_in_path(const char *name, char *detail, size_t detail_sz)
{
    const char *path = getenv("PATH");
    char dir[MAX_LINE];
    char full[MAX_LINE * 2];

    if (path == NULL || path[0] == '\0')
        path = "/usr/local/bin:/usr/bin:/bin";

    for (;;) {
        const char *colon = strchr(path, ':');
        size_t len = (colon != NULL) ? (size_t)(colon - path) : strlen(path);
        struct stat st;

        if (len == 0) {
            snprintf(dir, sizeof dir, "%s", ".");
        } else if (len < sizeof dir) {
            memcpy(dir, path, len);
            dir[len] = '\0';
        } else {
            goto advance;
        }

        if (snprintf(full, sizeof full, "%s/%s", dir, name) < (int)sizeof full &&
            stat(full, &st) == 0 && S_ISREG(st.st_mode) && !is_foreign_symlink(full) &&
            copy_detail(detail, detail_sz, full)) {
            return 1;
        }

    advance:
        if (colon == NULL)
            break;
        path = colon + 1;
    }

    return 0;
}

static int probe_hit(const char *probe, char *detail, size_t detail_sz)
{
    struct stat st;

    if (probe[0] != '/')
        return lookup_in_path(probe, detail, detail_sz);

    if (stat(probe, &st) != 0)
        return 0;

    return copy_detail(detail, detail_sz, probe);
}

static int entry_hit(const struct entry *entry, char *detail, size_t detail_sz)
{
    int i;

    for (i = 0; entry->probes[i] != NULL; i++) {
        if (probe_hit(entry->probes[i], detail, detail_sz))
            return 1;
    }

    detail[0] = '\0';
    return 0;
}

static void run_scan(struct scan *scan)
{
    int i;

    for (i = 0; i < ENTRY_COUNT; i++)
        scan->found[i] = entry_hit(&entries[i], scan->detail[i], MAX_DETAIL);
}

static void tally_up(const struct scan *scan, struct tally *tally)
{
    int i;

    tally->infection_points = 0;
    tally->antibody_points = 0;
    tally->infection_found = 0;
    tally->antibody_found = 0;

    for (i = 0; i < ENTRY_COUNT; i++) {
        if (!scan->found[i])
            continue;
        if (entries[i].antibody) {
            tally->antibody_points += entries[i].weight;
            tally->antibody_found++;
        } else {
            tally->infection_points += entries[i].weight;
            tally->infection_found++;
        }
    }
}

static int total_points(const struct tally *tally)
{
    return tally->infection_points + tally->antibody_points;
}

static int infection_share(const struct tally *tally)
{
    int total = total_points(tally);

    if (total == 0)
        return 0;

    return (tally->infection_points * 100 + total / 2) / total;
}

static int antibody_share(const struct tally *tally)
{
    int total = total_points(tally);

    if (total == 0)
        return 0;

    return 100 - infection_share(tally);
}

static const char *status_text(const struct tally *tally)
{
    int percent = infection_share(tally);

    if (total_points(tally) == 0)
        return "nothing found to measure";
    if (percent == 0)
        return "GNU/GPL-free sanctuary";
    if (percent <= 25)
        return "minor contamination";
    if (percent <= 50)
        return "moderate infestation";
    if (percent <= 75)
        return "severe infection";
    if (percent < 100)
        return "terminal GNU/GPL slop";
    return "fully GNU/GPL-pilled";
}

static void print_usage(FILE *out);
static void cmd_repl(void);

static void cmd_summary(void)
{
    struct scan scan;
    struct tally tally;

    run_scan(&scan);
    tally_up(&scan, &tally);

    printf("GNU/GPL:  %d%%\n", infection_share(&tally));
    printf("non-GNU:  %d%%\n", antibody_share(&tally));
    printf("\n  status: %s  (%d GNU/GPL, %d non-GNU)\n", status_text(&tally),
           tally.infection_found, tally.antibody_found);
    printf("\nInspect the infection: gnuger -l\n");
    printf("Find replacements:   gnuger -r\n");
}

static void print_found(const struct scan *scan, int antibody)
{
    int i;

    for (i = 0; i < ENTRY_COUNT; i++) {
        const struct entry *entry = &entries[i];

        if (entry->antibody != antibody || !scan->found[i])
            continue;

        printf("  %-12s %-11s %s\n", entry->name, entry->category, scan->detail[i]);
    }
}

static void print_list(const struct scan *scan)
{
    printf("GNU/GPL components:\n\n");
    print_found(scan, 0);
    printf("\nnon-GNU components:\n\n");
    print_found(scan, 1);
}

static void cmd_list(void)
{
    struct scan scan;

    run_scan(&scan);
    print_list(&scan);
}

static void print_replacements(const struct scan *scan)
{
    int i;

    for (i = 0; i < ENTRY_COUNT; i++) {
        const struct entry *entry = &entries[i];

        if (entry->antibody || !scan->found[i])
            continue;

        printf("%s%-12s %s--> %s%s\n", color_gray, entry->name, color_white,
               (entry->replace != NULL) ? entry->replace : "(no replacement)", color_reset);
    }
}

static void print_explain_infection(const struct entry *entry)
{
    printf("%s  %-12s %s--> %s%s [%s, %d points]\n", color_gray, entry->name, color_white,
           entry->replace != NULL ? entry->replace : "(no replacement)", color_reset,
           entry->category, entry->weight);
}

static void cmd_explain(void)
{
    int i;

    for (i = 0; i < ENTRY_COUNT; i++)
        if (!entries[i].antibody)
            print_explain_infection(&entries[i]);
}

static void cmd_help(void)
{
    print_usage(stdout);
}

static void cmd_version(void)
{
    printf("gnuger %s\n", GNUGER_VERSION);
}

struct command {
    const char *flag;
    const char *name;
    void (*run)(void);
    const char *usage;
};

static const struct command commands[] = {
    { "-l", "list",    cmd_list,    "list detected GNU/GPL and non-GNU components" },
    { "-e", "explain", cmd_explain, "show weights and replacements" },
    { "-r", "repl",    cmd_repl,    "show detected replacements" },
    { "-h", "help",    cmd_help,    "show this help" },
    { "-v", "version", cmd_version, "show version" }
};

static void cmd_repl(void)
{
    struct scan scan;

    run_scan(&scan);

    print_replacements(&scan);
}

static void print_usage(FILE *out)
{
    size_t i;

    fprintf(out, "usage: gnuger [command]\n\n");
    fprintf(out, "  (no command)  show infection summary\n");

    for (i = 0; i < sizeof commands / sizeof commands[0]; i++)
        fprintf(out, "  %s%s/%s%-8s %s\n", commands[i].flag, color_gray, color_reset,
                commands[i].name, commands[i].usage);
}

int main(int argc, char **argv)
{
    const char *arg = (argc > 1) ? argv[1] : NULL;
    size_t i;

    if (isatty(STDOUT_FILENO)) {
        color_gray = "\033[90m";
        color_white = "\033[97m";
        color_reset = "\033[0m";
    }

    if (arg == NULL) {
        cmd_summary();
        return 0;
    }

    for (i = 0; i < sizeof commands / sizeof commands[0]; i++) {
        if (strcmp(arg, commands[i].flag) != 0 && strcmp(arg, commands[i].name) != 0)
            continue;

        commands[i].run();
        return 0;
    }

    fprintf(stderr, "gnuger: unknown command '%s'\n", arg);
    print_usage(stderr);
    return 1;
}
