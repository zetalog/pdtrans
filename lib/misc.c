#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

/* miscellaneous functions */
void chomp(char *line)
{
    char *last = line+strlen(line);

    for (last; last != line; last--)
    {
        if (strchr(" \r\n\t", *last))
            *last = '\0';
        else
            break;
    }
}

char *get_ext(const char *path)
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    static char ext[_MAX_EXT];

    _splitpath(path, drive, dir, fname, ext);
    return ext;
}

void strupper(char *string)
{
    for (; string && *string; string++)
    {
        if (islower(*string))
            *string = _toupper(*string);
    }
}

char *stristr(const char *string, const char *set)
{
    char *_string = NULL;
    char *_set = NULL;
    char *position;
    char *result = NULL;

    if (!string || !set)
        return NULL;

    _string = strdup(string);
    _set = strdup(set);

    assert(_string && _set);

    strupper(_string);
    strupper(_set);

    position = strstr(_string, _set);
    result = (char *)(position?string+(position-_string):NULL);

    free(_string);
    free(_set);

    return result;
}

int pdt_must_localize(char *string)
{
    unsigned char *temp = string;

    if (!temp)
        return 0;

    for (temp = string; *temp; temp++)
    {
        if (*temp >= 0x80)
            return 1;
    }

    return 0;
}

int create_buffer(char **bufptr, int *n, char **posptr, int *avail)
{
    if (!*bufptr)
    {
        *n = MIN_CHUNK;
        *bufptr = malloc (*n);
        if (!*bufptr)
        {
            errno = ENOMEM;
            return -1;
        }
    }
    
    *avail = *n;
    *posptr = *bufptr;

    return 0;
}

int read_char(FILE *stream, char **bufptr, int *n, char **posptr, int *avail, char *pchar)
{
    int save_errno;

    *pchar = getc (stream);
    
    save_errno = errno;
    
    /* We always want at least one char left in the buffer, since we
       always (unless we get an error while reading the first char)
       NUL-terminate the line buffer.  */
    
    if (*avail < 2)
    {
        if (*n > MIN_CHUNK)
            *n *= 2;
        else
            *n += MIN_CHUNK;
        
        *avail = *n + *bufptr - *posptr;
        *bufptr = realloc (*bufptr, *n);
        if (!*bufptr)
        {
            errno = ENOMEM;
            return -1;
        }
        *posptr = *n - *avail + *bufptr;
        assert ((*bufptr + *n) == (*posptr + *avail));
    }

    if (ferror (stream))
    {
        /* Might like to return partial line, but there is no
           place for us to store errno.  And we don't want to just
           lose errno.  */
        errno = save_errno;
        return -1;
    }
    
    if (*pchar == EOF)
    {
        /* Return partial line, if any.  */
        if (*posptr == *bufptr)
            return -1;
        else
            return 0;
    }
    
    *(*posptr)++ = *pchar;
    *avail = *avail-1;

    return 1;
}

void pdt_output_word(char *block, int size, char *word, FILE *stream)
{
    int index;

    for (index = 0; index < size; index++)
    {
        fputc(block[index], stream);
    }
    if (word)
        fprintf(stream, "%s", word);
}

int pdt_compare_dir(const char *src_path, const char *dst_path)
{
    char full_src_path[MAX_PATH];
    char full_dst_path[MAX_PATH];
    char cwd_path[MAX_PATH];

    if (src_path && (strlen(src_path) == 0))
        return 0;

    if (dst_path && (strlen(dst_path) == 0))
        return 0;
    
    if (getcwd(cwd_path, MAX_PATH) == NULL) 
    {
        pdt_report_error(PDT_REPORT_ERROR, "Error getting current working direct\nAbort replacing!\n", 0);
        return 0;
    }

    if (src_path[1] != ':') 
        sprintf(full_src_path, "%s%s", cwd_path, src_path);
    if (dst_path[1] != ':')
        sprintf(full_dst_path, "%s%s", cwd_path, dst_path);

    return strcmp(full_src_path, full_dst_path);
}

void pdt_dir_back(char *dir)
{
    int index = 0;
    assert(dir && dir[strlen(dir) - 1] == '\\');
    index = strlen(dir) - 1;
    do
    {
        index--;
        assert(dir[index] != ':');
    }
    while (dir[index] != '\\');
    dir[++index] = PDT_TERMINATOR;
    return;
}

