/*
 * ZETALOG's Personal COPYRIGHT
 *
 * Copyright (c) 2003
 *    ZETALOG - "Lv ZHENG".  All rights reserved.
 *    Author: Lv "Zetalog" Zheng
 *    Internet: zetalog@hzcnc.com
 *
 * This COPYRIGHT used to protect Personal Intelligence Rights.
 * Redistribution and use in source and binary forms with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the Lv "Zetalog" ZHENG.
 * 3. Neither the name of this software nor the names of its developers may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 4. Permission of redistribution and/or reuse of souce code partially only
 *    granted to the developer(s) in the companies ZETALOG worked.
 * 5. Any modification of this software should be published to ZETALOG unless
 *    the above copyright notice is no longer declaimed.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ZETALOG AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE ZETALOG OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)pd_test.c: unit test program for ProcessDirector translation tools
 * $Id: pd_test.c,v 1.18 2004/07/13 06:37:39 fengxg Exp $
 */

/* the only header should be included */
#include <cunit.h>
#include <pd_intern.h>

#define PDT_VB_RESOURCE     "..\\data\\csv\\vbmsg_test.csv"
#define PDT_ASP_RESOURCE    "..\\data\\csv\\aspmsg_test.csv"

#define PDT_VB_INPUT1       "..\\data\\vb\\input1.frm"
#define PDT_ASP_INPUT1      "..\\data\\asp\\input1.asp"
#define PDT_ASP_INPUT2      "..\\data\\asp\\input2.asp"
#define PDT_ASP_INPUT3      "..\\data\\asp\\input3.asp"
#define PDT_INC_INPUT1      "..\\data\\inc\\input1.inc"
#define PDT_INC_INPUT2      "..\\data\\inc\\input2.inc"
#define PDT_INC_INPUT3      "..\\data\\asp\\input1.inc"
#define PDT_INC_INPUT4      "..\\data\\asp\\input2.inc"

#define PDT_VB_EXPECTED1    "..\\data\\vb\\output1.frm"
#define PDT_ASP_EXPECTED1   "..\\data\\asp\\output1.asp"
#define PDT_ASP_EXPECTED2   "..\\data\\asp\\output2.asp"
#define PDT_ASP_EXPECTED3   "..\\data\\asp\\output3.asp"
#define PDT_INC_EXPECTED1   "..\\data\\inc\\output1.inc"
#define PDT_INC_EXPECTED2   "..\\data\\inc\\output2.inc"

#define PDT_VB_ACTUAL1      "..\\obj\\test\\vb\\output1.frm"
#define PDT_ASP_ACTUAL1     "..\\obj\\test\\asp\\output1.asp"
#define PDT_ASP_ACTUAL2     "..\\obj\\test\\asp\\output2.asp"
#define PDT_ASP_ACTUAL3     "..\\obj\\test\\asp\\output3.asp"
#define PDT_INC_ACTUAL1     "..\\obj\\test\\inc\\output1.inc"
#define PDT_INC_ACTUAL2     "..\\obj\\test\\asp\\input1.inc"
#define PDT_INC_ACTUAL3     "..\\obj\\test\\asp\\input2.inc"

void cunit_equals_string(CUNIT_SOURCE_LINE_DECL,
                         IN const char *expected, IN const char *actual)
{
    int result = strcmp(expected, actual);
    if (result)
        CUNIT_RAISE_EXCEPTION("Value(string) does not equals."/*, actual, excepted*/);
}

