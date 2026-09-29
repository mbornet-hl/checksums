/*
 *  Simultaneous computation of MD5, SHA256 and SHA512 checksums of a file
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  @(#) [Zen] ai_checksums.c   Version 1.50 du 26/09/29 - 
 *
 *  vim: ts=4 sw=4 et foldmethod=marker :
 *
 *  Needs GNUlib.
 *
 *  3 threads are created and are each attached to a CPU core.
 *  On 64 bits machines, MD5 run faster than SHA512, which run faster than SHA256.
 *  Computed checksums are stored in trusted extended attributes, if the users is
 *  allowed to access them.
 *
 * Remark: the prefix 'ai' you can see in the file names and in the source code has
 * no meaning, it's just after "ah' and before "aj". There's no artificial intelligence
 * there.
 */
#define _GNU_SOURCE  /* Needed for pthread_setaffinity_np and sched_getcpu */

/* Includes {{{ */
#include "../gnulib-build/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/capability.h>
#include <ftw.h>

#include "../gnulib/lib/md5.h"
#include "../gnulib/lib/sha256.h"
#include "../gnulib/lib/sha512.h"
#include "ai_cpri.h"
#include "ai_epri.h"

/* Includes }}} */

/* Version of the program */
#define AI_VERSION              ("1.50")

/* ai_max() {{{ */

/******************************************************************************

                AI_MAX

******************************************************************************/
static inline int ai_max(int n, int p)
{
    return (n > p) ? n : p;
}

/* ai_max() }}} */
/* ai_disp_xattr_sums() {{{ */

/******************************************************************************

                AI_DISP_XATTR_SUMS

******************************************************************************/
void ai_disp_xattr_sums(char *pathname, ai_xattr_sums *ref_xattr_sums)
{
    printf("[%-*s] date   = %s\n", G.path_width, pathname, ref_xattr_sums->chksum_ts_str);
    printf("[%-*s] MD5    = %s\n", G.path_width, pathname, ref_xattr_sums->MD5);
    printf("[%-*s] SHA256 = %s\n", G.path_width, pathname, ref_xattr_sums->SHA256);
    printf("[%-*s] SHA512 = %s\n", G.path_width, pathname, ref_xattr_sums->SHA512);
}

/* ai_disp_xattr_sums() }}} */
/* ai_disp_xattr_desc() {{{ */

/******************************************************************************

                AI_DISP_XATTR_DESC

******************************************************************************/
void ai_disp_xattr_desc(char *pathname, ai_xattr_desc *ref_xattr_desc)
{
    printf("[%-*s] Type = 0x%02X is_checksum = %d %-*s %-64s\n",
           G.path_width, pathname,
           ref_xattr_desc->chksum_type,
           ref_xattr_desc->is_checksum,
           G.tag_width,
           ref_xattr_desc->xattr_name,
           ref_xattr_desc->xattr_value);
}

/* ai_disp_xattr_desc() }}} */
/* usage() {{{ */
/******************************************************************************

                USAGE

******************************************************************************/
void usage()
{
    fprintf(stderr, "%s : version %s\n", G.progname, AI_VERSION);
    fprintf(stderr, "Usage : %s [-hvDNfM25rsdOCmnulL][-w width] pathname [pathname ...]\n", G.progname);
    fprintf(stderr, "  -h : help\n");
    fprintf(stderr, "  -v : verbose mode (for lists, and to display the origin of checksums)\n");
    fprintf(stderr, "  -D : display debug information\n");
    fprintf(stderr, "  -N : display pathname only\n");
    fprintf(stderr, "  -f : force checksums computation\n");
    fprintf(stderr, "  -M : display MD5 checksum\n");
    fprintf(stderr, "  -2 : display SHA256 checksum\n");
    fprintf(stderr, "  -5 : display SHA512 checksum\n");
    fprintf(stderr, "  -r : recursively traverses the directory tree\n");
    fprintf(stderr, "  -s : silent\n");
    fprintf(stderr, "  -d : display directory checksum (specific to this program)\n");
    fprintf(stderr, "  -O : list files that have all checksums\n");
    fprintf(stderr, "  -C : list files that needs checksum computation\n");
    fprintf(stderr, "  -n : list files that have no checksums in xattr\n");
    fprintf(stderr, "  -m : list files that have missing checksums in xattr\n");
    fprintf(stderr, "  -u : list files that have checksums in xattr but need updates\n");
    fprintf(stderr, "  -l : list xattr checksums status\n");
    fprintf(stderr, "  -L : legacy display format\n");
    fprintf(stderr, "  -w : pathname width\n");
}

/* usage() }}} */
/* ai_task_checksums() {{{ */

