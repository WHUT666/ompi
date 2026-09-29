/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * mmap() family + dirent implementations for the native Windows build.
 */

#include "opal_config.h"

#ifdef _WIN32

#    include "opal/win32/opal_win32.h"

#    undef mmap
#    undef mmap64
#    undef munmap
#    undef mprotect
#    undef msync
#    undef madvise
#    undef posix_madvise
#    undef mlock
#    undef munlock
#    undef mlockall
#    undef munlockall
#    undef mincore
#    undef shm_open
#    undef shm_unlink
#    undef opendir
#    undef readdir
#    undef closedir
#    undef rewinddir
#    undef telldir
#    undef seekdir
#    undef dirfd
#    undef scandir
#    undef alphasort
#    undef versionsort
#    undef stat
#    undef lstat
#    undef mkdir
#    undef ftok

/* ================================================================== */
/* mmap                                                               */
/*                                                                    */
/* Strategy: every mapping gets a registry entry recording the        */
/* HANDLE of the file mapping object + the mapped base.  munmap       */
/* looks the entry up to unmap + close.                               */
/* ================================================================== */

struct mmap_ent {
    void   *addr;
    size_t  length;
    HANDLE  map;
    struct mmap_ent *next;
};

static CRITICAL_SECTION mmap_lock;
static volatile LONG mmap_lock_init = 0;
static struct mmap_ent *mmap_list = NULL;

static void mmap_lock_once(void)
{
    if (0 == InterlockedCompareExchange(&mmap_lock_init, 1, 0)) {
        InitializeCriticalSection(&mmap_lock);
        InterlockedExchange(&mmap_lock_init, 2);
    }
    while (1 == mmap_lock_init) {
        Sleep(0);
    }
}

static void mmap_register(void *addr, size_t length, HANDLE map)
{
    struct mmap_ent *e = calloc(1, sizeof(*e));
    if (NULL == e) {
        return;
    }
    e->addr = addr;
    e->length = length;
    e->map = map;
    mmap_lock_once();
    EnterCriticalSection(&mmap_lock);
    e->next = mmap_list;
    mmap_list = e;
    LeaveCriticalSection(&mmap_lock);
}

static HANDLE mmap_lookup(void *addr, size_t *length)
{
    struct mmap_ent *e;
    HANDLE map = NULL;
    mmap_lock_once();
    EnterCriticalSection(&mmap_lock);
    for (e = mmap_list; e; e = e->next) {
        if (e->addr == addr) {
            map = e->map;
            if (length) {
                *length = e->length;
            }
            break;
        }
    }
    LeaveCriticalSection(&mmap_lock);
    return map;
}

static void mmap_unregister(void *addr)
{
    struct mmap_ent **pp, *e;
    mmap_lock_once();
    EnterCriticalSection(&mmap_lock);
    for (pp = &mmap_list; NULL != (e = *pp); pp = &e->next) {
        if (e->addr == addr) {
            *pp = e->next;
            free(e);
            break;
        }
    }
    LeaveCriticalSection(&mmap_lock);
}

