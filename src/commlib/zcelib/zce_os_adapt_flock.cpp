#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_flock.h"

void ZCE_OS::flock_adjust_params (zce_flock_t *lock,
                                  int whence,
                                  size_t &start,
                                  size_t &len)
{

#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    switch (whence)
    {
        case SEEK_SET:
            break;

        case SEEK_CUR:
        {
            LARGE_INTEGER offset;
            LARGE_INTEGER distance;
            distance.QuadPart = 0;

            if (!::SetFilePointerEx (lock->handle_,
                                     distance,
                                     &offset,
                                     FILE_CURRENT))
            {
                errno = ZCE_OS::last_error();
                return;
            }

            start += static_cast<ssize_t> (offset.QuadPart);
        }
        break;

        case SEEK_END:
        {
            size_t file_size = 0;
            ret = ZCE_OS::filesize (lock->handle_, &file_size);

            if ( ret != 0)
            {
                return;
            }

            start += file_size;
        }
        break;
    }

    LARGE_INTEGER large_start;
    large_start.QuadPart  = start;

    lock->overlapped_.Offset = large_start.LowPart;
    lock->overlapped_.OffsetHigh = large_start.HighPart;

    //�������Ϊ0����ʶ��ȫ�ļ����в�������WINDOWSƽ̨�������ļ�����
    if (len == 0)
    {
        size_t file_size = 0;
        ret = ZCE_OS::filesize (lock->handle_, &file_size);

        if ( ret != 0)
        {
            return;
        }

        len = file_size - start;
    }

#elif defined (ZCE_OS_LINUX)

    lock->lock_.l_whence = whence;
    lock->lock_.l_start = start;
    lock->lock_.l_len = len;
#endif
}

//�ļ�����ʼ��zce_flock_t,���ļ����õ�FD
int ZCE_OS::flock_init (zce_flock_t *lock,
                        const char *file_name,
                        int flags,
                        mode_t perms)
{
    //
    ZCE_ASSERT(file_name && lock);

    ZCE_HANDLE fd = ZCE_OS::open (file_name, flags, perms);

    if (fd == ZCE_INVALID_HANDLE)
    {
        return -1;
    }

    //�����򿪵ı�־
    lock->open_by_self_ = true;

    return ZCE_OS::flock_init(lock, fd);
}

//�ļ�����ʼ��zce_flock_t,ֱ����fd
int ZCE_OS::flock_init (zce_flock_t *lock,
                        ZCE_HANDLE file_hadle)
{

#if defined (ZCE_OS_WINDOWS)

    // Once initialized, these values are never changed.
    lock->overlapped_.Internal = 0;
    lock->overlapped_.InternalHigh = 0;
    lock->overlapped_.hEvent = 0;

    //�������е�����
    lock->overlapped_.Offset = 0;
    lock->overlapped_.OffsetHigh = 0;

#endif

    lock->handle_ = file_hadle;

    return 0;
}

//���������������Ͳ����ر��������ʵӦ��˵������zce_flock_t����ṹ
int ZCE_OS::flock_destroy (zce_flock_t *lock)
{

    if (lock->handle_ != ZCE_INVALID_HANDLE)
    {

        if (lock->open_by_self_)
        {
            //close the handle.
            ZCE_OS::close (lock->handle_);
            lock->handle_ = ZCE_INVALID_HANDLE;
        }
    }

    return 0;
}

//whence == SEEK_SET,SEEK_CUR
int ZCE_OS::flock_unlock (zce_flock_t *lock,
                          int  whence,
                          size_t start,
                          size_t len)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;

    //�������
    BOOL ret_bool = ::UnlockFileEx (lock->handle_,
                                    0,
                                    large_len.LowPart,
                                    large_len.HighPart,
                                    &lock->overlapped_);

    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);
    // Unlock file.
    lock->lock_.l_type = F_UNLCK;
    return ::fcntl(lock->handle_,
                   F_SETLK,
                   reinterpret_cast<long> (&lock->lock_));
#endif

}

int ZCE_OS::flock_rdlock (zce_flock_t *lock,
                          int  whence,
                          size_t start,
                          size_t len)
{

#if defined (ZCE_OS_WINDOWS)
    ZCE_OS::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;
    //��2������Ϊ0��ʾ�ǹ�����
    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  0,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //���ִ���
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);
    // set read lock
    lock->lock_.l_type = F_RDLCK;
    //block, if no access ע��F_SETLKW��F_SETLK������
    return ::fcntl(lock->handle_,
                   F_SETLKW,
                   reinterpret_cast<long> (&lock->lock_));

#endif
}