void pdt_dir_cat(char *base_dir, char *cat)
{
    int index = 0;
    int base_len = 0;
    assert(base_dir && cat && strlen(base_dir) < MAX_PATH);

    base_len = strlen(base_dir);
    if (*(base_dir+base_len-1) != '\\')
    {
        *(base_dir+base_len) = '\\';
        base_len++;
    }

    while (cat[index])
    {
        base_dir[base_len + index] = cat[index];
        index++;
    }
    base_dir[base_len + index] = PDT_TERMINATOR;

    assert(strlen(base_dir) < MAX_PATH);
    return;
}

char *pdt_include_to_path(const char *ref_path, const char *include)
{
    static char return_path[MAX_PATH];
    char already_read[MAX_PATH];
    int  buffer_index = 0;
    char cwd_path[MAX_PATH];
    char *rel_path_dup = NULL;
    char *ppos = NULL;
    int index = 0;

    assert(ref_path && include);

    if (include[1] == ':')
    {

        strcpy(return_path, include);
        return return_path;
 
        /*
        pdt_report_error(PDT_REPORT_ERROR, "Can't guess the output path for absolute path %s!", include);
        return NULL;
       */
    }

    if (ref_path[1] == ':')
    {
        strcpy(return_path, ref_path);
    }
    else
    {
        if (getcwd(cwd_path, MAX_PATH) == NULL) 
        {
            pdt_report_error(PDT_REPORT_ERROR, "Error getting current working direct!\n", 0);
            return NULL;
        }
        else
        {
            _fullpath(return_path, ref_path, MAX_PATH);
        }
    }

    index = strlen(return_path) - 1;
    while (!pdt_is_dir(return_path) && *(return_path+strlen(return_path)-1) != ':')
    {
        do
        {
            index--;
        }
        while (return_path[index] != '\\' && return_path[index] != '/');
        return_path[index] = PDT_TERMINATOR;
    }

    assert(pdt_is_dir(return_path) || return_path[1] == ':');

    return_path[index++] = '\\';
    return_path[index] = PDT_TERMINATOR;

    rel_path_dup = strdup(include);
    if (include[0] == '\\')
    {
        rel_path_dup++;
    }

    index = 0;
    buffer_index = 0;
    do
    {
        if ((rel_path_dup[index] == '/') ||
            (rel_path_dup[index] == '\\'))
        {
            rel_path_dup[index] = '\\';
            already_read[buffer_index] = PDT_TERMINATOR;
            if (strcmp(already_read, ".") == 0) 
            {
                /* must not do anything here */
                ;
            }
            else if (strcmp(already_read, "..") == 0)
            {
                pdt_dir_back(return_path);
            }
            else
            {
                pdt_dir_cat(return_path, already_read);

                if (access(return_path, 0)) 
                {
                    mkdir(return_path);
                }

            }
            buffer_index = 0;
        }
        else
        {
            already_read[buffer_index++] = rel_path_dup[index];
        }
        index++;
    }
    while (rel_path_dup[index] != PDT_TERMINATOR);

    already_read[buffer_index++] = PDT_TERMINATOR;
    pdt_dir_cat(return_path, already_read);

    if (rel_path_dup)
        free(rel_path_dup);

    return return_path;
}

/**
 * Return the relative path of src_file according to path
 * If path and ref_path have nothing in common return the absolute path
 * Remember to strdup the return value!
 */
