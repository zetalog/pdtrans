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
 * @(#)pd_trans.h: ProcessDirector translation unitlity header
 * $Id: pd_trans.h,v 1.29 2005/01/26 06:32:12 gehb Exp $
 */

#ifndef __PD_TRANS_H_INCLUDE__
#define __PD_TRANS_H_INCLUDE__

#define LANGUAGE_UNKNOWN           -1
/** sometimes ASP means language undetermined */
#define LANGUAGE_ASP                1
/* Visual Basic or VB Script */
#define LANGUAGE_VB                 2
/* Java Script */
#define LANGUAGE_JS                 3
/* Normal HTML code */
#define LANGUAGE_HTML               4
/* SQL script */
#define LANGUAGE_SQL                5
/* C/C++ */
#define LANGUAGE_CPP                6
/* RC */
#define LANGUAGE_RC					7

/*
 * FIXME: MSVC do not support varibles in macro.
 *        following line should be:
 *
 * #define pdt_report_error(level, format, args...)   \
 */
#define pdt_report_error(level, format, args)                                   \
    do {                                                                        \
        if (level && strlen(level) > 0)                                         \
        {                                                                       \
            fprintf(stderr, "%s(%d) : " ## level ## ": " ## format ## "\r\n",   \
                    __FILE__, __LINE__, ## args);                               \
        }                                                                       \
    } while (0)

#define PDT_REPORT_ERROR        "error"
#define PDT_REPORT_WARNING      "warning"
#ifdef _DEBUG
#define PDT_REPORT_DEBUG        "debug"
#else
#define PDT_REPORT_DEBUG        ""
#endif

#define PDT_MODE_DRAW           1
#define PDT_MODE_REPLACE        2

typedef void *pdt_handle_t;
typedef struct _pdt_process_t *pdt_process_t;
typedef unsigned long pdt_param_t;

pdt_handle_t pdt_startup();
void pdt_cleanup(pdt_handle_t handle);

void pdt_draw_csv_resources(pdt_handle_t handle, char *language, char *keyword);
void pdt_save_csv_resources(pdt_handle_t handle, FILE *stream);
void pdt_load_csv_resources(pdt_handle_t handle, FILE *stream, int id, int start);
pdt_handle_t pdt_diff_csv_resources(pdt_handle_t handle1, pdt_handle_t handle2, char *language, int patch);

/**
 * generate .RC file.
 */
void pdt_save_rc_resources(pdt_handle_t handle, FILE *stream);

/**
 * generate .BAS constant header file.
 */
void pdt_save_vb_constants(pdt_handle_t handle, pdt_param_t vb_param);

/**
 * generate .h constant header file.
 */
void pdt_save_vc_constants(pdt_handle_t handle, pdt_param_t vc_param);

void pdt_enable_sufix_processor(char *sufix, pdt_handle_t handle, char *language, char *translation, pdt_param_t param);

pdt_process_t pdt_create_file_processor(char *input, char *output, int mode);
void pdt_destroy_file_processor(pdt_process_t parser);

void pdt_process_stream(FILE *in_stream, FILE *out_stream, pdt_process_t parser);
void pdt_process_file(char *input, char *output, int mode, int escape);
int pdt_process_folder(char *input, char *output, int mode, int escape, char *vb_trans);

/* creator parameter generators */
/* IMPORTANT: if processor plugin need creator parameter, it must export these two functions */
pdt_param_t pdt_create_asp_param(int language, char *message_include);
void pdt_destroy_asp_param(pdt_param_t param);

pdt_param_t pdt_create_vb_param(int cache_properties, char *const_header);
void pdt_destroy_vb_param(pdt_param_t param);

pdt_param_t pdt_create_vc_param(int cache_properties, char *const_header);
void pdt_destroy_vc_param(pdt_param_t param);

/* misc */
int pdt_is_dir(const char *path);

#endif /* __PD_TRANS_H_INCLUDE__ */
