#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_socket_addr_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_trace_log_debug.h"
#include "zce_socket_base.h"

//���캯��
ZCE_Socket_Base::ZCE_Socket_Base():
    socket_handle_(ZCE_INVALID_SOCKET)
{
}

ZCE_Socket_Base::ZCE_Socket_Base(const ZCE_SOCKET &socket_handle):
    socket_handle_(socket_handle)
{
}

ZCE_Socket_Base::~ZCE_Socket_Base()
{
}

//
void ZCE_Socket_Base::set_handle(const ZCE_SOCKET &socket_handle)
{
    socket_handle_ = socket_handle;
}

//
ZCE_SOCKET ZCE_Socket_Base::get_handle() const
{
    return socket_handle_;
}

//Open SOCK�������BIND���ص�ַ�ķ�ʽ
int ZCE_Socket_Base::open(int type,
                          int family,
                          int protocol,
                          bool reuse_addr)
{
    int ret = 0;

    //��ֹ��ɻ��£��ظ����ã������Դ�޷��ͷ�
    assert(socket_handle_ == ZCE_INVALID_SOCKET);

    socket_handle_ = ZCE_OS::socket(family, type, protocol);
    if (ZCE_INVALID_SOCKET == socket_handle_ )
    {
        int last_err = ZCE_OS::last_error();
        ZLOG_ERROR("ZCE_OS::socket return fail last error %d|%s.",
            last_err,
            strerror(last_err));
        return -1;
    }

    //���ҪREUSE�����ַ
    if (reuse_addr)
    {
        int one = 1;
        ret = ZCE_OS::setsockopt (socket_handle_,
                                  SOL_SOCKET,
                                  SO_REUSEADDR,
                                  &one,
                                  sizeof (int));

        if (ret != 0)
        {
            ZCE_OS::closesocket(socket_handle_);
            return -1;
        }
    }

    return 0;
}

//Open SOCK�����BIND��ַ�ķ�ʽ
int ZCE_Socket_Base::open(int type,
                          const ZCE_Sockaddr *local_addr,
                          int family,
                          int protocol,
                          bool reuse_addr)
{
    int ret = 0;

    //���û�б�עЭ��أ���bind�ı��ص�ַ��Э��ر�ʶ
    if (local_addr && family == AF_UNSPEC )
    {
        family = local_addr->sockaddr_ptr_->sa_family;
    }

    //�����ַЭ���socket��Э��ز�һ����
    if (local_addr && family != local_addr->sockaddr_ptr_->sa_family)
    {
        assert(false);
        return -1;
    }

    ret = this->open(type,
                     family,
                     protocol,
                     reuse_addr);

    //���Ҫ�󶨱��ص�ַ��һ��SOCKET����˲�
    if (local_addr)
    {
        ret = ZCE_Socket_Base::bind(local_addr);

        if (ret != 0)
        {
            close();
            return ret;
        }
    }

    return 0;

}

//�ر�֮
int ZCE_Socket_Base::close()
{
    int ret = ZCE_OS::closesocket(socket_handle_);

    if (0 == ret)
    {
        socket_handle_ = ZCE_INVALID_SOCKET;
    }

    return ret;
}

int ZCE_Socket_Base::bind(const ZCE_Sockaddr *add_name) const
{
    return ZCE_OS::bind(socket_handle_, add_name->sockaddr_ptr_, add_name->sockaddr_size_);
}

//��ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK
int ZCE_Socket_Base::sock_enable (int value) const
{
    return ZCE_OS::sock_enable(socket_handle_, value);
}

//�ر�ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK
int ZCE_Socket_Base::sock_disable(int value) const
{
    return ZCE_OS::sock_disable(socket_handle_, value);
}

//��ȡSocket��ѡ��
int ZCE_Socket_Base::getsockopt (int level,
                                 int optname,
                                 void *optval,
                                 socklen_t *optlen)  const
{
    return ZCE_OS::getsockopt(socket_handle_, level, optname, optval, optlen);
}

//����Socket��ѡ��
int ZCE_Socket_Base::setsockopt (int level,
                                 int optname,
                                 const void *optval,
                                 int optlen) const
{
    return ZCE_OS::setsockopt(socket_handle_,
                              level,
                              optname,
                              optval,
                              optlen);
}

//ȡ�öԶ˵ĵ�ַ��Ϣ
int ZCE_Socket_Base::getpeername (ZCE_Sockaddr *addr)  const
{
    return ZCE_OS::getpeername (socket_handle_,
                                addr->sockaddr_ptr_,
                                &addr->sockaddr_size_);

}

//ȡ�ñ��صĵ�ַ��Ϣ
int ZCE_Socket_Base::getsockname (ZCE_Sockaddr *addr)  const
{
    return ZCE_OS::getsockname (socket_handle_,
                                addr->sockaddr_ptr_,
                                &addr->sockaddr_size_);
}

//connectĳ����ַ
int ZCE_Socket_Base::connect(const ZCE_Sockaddr *addr) const
{
    return ZCE_OS::connect(socket_handle_,
                           addr->sockaddr_ptr_,
                           addr->sockaddr_size_);
}

//�������ݣ���������״̬������Ϊ
ssize_t ZCE_Socket_Base::recv (void *buf,
                               size_t len,
                               int flags) const
{
    return ZCE_OS::recv(socket_handle_,
                        buf,
                        len,
                        flags);
}

//�������ݣ���������״̬������Ϊ
ssize_t ZCE_Socket_Base::send (const void *buf,
                               size_t len,
                               int flags) const
{
    return ZCE_OS::send(socket_handle_,
                        buf,
                        len,
                        flags);
}