void cunit_equals_contents(CUNIT_SOURCE_LINE_DECL,
                           IN const char *src_file, IN const char *dst_file)
{
#define ERROR_SRC_OPEN          1
#define ERROR_DST_OPEN          2
#define ERROR_SRC_READ          3
#define ERROR_DST_READ          4
#define ERROR_IDENTICAL         5

    int result = 0;
    int count_src;
    int count_dst;

    char buffer_src[BUFFER_SIZE];
    char buffer_dst[BUFFER_SIZE];

    FILE * src_fp = NULL;
    FILE * dst_fp = NULL;

    if ((src_fp = fopen(src_file, "r")) == NULL)
    {
        result = ERROR_SRC_OPEN;
        goto failure;
    }
    if ((dst_fp = fopen(dst_file, "r")) == NULL)
    {
        result = ERROR_DST_OPEN;
        goto failure;
    }

    while (!feof(src_fp))
    {
        /* Attempt to read in BUFFER_SIZE bytes: */
        count_src = fread( buffer_src, sizeof(char), BUFFER_SIZE, src_fp );
        if (count_src < 0 || ferror(src_fp))
        {
            result = -1;
            break;
        } 
        count_dst = fread(buffer_dst, sizeof (char), BUFFER_SIZE, dst_fp );
        if (count_dst < 0 || ferror(dst_fp))
        {
            result = ERROR_DST_OPEN;
            break;
        } 
        if ((count_src != count_dst) ||
            (memcmp(buffer_src, buffer_dst, count_src)))
        {
            result = ERROR_IDENTICAL;
            fprintf(stderr, "\nFile not identical:");
            fprintf(stderr, "\nsource:\n");
            fprintf(stderr, "\n------------------------------\n");
            fwrite(buffer_src, sizeof(char), count_src, stderr);
            fprintf(stderr, "\n------------------------------");
            fprintf(stderr, "\ndestination:\n");
            fprintf(stderr, "------------------------------\n");
            fwrite(buffer_dst, sizeof(char), count_dst, stderr);
            fprintf(stderr, "\n------------------------------\n");
            break;
        }
    }

failure:
    if (src_fp) fclose(src_fp);
    if (dst_fp) fclose(dst_fp);

    switch (result)
    {
    case ERROR_SRC_OPEN:
        CUNIT_RAISE_EXCEPTION("Error opening source file.");
    case ERROR_DST_OPEN:
        CUNIT_RAISE_EXCEPTION("Error opening destination file.");
    case ERROR_SRC_READ:
        CUNIT_RAISE_EXCEPTION("Error reading source file.");
    case ERROR_DST_READ:
        CUNIT_RAISE_EXCEPTION("Error reading destination file.");
    case ERROR_IDENTICAL:
        CUNIT_RAISE_EXCEPTION("Files not identical.");
    }
}


/*=========================================================================
 * CASE:          pd_test_vb_draw_1
 * TYPE:          unit test case
 * OVERVIEW:      test for VB draw case 1(.frm).
 *=======================================================================*/
void CUNITCBK pd_test_vb_draw_1(void) {
    vector_t *vb_resource = pdt_startup();
    pdt_resource_t *resource;
    char *string1;

    CUNIT_ASSERT_NOTNULL(vb_resource);
    pdt_enable_sufix_processor(".frm", vb_resource, "japanese", "english", 0);
    pdt_draw_csv_resources(vb_resource, "japanese", NULL);
    pdt_process_file(PDT_VB_INPUT1, NULL, PDT_MODE_DRAW, 0);
    CUNIT_ASSERT_EQUALS(int, 1, element_count(vb_resource));
    resource = get_element(vb_resource, 0);
    CUNIT_ASSERT_NOTNULL(resource);
    CUNIT_ASSERT_EQUALS(string, resource->language, "japanese");
    CUNIT_ASSERT_NULL(resource->handler);
    CUNIT_ASSERT_EQUALS(int, 18, element_count(resource->strings));

    string1 = get_element(resource->strings, 0);
    CUNIT_ASSERT_EQUALS(string, string1, "\"1プロパティ前\"");
    string1 = get_element(resource->strings, 1);
    CUNIT_ASSERT_EQUALS(string, string1, "\"2プロパティ中\"");

    string1 = get_element(resource->strings, 2);
    CUNIT_ASSERT_EQUALS(string, string1, "\"3込みプロパティ中\"");
    string1 = get_element(resource->strings, 3);
    CUNIT_ASSERT_EQUALS(string, string1, "\"4込みプロパティ後\"");

    string1 = get_element(resource->strings, 4);
    CUNIT_ASSERT_EQUALS(string, string1, "\"5プロパティ後\"");
    string1 = get_element(resource->strings, 5);
    CUNIT_ASSERT_EQUALS(string, string1, "\"6込み中\"");

    string1 = get_element(resource->strings, 6);
    CUNIT_ASSERT_EQUALS(string, string1, "\"7中\"");

    string1 = get_element(resource->strings, 7);
    CUNIT_ASSERT_EQUALS(string, string1, "\"8単コンメト前\"");
    string1 = get_element(resource->strings, 8);
    CUNIT_ASSERT_EQUALS(string, string1, "\"9フォーム単コメント後\"");
    string1 = get_element(resource->strings, 9);

    CUNIT_ASSERT_EQUALS(string, string1, "\"18小括号タイトル0\"");
    string1 = get_element(resource->strings, 10);
    CUNIT_ASSERT_EQUALS(string, string1, "\"19小括号タイトル1\"");
    string1 = get_element(resource->strings, 11);
    
    CUNIT_ASSERT_EQUALS(string, string1, "\"10二Eng中\"");
    string1 = get_element(resource->strings, 12);
    CUNIT_ASSERT_EQUALS(string, string1, "\"11外部\"");
    string1 = get_element(resource->strings, 13);

    CUNIT_ASSERT_EQUALS(string, string1, "\"12内部\"");
    string1 = get_element(resource->strings, 14);
    CUNIT_ASSERT_EQUALS(string, string1, "\"14コード単コメント後\"");
    string1 = get_element(resource->strings, 15);
    CUNIT_ASSERT_EQUALS(string, string1, "\"15多列コメント後\"");

    string1 = get_element(resource->strings, 16);
    CUNIT_ASSERT_EQUALS(string, string1, "\"16\"\"Escape序列\"");

    string1 = get_element(resource->strings, 17);
    CUNIT_ASSERT_EQUALS(string, string1, "\"17定数\"");

    pdt_cleanup(vb_resource);
}