char *pdt_include_from_path(const char *ref_path, const char *path)
{
    char *absolute_base_file;
    char *absolute_src_file;
    static char return_relpath[MAX_PATH];
    int up_deep = 0;
    int index_common = 0;
    int index_base = 0;
    int index = 0;
    char * pindex = NULL;

    assert(path && ref_path);

    absolute_base_file = strdup(pdt_get_absolute_path(path));
    if (absolute_base_file == NULL)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Error getting absolute path for %s\n", path);
        return NULL;
    }

    absolute_src_file = strdup(pdt_get_absolute_path(ref_path));
    if (absolute_src_file == NULL)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Error getting absolute path for %s\n", path);
        return NULL;
    }

    /* if it's an Include file, it may not exist */
    //assert(pdt_is_file(absolute_base_file) && pdt_is_file(absolute_src_file));

    while (absolute_base_file[index_common] == absolute_src_file[index_common])
    {
        index_common++;
    }

    if (absolute_base_file[index_common] != '\\')
    {
        while (absolute_base_file[index_common] != '\\')
        {
            index_common--;
        }
    }

    assert(absolute_base_file[index_common] == '\\');

    /* skip the last common '\\' */
    index_common++;
    index_base = index_common;

    assert(absolute_base_file[index_common] && absolute_src_file[index_common]);

    while (absolute_src_file[index_common])
    {
        if (absolute_src_file[index_common++] == '\\') up_deep++;
    }

    for (index = 0; index < up_deep; index++)
    {
        return_relpath[3*index + 0] = '.';
        return_relpath[3*index + 1] = '.';
        return_relpath[3*index + 2] = '/';
    }
    return_relpath[up_deep*3] = PDT_TERMINATOR;
    strcat(return_relpath, (absolute_base_file + index_base));

    do 
    {
        pindex = strchr(return_relpath, '\\') ;
        if (pindex) 
            *pindex = '/';
        else
            break;
    }
    while (1);

    if (absolute_base_file)
        free(absolute_base_file);
    if (absolute_src_file)
        free(absolute_src_file);

    return return_relpath;
}

char *pdt_get_absolute_path(const char *path)
{
    char cwd_path[MAX_PATH];
    static char absolute_path[MAX_PATH];

    assert(path && strlen(path) >= 1);

    if (getcwd(cwd_path, MAX_PATH) == NULL) 
    {
        pdt_report_error(PDT_REPORT_ERROR, "Error getting current working direct!\n", 0);
        return NULL;
    }

    if (strlen(path) == 1)
    {
        if (path[0] == '.')
        {
            strcpy(absolute_path, cwd_path);
        }
        else
        {
            pdt_report_error(PDT_REPORT_ERROR, "Error get absolute path for %s!\n", path);
            return NULL;
        }
    }
    else
    {
        if (path[1] == ':')
        {
            strcpy(absolute_path, path);
        }
        else
        {
            _fullpath(absolute_path, path, MAX_PATH);
        }
    }
    return absolute_path;

}

int pdt_copy_file(const char *src_file, const char *dst_file) 
{
    int  retval = 0;
    int  count, total = 0;
    char buffer[BUFFER_SIZE];
    FILE *src_stream = NULL;
    FILE *dst_stream = NULL;
    
    src_stream = fopen(src_file, "rb");
    if (!src_stream)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Failed open source file %d.\n", GetLastError());
        retval = -1;
        goto failure;
    }
    dst_stream = fopen(dst_file, "wb");
    if (!dst_stream)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Failed open destination file %d.\n", GetLastError());
        retval = -1;
        goto failure;
    }
    
    /* Cycle until end of file reached: */
    while (!feof(src_stream))
    {
        /* Attempt to read in BUFFER_SIZE bytes: */
        count = fread( buffer, sizeof(char), BUFFER_SIZE, src_stream );
        if (count < 0 || ferror(src_stream))
        {
            pdt_report_error(PDT_REPORT_ERROR, "Failed copying file %s.\n", src_file);
            retval = -1;
            break;
        } 
        else
            fwrite(buffer, sizeof(char), count, dst_stream);
    }
failure:
    if (src_stream) 
    {
        fflush(src_stream);
        fclose(src_stream);
    }
    if (dst_stream)
    {
        fflush(dst_stream);
        fclose(dst_stream);
    }

   return retval;
}

int pdt_is_dir(const char *file)
{
    int retval;
    struct stat sb;
    
    retval = stat(file, &sb);
    if (retval < 0)
        return FALSE;
    return ((sb.st_mode & _S_IFDIR) == _S_IFDIR) ? TRUE : FALSE;
}

int pdt_is_file(const char *file)
{
    int retval;
    struct stat sb;
    
    retval = stat(file, &sb);
    if (retval < 0)
        return FALSE;
    return ((sb.st_mode & _S_IFREG) == _S_IFREG) ? TRUE : FALSE;
}