/******************************************************************************

                AI_TASK_CHECKSUMS

******************************************************************************/
void ai_task_checksums(const char *pathname, const struct stat *stat)
{
    char                 _indent[16];
    int                  _origin;
    ai_xattr_sums        _xattr_chksums;

    if ((stat->st_mode & S_IFMT) == S_IFREG) {
        /* Regular file : run checksums task */
        ai_get_checksums(pathname, &_xattr_chksums, &_origin);

        if (!G.list && !G.silent) {
            if (G.disp_name_only) {
                printf("%s\n", pathname);
            }
            else {
                if (!G.unique_target) {
                    if (!G.legacy_format) {
                        printf("%s :", pathname);
                        sprintf(_indent, "    ");
                        if (G.verbose) {
                            if (_origin == AI_ORIGIN_XATTR) {
                                printf(" (from extended attributes)");
                            }
                            else {
                                printf(" (computed)");
                            }
                        }
                        printf("\n");
                    }
                }
                else {
                    _indent[0]          = 0;
                }

                if (G.legacy_format) {
                    /* Legacy format : checksum filename */
                    if (G.disp_md5) {
                        printf("%-*s  %s\n", G.checksum_width, _xattr_chksums.MD5, pathname);
                    }
                    if (G.disp_sha256) {
                        printf("%-*s  %s\n", G.checksum_width, _xattr_chksums.SHA256, pathname);
                    }
                    if (G.disp_sha512) {
                        printf("%-*s  %s\n", G.checksum_width, _xattr_chksums.SHA512, pathname);
                    }
                }
                else {
                    if (G.disp_md5) {
                        printf("%sMD5    : %s\n", _indent, _xattr_chksums.MD5);
                    }
                    if (G.disp_sha256) {
                        printf("%sSHA256 : %s\n", _indent, _xattr_chksums.SHA256);
                    }
                    if (G.disp_sha512) {
                        printf("%sSHA512 : %s\n", _indent, _xattr_chksums.SHA512);
                    }
                    if (!G.unique_target) {
                        putchar('\n');
                    }
                }
            }
        }
    }
    else {
        /* Not a regulard file : no checksum to compute */
        if (!G.silent) {
            fprintf(stderr, "%s: \"%s\" is not a regular file nor a directory.\n",
                    G.progname, pathname);
        }
    }
}

/* ai_task_checksums() }}} */
/* ai_traverse() {{{ */

/******************************************************************************

                AI_TRAVERSE

******************************************************************************/
int ai_traverse(const char *pathname, const struct stat *stat, int typeflag,
                struct FTW *ftwbuf)
{
    if ((typeflag == FTW_D) && (ftwbuf->level > 0) && !G.recurse) {
        return FTW_SKIP_SUBTREE;
    }

    switch (typeflag) {

    case FTW_F:
        ai_task_checksums(pathname, stat);
        break;

    case FTW_D:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): FTW_D\n", G.path_width, pathname, __func__);
        }
        break;

    case FTW_DNR:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): FTW_DNR\n", G.path_width, pathname, __func__);
        }
        break;

    case FTW_DP:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): FTW_DR\n", G.path_width, pathname, __func__);
        }
        break;

    case FTW_NS:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): FTW_NS\n", G.path_width, pathname, __func__);
        }
        break;

    case FTW_SL:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): FTW_SL\n", G.path_width, pathname, __func__);
        }
        break;

    case FTW_SLN:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): FTW_SLN\n", G.path_width, pathname, __func__);
        }
        break;

    default:
        if (G.debug) {
            fprintf(stderr, "%-*s : [DEBUG] %s(): 0x%08X\n", G.path_width, pathname, __func__, typeflag);
        }
        break;
    }

    return 0;
}

