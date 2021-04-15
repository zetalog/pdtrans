/*
 * Copyright (c) 2004
 *    Author: Lv "Zetalog" Zheng
 *    Internet: zhengl@sh.necas.nec.com.cn
 *
 * Redistribution and use in source and binary forms in company, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the NEC-CAS Shanghai
 *    Development Center.
 * 3. Neither the name of the companay nor the names of its developer may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 4. Any out company redistribution are not permitted unless companay
 *    copyright no longer declaimed, but must not remove developer(s) above.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NEC-CAS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE NEC-CAS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)pd_diff.c: difference of two results for ProcessDirector
 * $Id: pd_diff.c,v 1.7 2004/07/01 01:05:32 zhengl Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <pd_trans.h>

void usage()
{
    fprintf(stdout, "usage:\n");
    fprintf(stdout, "pd_diff [-p] [-l language] [-o output] older [newer] \n");
    fprintf(stdout, "where:\n");
    fprintf(stdout, "\t-p\tpatch tow translations\n");
    fprintf(stdout, "\t-l\tset compare language name\n");
    fprintf(stdout, "\t\tlocale\tcompare language, default is \"japanese\"\n");
    fprintf(stdout, "\t-o\tset output file name\n");
    fprintf(stdout, "\t\toutput\toutput file, default means using stdout\n");
    fprintf(stdout, "\tolder\told version to be compared\n");
    fprintf(stdout, "\tnewer\tnew version to be compared, default means using stdin\n");

    exit(0);
}

int main(int argc, char **argv)
{
    char c;

    pdt_handle_t from_resources = NULL;
    pdt_handle_t to_resources = NULL;

    pdt_handle_t differences = NULL;

    char *older = NULL;
    char *newer = NULL;
    char *out_file = NULL;
    FILE *out_stream = stdout;
    FILE *out_fp = NULL;

    FILE *in_fp1 = NULL;
    FILE *in_fp2 = NULL;
    FILE *in_stream = stdin;

    int patch = 0;
    char *language = "japanese";

    /* parse args */
    optind = 0;
    while ((c = getopt(argc, argv, "pl:o:h")) != -1)
    {
        switch (c)
        {
        case 'p':
            patch = 1;
            break;
        case 'l':
            language = optarg;
            break;
        case 'o':
            out_file = optarg;
            break;
        case 'h':
            usage();
            break;
        case '?':
            usage();
        default:
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1)
        usage(1);

    older = argv[0];

    if (argc > 1)
    {
        newer = argv[1];
        in_fp2 = fopen(newer, "r");
        if (!in_fp2)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open new version - %s.", newer);
            goto failure;
        }
        in_stream = in_fp2;
    }

    /* determin output stream */
    if (out_file)
    {
        out_fp = fopen(out_file, "w");
        if (!out_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open output file - %s.", out_file);
            goto failure;
        }
        out_stream = out_fp;
    }

    in_fp1 = fopen(older, "r");
    if (!in_fp1)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Cannot open old version - %s.", older);
        goto failure;
    }

    from_resources = pdt_startup();
    to_resources = pdt_startup();

    pdt_load_csv_resources(from_resources, in_fp1, 0, 0);
    pdt_load_csv_resources(to_resources, in_stream, 0, 0);

    differences = pdt_diff_csv_resources(from_resources, to_resources, language, patch);
    pdt_save_csv_resources(differences, out_stream);

failure:
    if (out_fp) fclose(out_fp);
    if (in_fp1) fclose(in_fp1);
    if (in_fp2) fclose(in_fp2);

    pdt_cleanup(from_resources);
    pdt_cleanup(to_resources);
    pdt_cleanup(differences);
}
