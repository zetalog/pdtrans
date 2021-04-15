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
 * @(#)pd_trans.c: resource translator for ProcessDirector sources
 * $Id: pd_trans.c,v 1.12 2005/01/26 07:53:55 gehb Exp $
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

#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#include <pd_trans.h>

void usage()
{
    fprintf(stdout, "usage:\n");
    fprintf(stdout, "pd_trans [-l source] [-t target] [-m mode] [-k keyword] [-i include] ");
    fprintf(stdout, "[-c const] [-a aspres] [-b vbres] [-p cppres] [-r rcres] [-s sqlres] [-g] [-o output] input\n");
    fprintf(stdout, "where:\n");
    fprintf(stdout, "\t-l\tset source language name for comparation\n");
    fprintf(stdout, "\t\tlocale\tsource language, default is \"japanese\"\n");
    fprintf(stdout, "\t-t\tset target language name for translation\n");
    fprintf(stdout, "\t\ttarget\ttarget language, default is \"english\"\n");
    fprintf(stdout, "\t-m\tset processing mode\n");
    fprintf(stdout, "\t\tmode\tprocessing mode, can be following:\n");
    fprintf(stdout, "\t\tdraw\tdraw mode for resource generation, default mode\n");
    fprintf(stdout, "\t\treplace\treplace mode for resource replacement\n");
    fprintf(stdout, "\t-k\tset keyword to filter output CSV\n");
    fprintf(stdout, "\t\tkeyword\tkeyword filter, valid for \"draw\" mode\n");
    fprintf(stdout, "\t-i\tset ASP include file to append\n");
    fprintf(stdout, "\t\tinclude\tinclude file, valid for \"ASP replace\" mode\n");
    fprintf(stdout, "\t-c\tgenerate constant VB header file\n");
    fprintf(stdout, "\t\tconst\tconstant header for VB, valid for \"VB replace\" mode\n");
    fprintf(stdout, "\t-a\tCSV resource file name for ASP\n");
    fprintf(stdout, "\t\taspres\tASP resource file\n");
    fprintf(stdout, "\t-b\tCSV resource file name for VB\n");
    fprintf(stdout, "\t\tvbres\tVB resource file\n");
    fprintf(stdout, "\t-p\tCSV resource file name for CPP\n");
    fprintf(stdout, "\t\tvbres\tCPP resource file\n");
    fprintf(stdout, "\t-r\tCSV resource file name for RC\n");
    fprintf(stdout, "\t\tvbres\tRC resource file\n");
    fprintf(stdout, "\t-s\tCSV resource file name for SQL\n");
    fprintf(stdout, "\t\tsqlres\tSQL resource file\n");
    fprintf(stdout, "\t-g\tgenerate _target for .frm.ctl, valid for \"VB replace\" mode\n");
    fprintf(stdout, "\t-o\toutput for \"replace\" mode, do not allow overwriting\n");
    fprintf(stdout, "\t\toutput\treplacement result, valid for \"replace\" mode\n");
    fprintf(stdout, "\tinput\tinput files or folders\n");

    exit(0);
}

int main(int argc, char **argv)
{
    char c;

	char *constant = NULL;
    char *keyword = NULL;

    char *source = "japanese";
    char *target = "english";

    int mode = PDT_MODE_DRAW;

    char *res_file = NULL;
    FILE *res_fp = NULL;

    int gen_vb_trans = 0;

    pdt_process_t parser = NULL;

    char *asp_res = NULL;
    FILE *asp_fp = NULL;
    pdt_handle_t asp_resource = NULL;
    char *asp_include = NULL;

    char *vb_res = NULL;
    FILE *vb_fp = NULL;
    pdt_handle_t vb_resource = NULL;

    char *sql_res = NULL;
    FILE *sql_fp = NULL;
    pdt_handle_t sql_resource = NULL;

    char *vc_res = NULL;
    FILE *vc_fp = NULL;
    pdt_handle_t vc_resource = NULL;

    char *rc_res = NULL;
    FILE *rc_fp = NULL;
    pdt_handle_t rc_resource = NULL;

    char *out_file = NULL;

    pdt_param_t asp_param = 0;
    pdt_param_t vb_param = 0;
	pdt_param_t vc_param = 0;
    pdt_param_t unknown_param = 0;

    /* parse args */
    optind = 0;
    while ((c = getopt(argc, argv, "m:l:t:k:c:a:i:b:s:o:p:r:gh")) != -1)
    {
        switch (c)
        {
        case 'g':
            gen_vb_trans = 1;
            break;
        case 'm':
            if (stricmp(optarg, "replace") == 0)
                mode = PDT_MODE_REPLACE;
            else if (stricmp(optarg, "draw") == 0)
                mode = PDT_MODE_DRAW;
            break;
        case 'l':
            source = optarg;
            break;
        case 't':
            target = optarg;
            break;
        case 'a':
            asp_res = optarg;
            break;
        case 'b':
            vb_res = optarg;
            break;
        case 'p':
            vc_res = optarg;
            break;
        case 'r':
            rc_res = optarg;
            break;
        case 's':
            sql_res = optarg;
            break;
        case 'k':
            keyword = optarg;
            break;
        case 'i':
            asp_include = optarg;
            break;
        case 'c':
            constant = optarg;
            break;
        case 'o':
            out_file = optarg;
            break;
        case 'h':
        case '?':
            usage();
        default:
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if (!(asp_res || vb_res || sql_res || vc_res || rc_res))
    {
        pdt_report_error(PDT_REPORT_ERROR, "Can not ommit resource file.", 0);
        usage();
    }

    if (mode == PDT_MODE_REPLACE && !out_file)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Must specify output for replace mode.", 0);
        usage();
        goto failure;
    }

    /* sanity check */
    if (argc > 0 && mode == PDT_MODE_REPLACE)
    {
        if (pdt_is_dir(argv[0]))
        {
            if (access(out_file, 00) && !pdt_is_dir(out_file))
            {
                pdt_report_error(PDT_REPORT_ERROR, "Output %s must be directory.", out_file);
                goto failure;
            }
            mkdir(out_file);
        }
    }

    /* loading resource */
    if (asp_res)
    {
        asp_resource = pdt_startup();

        if (mode == PDT_MODE_REPLACE)
        {
            asp_fp = fopen(asp_res, "r");
            pdt_load_csv_resources(asp_resource, asp_fp, 0, 0);
        }
        else if (mode == PDT_MODE_DRAW)
        {
            asp_fp = fopen(asp_res, "w");
            pdt_draw_csv_resources(asp_resource, source, keyword);
        }

        if (!asp_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open resource file - %s.", asp_res);
            goto failure;
        }

        asp_param = pdt_create_asp_param(LANGUAGE_VB, asp_include);
        unknown_param = pdt_create_asp_param(LANGUAGE_UNKNOWN, asp_include);

        pdt_enable_sufix_processor(".asp", asp_resource, source, target, asp_param);
        pdt_enable_sufix_processor(".inc", asp_resource, source, target, unknown_param);
        pdt_enable_sufix_processor(".js", asp_resource, source, target, 0);
        pdt_enable_sufix_processor(".vbs", asp_resource, source, target, 0);
    }

    if (vb_res)
    {
        vb_resource = pdt_startup();

        if (mode == PDT_MODE_REPLACE)
        {
            vb_fp = fopen(vb_res, "r");
            pdt_load_csv_resources(vb_resource, vb_fp, 0, 0);
        }
        else if (mode == PDT_MODE_DRAW)
        {
            vb_fp = fopen(vb_res, "w");
            pdt_draw_csv_resources(vb_resource, source, keyword);
        }

        if (!vb_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open resource file - %s.", vb_res);
            goto failure;
        }

        vb_param = pdt_create_vb_param(1, constant);

        pdt_enable_sufix_processor(".frm", vb_resource, source, target, vb_param);
        pdt_enable_sufix_processor(".ctl", vb_resource, source, target, vb_param);
        pdt_enable_sufix_processor(".cls", vb_resource, source, target, vb_param);
        pdt_enable_sufix_processor(".bas", vb_resource, source, target, vb_param);
    }

    if (vc_res)
    {
        vc_resource = pdt_startup();

        if (mode == PDT_MODE_REPLACE)
        {
            vc_fp = fopen(vc_res, "r");
            pdt_load_csv_resources(vc_resource, vc_fp, 0, 0);
        }
        else if (mode == PDT_MODE_DRAW)
        {
            vc_fp = fopen(vc_res, "w");
            pdt_draw_csv_resources(vc_resource, source, keyword);
        }

        if (!vc_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open resource file - %s.", vc_res);
            goto failure;
        }

        vc_param = pdt_create_vc_param(1, constant);

        pdt_enable_sufix_processor(".cpp", vc_resource, source, target, vc_param);
		pdt_enable_sufix_processor(".c", vc_resource, source, target, vc_param);
        pdt_enable_sufix_processor(".hpp", vc_resource, source, target, vc_param);
		pdt_enable_sufix_processor(".h", vc_resource, source, target, vc_param);

    }

    if (rc_res)
    {
        rc_resource = pdt_startup();

        if (mode == PDT_MODE_REPLACE)
        {
            rc_fp = fopen(rc_res, "r");
            pdt_load_csv_resources(rc_resource, rc_fp, 0, 0);
        }
        else if (mode == PDT_MODE_DRAW)
        {
            rc_fp = fopen(rc_res, "w");
            pdt_draw_csv_resources(rc_resource, source, keyword);
        }

        if (!rc_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open resource file - %s.", rc_res);
            goto failure;
        }

        //rc_param = pdt_create_rc_param(1, constant);

        pdt_enable_sufix_processor(".rc", rc_resource, source, target, unknown_param);
		pdt_enable_sufix_processor(".rc2", rc_resource, source, target, unknown_param);
    }

    if (sql_res)
    {
        sql_resource = pdt_startup();

        if (mode == PDT_MODE_REPLACE)
        {
            sql_fp = fopen(sql_res, "r");
            pdt_load_csv_resources(sql_resource, sql_fp, 0, 0);
        }
        else if (mode == PDT_MODE_DRAW)
        {
            sql_fp = fopen(sql_res, "w");
            pdt_draw_csv_resources(sql_resource, source, keyword);
        }

        if (!sql_fp)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open resource file - %s.", sql_res);
            goto failure;
        }

       pdt_enable_sufix_processor(".sql", sql_resource, source, target, 0);
    }

    if (argc > 0)
    {
        if (pdt_is_dir(argv[0]))
            pdt_process_folder(argv[0], out_file, mode, 0, gen_vb_trans?target:NULL);
        else
        {
            pdt_process_file(argv[0], out_file, mode, 0);
        }
    }

    if (mode == PDT_MODE_REPLACE && constant)
	{
		if (vb_fp && vb_resource)
			pdt_save_vb_constants(vb_resource, vb_param);
		else if (vc_fp && vc_resource)
			pdt_save_vc_constants(vc_resource, vc_param);
	}

    if (mode == PDT_MODE_DRAW)
    {
        if (asp_fp && asp_resource)
            pdt_save_csv_resources(asp_resource, asp_fp);
        if (sql_fp && sql_resource)
            pdt_save_csv_resources(sql_resource, sql_fp);
        if (vb_fp && vb_resource)
            pdt_save_csv_resources(vb_resource, vb_fp);
        if (vc_fp && vc_resource)
            pdt_save_csv_resources(vc_resource, vc_fp);
        if (rc_fp && rc_resource)
            pdt_save_csv_resources(rc_resource, rc_fp);

    }
    
failure:
    if (asp_fp) fclose(asp_fp);
    if (vb_fp) fclose(vb_fp);
	if (vc_fp) fclose(vc_fp);
	if (rc_fp) fclose(rc_fp);
    if (sql_fp) fclose(sql_fp);
    if (parser) pdt_destroy_file_processor(parser);
    pdt_cleanup(asp_resource);
    pdt_cleanup(vb_resource);
	pdt_cleanup(vc_resource);
	pdt_cleanup(rc_resource);
    pdt_cleanup(sql_resource);
    pdt_destroy_asp_param(asp_param);
    pdt_destroy_vb_param(vb_param);
	pdt_destroy_vb_param(vc_param);
    pdt_destroy_asp_param(unknown_param);
}