/* ai_traverse() }}} */
/* main() {{{ */
/******************************************************************************

                MAIN

******************************************************************************/
int main(int argc, char *argv[])
{
    int                  _opt, _i;
    struct stat          _stat;

    G.progname          = argv[0];
    G.path_width        = AI_PATH_WIDTH;
    G.tag_width         = AI_TAG_WIDTH;
    G.disp_md5          = AI_DISP_UNSET,
    G.disp_sha256       = AI_DISP_UNSET,
    G.disp_sha512       = AI_DISP_UNSET;

    if (argc == 1) {
        usage();
        exit(AI_EXIT_USAGE);
    }

    /* Do no bufferize stdout to avoid sorting mess when redirecting outputs */
    if (setvbuf(stdout, (char *) 0, _IONBF, 0)) {
         fprintf(stderr, "%s: cannot unbufferize stdout !\n", G.progname);
    }

    while ((_opt = getopt(argc, argv, "hvDNfM25rsdOCmnulLw:")) != -1) {
        switch (_opt) {
        case 'h':
            usage();
            exit(AI_EXIT_USAGE);
            break;

        case 'v':
            G.verbose           = TRUE;
            break;

        case 'D':
            G.debug             = TRUE;
            break;

        case 'N':
            G.disp_name_only    = TRUE;
            break;

        case 'f':
            G.force_checksums   = TRUE;
            break;

        case 'M':
            G.disp_md5          = AI_DISP_YES;
            G.checksum_width    = ai_max(G.checksum_width, AI_MD5_WIDTH);
            break;

        case '2':
            G.disp_sha256       = AI_DISP_YES;
            G.checksum_width    = ai_max(G.checksum_width, AI_SHA256_WIDTH);
            break;

        case '5':
            G.disp_sha512       = AI_DISP_YES;
            G.checksum_width    = ai_max(G.checksum_width, AI_SHA512_WIDTH);
            break;

        case 'O':
            G.list_OK           = TRUE;
            break;

        case 'C':
            G.list_needs_sums   = TRUE;
            break;

        case 'm':
            G.list_missing      = TRUE;
            break;

        case 'n':
            G.list_none         = TRUE;
            break;

        case 'u':
            G.list_needs_update = TRUE;
            break;

        case 'l':
            G.list_status       = TRUE;
            break;

        case 'L':
            G.legacy_format     = TRUE;
            break;

        case 'r':
            G.recurse           = TRUE;
            break;

        case 's':
            G.silent            = TRUE;
            break;

        case 'd':
            G.dir_checksum      = TRUE;
            fprintf(stderr, "%s: option -d not implemented yet.\n", G.progname);
            exit(AI_EXIT_USAGE);
            break;

        case 'w':
            G.path_width        = atoi(optarg);
            break;

        default:
            fprintf(stderr, "%s: unknown option '-%c' !\n", G.progname, _opt);
            usage();
            exit(AI_EXIT_USAGE);
            break;
            
        }
    }

    if ((G.disp_md5 == AI_DISP_YES) && (G.silent == TRUE)) {
        fprintf(stderr, "%s: options -M and -s are incompatible !\n", G.progname);
        exit(AI_EXIT_USAGE);
    }

    if ((G.disp_sha256 == AI_DISP_YES) && (G.silent == TRUE)) {
        fprintf(stderr, "%s: options -2 and -s are incompatible !\n", G.progname);
        exit(AI_EXIT_USAGE);
    }

    if ((G.disp_sha512 == AI_DISP_YES) && (G.silent == TRUE)) {
        fprintf(stderr, "%s: options -5 and -s are incompatible !\n", G.progname);
        exit(AI_EXIT_USAGE);
    }
    if ((G.list_OK + G.list_none + G.list_missing + G.list_needs_update
       + G.list_needs_sums + G.list_status) > 1) {
        fprintf(stderr, "%s: only one option in [OCmnu] is allowed !\n", G.progname);
        exit(AI_EXIT_USAGE);
    }

    if ((G.list_OK || G.list_none || G.list_missing || G.list_needs_update
         || G.list_needs_sums || G.dir_checksum || G.list_status)
    &&  !ai_has_cap_sys_admin()) {
        fprintf(stderr, "%s: needs CAP_SYS_ADMIN capability to use an option from [dOCmnu] !\n",
                G.progname);
    }

    G.list       = G.list_OK || G.list_none || G.list_missing || G.list_needs_update
                  || G.list_needs_sums || G.list_status;

    if (G.force_checksums && G.list) {
        fprintf(stderr, "%s: options -f and -[OCmnu] are incompatible !\n",
                G.progname);
        exit(AI_EXIT_USAGE);
    }

    /* If "silent" is not explicitelty selected, and no checksum type is selected,
     * then all checksum types are selected by default */
    if ((G.silent != TRUE)
    &&  (G.disp_md5 == AI_DISP_UNSET)
    &&  (G.disp_sha256 == AI_DISP_UNSET)
    &&  (G.disp_sha512 == AI_DISP_UNSET)) {
        G.disp_md5          = AI_DISP_YES;
        G.disp_sha256       = AI_DISP_YES;
        G.disp_sha512       = AI_DISP_YES;
        G.checksum_width    = AI_SHA512_WIDTH;
    }

    G.unique_target     = FALSE;
    if (optind == (argc -1)) {
        G.unique_target     = TRUE;
    }

    /* Treat all arguments */
    for (_i = optind; _i < argc; _i++) {
        if (stat(argv[_i], &_stat) == -1) {
            fprintf(stderr, "%s: cannot stat \"%s\" !\n", G.progname, argv[_i]);
            exit(AI_EXIT_ERR_STAT);
        }
        if ((_stat.st_mode & S_IFMT) == S_IFDIR) {
            G.unique_target     = FALSE;
            if (nftw(argv[_i], ai_traverse, 32, FTW_PHYS|FTW_ACTIONRETVAL) == -1) {
                perror("nftw");
                exit(AI_EXIT_ERR_STAT);
            }
        }
        else if ((_stat.st_mode & S_IFMT) == S_IFREG) {
            /* Regular file : run checksums task */
            ai_task_checksums(argv[_i], &_stat);
        }
        else {
            fprintf(stderr, "%s: \"%s\" is not a regular file nor a directory.\n",
                    G.progname, argv[_i]);
        }
    }

    if (G.list) {
        fprintf(stderr, "*** %s: list only => no xattr have been updated.\n", G.progname);
    }

    return EXIT_SUCCESS;
}

/* main() }}} */