//�����Ƿ���Լ��������������������
int ZCE_OS::flock_tryrdlock (::zce_flock_t *lock,
                             int  whence,
                             size_t start,
                             size_t len)
{

#if defined (ZCE_OS_WINDOWS)

    //������������ΪWINDOWS������һЩ�鷳
    ZCE_OS::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;
    //LOCKFILE_FAIL_IMMEDIATELY��ʾ�������в��ԣ�ʧ�ܲ�������Ҳ����try��
    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  LOCKFILE_FAIL_IMMEDIATELY,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //���ִ���
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);
    lock->lock_.l_type = F_RDLCK;         // set read lock

    int result = 0;
    // Does not block, if no access, returns -1 and set errno = EBUSY; ע��F_SETLKW��F_SETLK������
    result = ::fcntl(lock->handle_,
                     F_SETLK,
                     reinterpret_cast<long> (&lock->lock_));

    if (result == -1 && (errno == EACCES || errno == EAGAIN))
    {
        errno = EBUSY;
    }

    return 0;
#endif
}

//���Խ���д����
int ZCE_OS::flock_trywrlock (zce_flock_t *lock,
                             int  whence,
                             size_t start,
                             size_t len)
{

#if defined (ZCE_OS_WINDOWS)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;

    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //���ִ���
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);
    lock->lock_.l_type = F_WRLCK;         // set write lock

    int result = 0;
    // Does not block, if no access, returns -1 and set errno = EBUSY;
    result = ::fcntl(lock->handle_,
                     F_SETLK,
                     reinterpret_cast<long> (&lock->lock_));

    if (result == -1 && (errno == EACCES || errno == EAGAIN))
    {
        errno = EBUSY;
    }

    return 0;
#endif
}

int ZCE_OS::flock_wrlock (zce_flock_t *lock,
                          int whence,
                          size_t start,
                          size_t len)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;
    //LOCKFILE_EXCLUSIVE_LOCK��ʾ��ռ����Ҳ����д��
    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  LOCKFILE_EXCLUSIVE_LOCK,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //���ִ���
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_OS::flock_adjust_params (lock, whence, start, len);
    // set write lock
    lock->lock_.l_type = F_WRLCK;
    // block, if no access
    return ::fcntl(lock->handle_,
                   F_SETLKW,
                   reinterpret_cast<long> (&lock->lock_));

#endif
}

//LOCK_SH
//Place a shared lock. More than one process may hold a shared lock for a given file at a given time.
//LOCK_EX
//Place an exclusive lock. Only one process may hold an exclusive lock for a given file at a given time.
//LOCK_UN
//Remove an existing lock held by this process.
//LOCK_NB
//A call to flock() may block if an incompatible lock is held by another process. To make a non-blocking request, include LOCK_NB (by ORing) with any of the above operations.

//�ļ���������ֻ��һ���ļ����м���
int ZCE_OS::flock(ZCE_HANDLE file_hadle, int operation)
{
#if defined (ZCE_OS_WINDOWS)

    zce_flock_t lock_handle;

    int ret = 0;
    ret = ZCE_OS::flock_init (&lock_handle,
                              file_hadle);

    //��ȡ����������
    if (LOCK_SH & operation )
    {
        //����ǲ�������
        if ( LOCK_NB & operation)
        {
            ret = ZCE_OS::flock_tryrdlock (&lock_handle,
                                           SEEK_SET,
                                           0,
                                           0);
        }
        //�������������
        else
        {
            ret = ZCE_OS::flock_rdlock (&lock_handle,
                                        SEEK_SET,
                                        0,
                                        0);
        }

    }
    //д��
    else if (LOCK_EX & operation)
    {
        //����ǲ�������
        if ( LOCK_NB & operation)
        {
            ret = ZCE_OS::flock_trywrlock (&lock_handle,
                                           SEEK_SET,
                                           0,
                                           0);
        }
        //�������������
        else
        {
            ret = ZCE_OS::flock_wrlock (&lock_handle,
                                        SEEK_SET,
                                        0,
                                        0);
        }
    }
    //�⿪��
    else if ( LOCK_UN & operation)
    {
        ret = ZCE_OS::flock_unlock (&lock_handle,
                                    SEEK_SET,
                                    0,
                                    0);
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    if (ret != 0)
    {
        ZLOG_ERROR("[zcelib] ZCE_OS::flock fail. operation =%d ret =%d", operation, ret);
    }

    //��ʵ��ȫ���Բ������������,��������Ϊ��������ù���,
    ZCE_OS::flock_destroy(&lock_handle);

    return ret;

#elif defined (ZCE_OS_LINUX)
    return ::flock(file_hadle, operation);
#endif

}