static DWORD prot_to_page(int prot)
{
    if (prot & PROT_EXEC) {
        return (prot & PROT_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    }
    if (prot & PROT_WRITE) {
        return PAGE_READWRITE;
    }
    if (prot & PROT_READ) {
        return PAGE_READONLY;
    }
    return PAGE_NOACCESS;
}

static DWORD prot_to_map(int prot)
{
    DWORD acc = 0;
    if (prot & PROT_EXEC) {
        acc |= FILE_MAP_EXECUTE;
    }
    if (prot & PROT_WRITE) {
        acc |= FILE_MAP_WRITE;
    }
    if (prot & PROT_READ) {
        acc |= FILE_MAP_READ;
    }
    if (0 == acc) {
        acc = FILE_MAP_READ;
    }
    return acc;
}

OPAL_WIN32_DECLSPEC void *opal_win32_mmap(void *addr, size_t length, int prot, int flags, int fd,
                      off_t offset)
{
    HANDLE fh = INVALID_HANDLE_VALUE;
    HANDLE map;
    void *view;
    DWORD page = prot_to_page(prot);
    DWORD mapacc = prot_to_map(prot);
    DWORD off_hi = (DWORD) ((unsigned long long) offset >> 32);
    DWORD off_lo = (DWORD) ((unsigned long long) offset & 0xffffffff);

    if (!(flags & (MAP_ANON | MAP_ANONYMOUS)) && fd >= 0) {
        fh = (HANDLE) _get_osfhandle(fd);
        if (INVALID_HANDLE_VALUE == fh) {
            errno = EBADF;
            return MAP_FAILED;
        }
    }
    /* grow the file if the mapping extends past its current end */
    if (fh != INVALID_HANDLE_VALUE && (prot & PROT_WRITE)) {
        LARGE_INTEGER cur, want;
        want.QuadPart = (LONGLONG) offset + (LONGLONG) length;
        if (GetFileSizeEx(fh, &cur) && cur.QuadPart < want.QuadPart) {
            SetFilePointerEx(fh, want, NULL, FILE_BEGIN);
            SetEndOfFile(fh);
        }
    }
    map = CreateFileMappingW(fh, NULL, page, (DWORD) (((unsigned long long) length) >> 32),
                             (DWORD) (length & 0xffffffff), NULL);
    if (NULL == map) {
        errno = ENOMEM;
        return MAP_FAILED;
    }
    view = MapViewOfFileEx(map, mapacc, off_hi, off_lo, length,
                         (flags & MAP_FIXED) ? addr : NULL);
    if (NULL == view) {
        CloseHandle(map);
        errno = ENOMEM;
        return MAP_FAILED;
    }
    mmap_register(view, length, map);
    return view;
}

OPAL_WIN32_DECLSPEC void *opal_win32_mmap64(void *addr, size_t length, int prot, int flags, int fd,
                        long long offset)
{
    return opal_win32_mmap(addr, length, prot, flags, fd, (off_t) offset);
}

OPAL_WIN32_DECLSPEC int opal_win32_munmap(void *addr, size_t length)
{
    HANDLE map;
    size_t len = 0;
    (void) length;
    map = mmap_lookup(addr, &len);
    if (NULL == map) {
        errno = EINVAL;
        return -1;
    }
    UnmapViewOfFile(addr);
    CloseHandle(map);
    mmap_unregister(addr);
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_mprotect(void *addr, size_t len, int prot)
{
    DWORD oldp;
    if (0 == VirtualProtect(addr, len, prot_to_page(prot), &oldp)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_msync(void *addr, size_t len, int flags)
{
    (void) flags;
    if (0 == FlushViewOfFile(addr, len)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_madvise(void *addr, size_t len, int advice)
{
    (void) addr;
    (void) len;
    (void) advice;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_posix_madvise(void *addr, size_t len, int advice)
{
    (void) addr;
    (void) len;
    (void) advice;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_mlock(const void *addr, size_t len)
{
    return VirtualLock((LPVOID) addr, len) ? 0 : -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_munlock(const void *addr, size_t len)
{
    return VirtualUnlock((LPVOID) addr, len) ? 0 : -1;
}

OPAL_WIN32_DECLSPEC int opal_win32_mlockall(int flags)
{
    (void) flags;
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_munlockall(void)
{
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_mincore(void *addr, size_t length, unsigned char *vec)
{
    size_t i, npages = (length + 4095) / 4096;
    (void) addr;
    for (i = 0; i < npages; i++) {
        vec[i] = 1;
    }
    return 0;
}

OPAL_WIN32_DECLSPEC int opal_win32_shm_open(const char *name, int oflag, mode_t mode)
{
    /* map the POSIX name onto a Windows named pagefile-backed mapping;
     * the fd we return wraps the HANDLE so close() releases it */
    char wname[MAX_PATH + 16];
    HANDLE h;
    int access = ((oflag & O_RDWR) || !(oflag & O_WRONLY)) ? PAGE_READWRITE
                                                          : PAGE_READWRITE;
    int fd;
    (void) mode;
    snprintf(wname, sizeof(wname), "Local\\ompi_shm_%s",
             (name[0] == '/' || name[0] == '\\') ? name + 1 : name);
    if (oflag & O_CREAT) {
        h = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, access, 0,
                               8 /* initial small */, NULL);
        if (NULL == h) {
            errno = ENOMEM;
            return -1;
        }
    } else {
        h = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, NULL);
        if (NULL == h) {
            errno = ENOENT;
            return -1;
        }
    }
    (void) wname;
    fd = _open_osfhandle((intptr_t) h, _O_RDWR);
    if (fd < 0) {
        CloseHandle(h);
        errno = EMFILE;
        return -1;
    }
    return fd;
}

OPAL_WIN32_DECLSPEC int opal_win32_shm_unlink(const char *name)
{
    (void) name;
    /* named mappings are refcounted; unlink is a no-op */
    return 0;
}

/* ================================================================== */
/* dirent                                                             */
/* ================================================================== */

struct DIR {
    HANDLE           h;
    WIN32_FIND_DATAA fdata;
    struct dirent    ent;
    long             pos;
    int              first_pending;
    int              fd;
    char             path[MAX_PATH];
};

DIR *opendir(const char *name)
{
    DIR *d;
    char pattern[MAX_PATH];
    size_t len = strlen(name);
    if (0 == len) {
        errno = ENOENT;
        return NULL;
    }
    d = calloc(1, sizeof(*d));
    if (NULL == d) {
        errno = ENOMEM;
        return NULL;
    }
    snprintf(pattern, sizeof(pattern), "%s%s*", name,
             (name[len - 1] == '/' || name[len - 1] == '\\') ? "" : "/");
    d->h = FindFirstFileA(pattern, &d->fdata);
    if (INVALID_HANDLE_VALUE == d->h) {
        free(d);
        errno = ENOENT;
        return NULL;
    }
    d->first_pending = 1;
    d->fd = -1;
    strncpy(d->path, name, sizeof(d->path) - 1);
    d->path[sizeof(d->path) - 1] = '\0';
    return d;
}

struct dirent *readdir(DIR *d)
{
    if (NULL == d) {
        return NULL;
    }
    if (d->first_pending) {
        d->first_pending = 0;
    } else {
        if (!FindNextFileA(d->h, &d->fdata)) {
            return NULL;
        }
    }
    strncpy(d->ent.d_name, d->fdata.cFileName, NAME_MAX);
    d->ent.d_name[NAME_MAX] = '\0';
    d->ent.d_ino = 0;
    d->ent.d_reclen = (unsigned short) sizeof(d->ent);
    d->ent.d_type = (d->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        ? DT_DIR
                        : DT_REG;
    d->pos++;
    return &d->ent;
}

OPAL_WIN32_DECLSPEC int closedir(DIR *d)
{
    if (NULL == d) {
        return -1;
    }
    if (INVALID_HANDLE_VALUE != d->h) {
        FindClose(d->h);
    }
    /* fdopendir() transferred ownership of the descriptor to us */
    if (d->fd >= 0) {
        _close(d->fd);
    }
    free(d);
    return 0;
}

/* fdopendir(): recover the directory path from the descriptor's handle
 * and hand it to opendir(); the DIR owns the fd afterwards. */
DIR *fdopendir(int fd)
{
    HANDLE h = (HANDLE) _get_osfhandle(fd);
    char path[MAX_PATH];
    DWORD n;
    DIR *d;

    if (INVALID_HANDLE_VALUE == h) {
        errno = EBADF;
        return NULL;
    }
    n = GetFinalPathNameByHandleA(h, path, sizeof(path), FILE_NAME_NORMALIZED);
    if (0 == n || n >= sizeof(path)) {
        errno = ENOTDIR;
        return NULL;
    }
    /* strip the \\?\ prefix and normalize separators */
    if (0 == strncmp(path, "\\\\?\\", 4)) {
        memmove(path, path + 4, strlen(path + 4) + 1);
    }
    {
        char *p;
        for (p = path; *p; ++p) {
            if ('\\' == *p) {
                *p = '/';
            }
        }
    }
    d = opendir(path);
    if (NULL == d) {
        return NULL;
    }
    d->fd = fd;
    return d;
}

OPAL_WIN32_DECLSPEC void rewinddir(DIR *d)
{
    (void) d;
    /* not supported: reopen semantics needed; callers don't use it */
}

OPAL_WIN32_DECLSPEC long telldir(DIR *d)
{
    return d ? d->pos : -1;
}

OPAL_WIN32_DECLSPEC void seekdir(DIR *d, long loc)
{
    (void) d;
    (void) loc;
}

OPAL_WIN32_DECLSPEC int dirfd(DIR *d)
{
    return d ? d->fd : -1;
}

OPAL_WIN32_DECLSPEC int alphasort(const struct dirent **a, const struct dirent **b)
{
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

OPAL_WIN32_DECLSPEC int versionsort(const struct dirent **a, const struct dirent **b)
{
    return alphasort(a, b);
}

OPAL_WIN32_DECLSPEC int scandir(const char *dirp, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **))
{
    DIR *d;
    struct dirent *e;
    size_t n = 0, cap = 16;
    struct dirent **list;

    d = opendir(dirp);
    if (NULL == d) {
        return -1;
    }
    list = malloc(cap * sizeof(*list));
    if (NULL == list) {
        closedir(d);
        return -1;
    }
    while (NULL != (e = readdir(d))) {
        if (filter && !filter(e)) {
            continue;
        }
        if (n == cap) {
            struct dirent **nl;
            cap *= 2;
            nl = realloc(list, cap * sizeof(*list));
            if (NULL == nl) {
                break;
            }
            list = nl;
        }
        list[n] = malloc(sizeof(*e));
        if (NULL == list[n]) {
            break;
        }
        *list[n] = *e;
        n++;
    }
    closedir(d);
    if (compar && n > 1) {
        qsort(list, n, sizeof(*list),
              (int (*)(const void *, const void *)) compar);
    }
    *namelist = list;
    return (int) n;
}

/* ================================================================== */
/* misc file ops                                                      */
/* ================================================================== */

OPAL_WIN32_DECLSPEC key_t ftok(const char *pathname, int proj_id)
{
    DWORD attrs = GetFileAttributesA(pathname);
    DWORD vol = 0, hi = 0, lo = 0;
    if (INVALID_FILE_ATTRIBUTES == attrs) {
        return (key_t) -1;
    }
    /* hash the absolute path as a stable "inode" surrogate */
    {
        char full[MAX_PATH];
        DWORD n = GetFullPathNameA(pathname, MAX_PATH, full, NULL);
        DWORD hash = 2166136261u;
        DWORD i;
        for (i = 0; i < n; i++) {
            hash = (hash ^ (unsigned char) full[i]) * 16777619u;
        }
        lo = hash;
    }
    return (key_t) (((proj_id & 0xff) << 24) | (lo & 0xffffff));
}

/* ================================================================== */
/* dirfd-relative (*at) operations                                    */
/* ================================================================== */

OPAL_WIN32_DECLSPEC int opal_win32_resolve_at(int dirfd, const char *name, char *out,
                          size_t outlen)
{
    size_t len;
    HANDLE h;
    DWORD n;
    char *p;

    if (NULL == name || NULL == out || 0 == outlen) {
        errno = EINVAL;
        return -1;
    }
    /* absolute names and AT_FDCWD resolve directly against the cwd */
    if (AT_FDCWD == dirfd || '/' == name[0] || '\\' == name[0]
        || (isalpha((unsigned char) name[0]) && ':' == name[1])) {
        if (strlen(name) >= outlen) {
            errno = ENAMETOOLONG;
            return -1;
        }
        strcpy(out, name);
        return 0;
    }
    h = (HANDLE) _get_osfhandle(dirfd);
    if (INVALID_HANDLE_VALUE == h) {
        errno = EBADF;
        return -1;
    }
    n = GetFinalPathNameByHandleA(h, out, (DWORD) outlen,
                                FILE_NAME_NORMALIZED);
    if (0 == n || n >= outlen) {
        errno = EBADF;
        return -1;
    }
    if (0 == strncmp(out, "\\\\?\\", 4)) {
        memmove(out, out + 4, strlen(out + 4) + 1);
    }
    for (p = out; *p; ++p) {
        if ('\\' == *p) {
            *p = '/';
        }
    }
    len = strlen(out);
    while (len > 3 && '/' == out[len - 1]) {
        out[--len] = '\0';
    }
    if (len + 1 + strlen(name) >= outlen) {
        errno = ENAMETOOLONG;
        return -1;
    }
    out[len] = '/';
    strcpy(out + len + 1, name);
    return 0;
}

OPAL_WIN32_DECLSPEC int openat(int dirfd, const char *pathname, int flags, ...);
OPAL_WIN32_DECLSPEC int opal_win32_open(const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    va_list ap;
    int fd;

    va_start(ap, flags);
    if (flags & _O_CREAT) {
        mode = va_arg(ap, mode_t);
    }
    va_end(ap);
    fd = openat(AT_FDCWD, pathname, flags, mode);
    return fd;
}

OPAL_WIN32_DECLSPEC int openat(int dirfd, const char *pathname, int flags, ...)
{
    char full[MAX_PATH];
    mode_t mode = 0;
    va_list ap;

    va_start(ap, flags);
    if (flags & _O_CREAT) {
        mode = va_arg(ap, mode_t);
    }
    va_end(ap);

    if (0 != opal_win32_resolve_at(dirfd, pathname, full, sizeof(full))) {
        return -1;
    }
    if (flags & O_NOFOLLOW) {
        DWORD attr = GetFileAttributesA(full);
        if (INVALID_FILE_ATTRIBUTES != attr
            && (attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
            errno = ELOOP;
            return -1;
        }
    }
    if (flags & O_DIRECTORY) {
        HANDLE h;
        DWORD attr;
        int fd;
        attr = GetFileAttributesA(full);
        if (INVALID_FILE_ATTRIBUTES == attr) {
            errno = ENOENT;
            return -1;
        }
        if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            errno = ENOTDIR;
            return -1;
        }
        h = CreateFileA(full, GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
        if (INVALID_HANDLE_VALUE == h) {
            errno = EACCES;
            return -1;
        }
        fd = _open_osfhandle((intptr_t) h, _O_RDONLY | _O_NOINHERIT);
        if (fd < 0) {
            CloseHandle(h);
            return -1;
        }
        return fd;
    }
    return _open(full, flags, mode);
}

OPAL_WIN32_DECLSPEC int unlinkat(int dirfd, const char *pathname, int flags)
{
    char full[MAX_PATH];
    if (0 != opal_win32_resolve_at(dirfd, pathname, full, sizeof(full))) {
        return -1;
    }
    if (flags & AT_REMOVEDIR) {
        return _rmdir(full);
    }
    return _unlink(full);
}

#endif /* _WIN32 */
