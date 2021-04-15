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
 * @(#)pd_resgen.c: RC resource generator for ProcessDirector
 * $Id: pd_resgen.c,v 1.11 2004/07/06 01:40:16 zhengl Exp $
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
    fprintf(stdout, "pd_resgen [-i index] [-s start] [-o output] [input1] [input2] ...\n");
    fprintf(stdout, "where:\n");
    fprintf(stdout, "\t-i\tset ID column for CSV files\n");
    fprintf(stdout, "\t\tindex\tID column index (0 to n)\n");
    fprintf(stdout, "\t-s\tset start column of CSV files for processing\n");
    fprintf(stdout, "\t\tstart\tstart coloumn, default is 0\n");
    fprintf(stdout, "\t-o\tset output RC file name\n");
    fprintf(stdout, "\t\toutput\toutput file, default means using stdout\n");
    fprintf(stdout, "\tinput[1-n]\n\t\tinput CSV files, default means using stdin\n");

    exit(0);
}

int main(int argc, char **argv)
{
    char c;

    pdt_handle_t resources = NULL;
    unsigned int start_index = 0;
    unsigned int id_index = 0;

    char *out_file = NULL;
    FILE *out_stream = NULL;
    FILE *in_fp = NULL;
    FILE *out_fp = NULL;

    /* parse args */
    optind = 0;
    while ((c = getopt(argc, argv, "i:s:o:h")) != -1)
    {
        switch (c)
        {
        case 'i':
            id_index = atoi(optarg);
            break;
        case 's':
            start_index = atoi(optarg);
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

    /* determin output stream */
    if (out_file)
    {
        out_fp = fopen(out_file, "w");
        if (!out_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open output file - %s.", out_file);
            goto failure;
        }
    }
    if (out_fp)
        out_stream = out_fp;
    else
        out_stream = stdout;

    /* read input csv file */
    resources = pdt_startup();

    if (argc > 0)
    {
        for (; argc > 0; argc--, argv++)
        {
            in_fp = fopen(argv[0], "r");
            if (!in_fp)
            {
                pdt_report_error(PDT_REPORT_ERROR, "Cannot open input file - %s.", *argv);
                goto failure;
            }
            pdt_load_csv_resources(resources, in_fp, id_index, start_index);
            fclose(in_fp);
            in_fp = NULL;
        }
    }
    else
        pdt_load_csv_resources(resources, stdin, id_index, start_index);
    
    pdt_save_rc_resources(resources, out_stream);

failure:
    if (in_fp) fclose(in_fp);
    if (out_fp) fclose(out_fp);

    pdt_cleanup(resources);
}