/*=========================================================================
 * CASE:          pd_test_asp_replace_1
 * TYPE:          unit test case
 * OVERVIEW:      test for ASP replace case 1 of PDE-UTD-DrawReplace.doc
                  auto add INCLUDE after first LANGUAGE Tag
 *=======================================================================*/
void CUNITCBK pd_test_asp_replace_1(void)
{
    vector_t *asp_resource = pdt_startup();
    FILE * asp_csv_fp = NULL;
    pdt_param_t asp_param = 0;

    CUNIT_ASSERT_NOTNULL(asp_resource);

    asp_param = pdt_create_asp_param(LANGUAGE_VB, PDT_INC_INPUT1);

    pdt_enable_sufix_processor(".asp", asp_resource, "japanese", "english", asp_param);

    asp_csv_fp = fopen(PDT_ASP_RESOURCE, "r");
    CUNIT_ASSERT_NOTNULL(asp_csv_fp);
    pdt_load_csv_resources(asp_resource, asp_csv_fp, 0, 0);
    CUNIT_ASSERT_NOTNULL(asp_resource);
    fclose(asp_csv_fp);
    
    CUNIT_ASSERT_EQUALS(int, 1, element_count(asp_resource));

    pdt_process_file(PDT_ASP_INPUT1, PDT_ASP_ACTUAL1, PDT_MODE_REPLACE, 0);

    CUNIT_ASSERT_EQUALS(contents , PDT_ASP_EXPECTED1, PDT_ASP_ACTUAL1);

    pdt_cleanup(asp_resource);
    pdt_destroy_asp_param(asp_param);
}


/*=========================================================================
 * CASE:          pd_test_asp_replace_2
 * TYPE:          unit test case
 * OVERVIEW:      test for ASP replace case 2, 4, 6, 7, 8, 9, 10 of
 *                PDE-UTD-DrawReplace.doc.
 *=======================================================================*/
void CUNITCBK pd_test_asp_replace_2(void)
{
    vector_t *asp_resource = pdt_startup();
    FILE * asp_csv_fp = NULL;
    pdt_param_t asp_param = 0;

    CUNIT_ASSERT_NOTNULL(asp_resource);

    asp_param = pdt_create_asp_param(LANGUAGE_VB, NULL);

    pdt_enable_sufix_processor(".asp", asp_resource, "japanese", "english", asp_param);
    pdt_enable_sufix_processor(".inc", asp_resource, "japanese", "english", 0);

    asp_csv_fp = fopen(PDT_ASP_RESOURCE, "r");
    CUNIT_ASSERT_NOTNULL(asp_csv_fp);

    pdt_load_csv_resources(asp_resource, asp_csv_fp, 0, 0);
    CUNIT_ASSERT_NOTNULL(asp_resource);
    fclose(asp_csv_fp);
    
    CUNIT_ASSERT_EQUALS(int, 1, element_count(asp_resource));

    pdt_process_file(PDT_ASP_INPUT2, PDT_ASP_ACTUAL2, PDT_MODE_REPLACE, 0);

    CUNIT_ASSERT_EQUALS(contents , PDT_ASP_EXPECTED2, PDT_ASP_ACTUAL2);
    CUNIT_ASSERT_EQUALS(contents , PDT_INC_EXPECTED1, PDT_INC_ACTUAL2);
    CUNIT_ASSERT_EQUALS(contents , PDT_INC_EXPECTED2, PDT_INC_ACTUAL3);

    pdt_cleanup(asp_resource);
    pdt_destroy_asp_param(asp_param);
}

/*=========================================================================
 * CASE:          pd_test_asp_replace_3
 * TYPE:          unit test case
 * OVERVIEW:      test for ASP replace case 3, 5 of
 *                PDE-UTD-DrawReplace.doc.
 *=======================================================================*/
void CUNITCBK pd_test_asp_replace_3(void)
{
    vector_t *asp_resource = pdt_startup();
    FILE * asp_csv_fp = NULL;
    pdt_param_t asp_param = 0;

    CUNIT_ASSERT_NOTNULL(asp_resource);

    asp_param = pdt_create_asp_param(LANGUAGE_VB, PDT_INC_INPUT2);

    pdt_enable_sufix_processor(".asp", asp_resource, "japanese", "english", asp_param);
    asp_csv_fp = fopen(PDT_ASP_RESOURCE, "r");
    CUNIT_ASSERT_NOTNULL(asp_csv_fp);

    pdt_load_csv_resources(asp_resource, asp_csv_fp, 0, 0);
    CUNIT_ASSERT_NOTNULL(asp_resource);
    fclose(asp_csv_fp);
    
    CUNIT_ASSERT_EQUALS(int, 1, element_count(asp_resource));

    pdt_process_file(PDT_ASP_INPUT3, PDT_ASP_ACTUAL3, PDT_MODE_REPLACE, 0);

    CUNIT_ASSERT_EQUALS(contents , PDT_ASP_EXPECTED3, PDT_ASP_ACTUAL3);

    pdt_cleanup(asp_resource);
    pdt_destroy_asp_param(asp_param);
}

/*=========================================================================
 * CASE:          pd_test_vb_replace_1
 * TYPE:          unit test case
 * OVERVIEW:      test for VB replace case 1(.frm).
 *=======================================================================*/
void CUNITCBK pd_test_vb_replace_1(void)
{
    vector_t *vb_resource = pdt_startup();
    FILE * vb_csv_fp = NULL;

    CUNIT_ASSERT_NOTNULL(vb_resource);

    pdt_enable_sufix_processor(".frm", vb_resource, "japanese", "english", 0);
    vb_csv_fp = fopen(PDT_VB_RESOURCE, "r");
    CUNIT_ASSERT_NOTNULL(vb_csv_fp);

    pdt_load_csv_resources(vb_resource, vb_csv_fp, 0, 0);
    CUNIT_ASSERT_NOTNULL(vb_resource);
    fclose(vb_csv_fp);
    
    CUNIT_ASSERT_EQUALS(int, 3, element_count(vb_resource));

    pdt_process_file(PDT_VB_INPUT1, PDT_VB_ACTUAL1, PDT_MODE_REPLACE, 0);

    CUNIT_ASSERT_EQUALS(contents , PDT_VB_EXPECTED1, PDT_VB_ACTUAL1);

    pdt_cleanup(vb_resource);
}


CUNIT_BEGIN_SUITE(pd_test_vb_draw)
    CUNIT_INCLUDE_CASE(pd_test_vb_draw_1)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_asp_draw)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_sql_draw)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_vb_replace)
    CUNIT_INCLUDE_CASE(pd_test_vb_replace_1)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_asp_replace)
    CUNIT_INCLUDE_CASE(pd_test_asp_replace_1)
    CUNIT_INCLUDE_CASE(pd_test_asp_replace_2)
    CUNIT_INCLUDE_CASE(pd_test_asp_replace_3)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_sql_replace)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_draw)
    CUNIT_INCLUDE_SUITE(pd_test_vb_draw)
    CUNIT_INCLUDE_SUITE(pd_test_asp_draw)
    CUNIT_INCLUDE_SUITE(pd_test_sql_draw)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_replace)
    CUNIT_INCLUDE_SUITE(pd_test_vb_replace)
    CUNIT_INCLUDE_SUITE(pd_test_asp_replace)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_diff)
CUNIT_END_SUITE

CUNIT_BEGIN_SUITE(pd_test_resgen)
CUNIT_END_SUITE

/*=========================================================================
 * SUITE:         pd_test_all
 * TYPE:          unit test suites
 * OVERVIEW:      Test for all draw/replace/diff/resgen functions.
 *=======================================================================*/
CUNIT_BEGIN_SUITE(pd_test_all)
    CUNIT_INCLUDE_SUITE(pd_test_draw)
    CUNIT_INCLUDE_SUITE(pd_test_replace)
    CUNIT_INCLUDE_SUITE(pd_test_diff)
    CUNIT_INCLUDE_SUITE(pd_test_resgen)
CUNIT_END_SUITE

/*=========================================================================
 * Runner:        main
 * TYPE:          c program entrance
 * OVERVIEW:      This function demonstrates use of cunit default
 *                TestRunner to run TestSuite(s) and TestCase(s).
 * INTERFACE:
 *   parameters:  command arguments
 *   returns:     exit code
 * NOTE:          TestResult will be printed into the debug context and
 *                STDOUT.
 *=======================================================================*/
int main(int argc, char **argv) {
    /* run test suite */
    CUNIT_RUN_SUITE(pd_test_all);
}
