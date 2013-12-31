#include "zce_predefine.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"

//Ĭ�Ϲ��캯��
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (void):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in6))
{

}

//����sockaddr_in���죬
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (const sockaddr_in6 *addr):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in6))
{
    in6_addr_ = *addr;
}

//���ݵ�ַ���֣��˿ںų�ʼ������
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (const char ip_addr_str[],
                                    uint16_t port_number):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in))
{
    int ret = ZCE_OS::set_sockaddr_in6(&in6_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {

    }
}

//���ݶ˿ںţ���IP��ַ��Ϣ����
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (uint16_t port_number,
                                    const char ipv6_addr_val[16]):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in))
{
    int ret = ZCE_OS::set_sockaddr_in6(&in6_addr_, port_number, ipv6_addr_val);

    if (ret != 0)
    {

    }
}

//�������죬һ��Ҫд�������Ļ���ָ����ָ���Լ���һ����ַ�ģ�
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (const ZCE_Sockaddr_In6 &others):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in6))
{
    in6_addr_ = others.in6_addr_;
}

ZCE_Sockaddr_In6::~ZCE_Sockaddr_In6()
{

}

//���õ�ַ��Ϣ
void ZCE_Sockaddr_In6::set_sockaddr (sockaddr *addr, socklen_t len)
{
    in6_addr_ = *(reinterpret_cast<sockaddr_in6 *>(addr));
    sockaddr_size_ = len;
}

//���ݵ�ַ���֣��˿ں�����
int ZCE_Sockaddr_In6::set(const char ip_addr_str[],
                          uint16_t port_number)
{
    int ret = ZCE_OS::set_sockaddr_in6(&in6_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//�Ƚ�������ַ�Ƿ����
bool ZCE_Sockaddr_In6::operator == (const ZCE_Sockaddr_In6 &others) const
{
    //in6_addr_.sin_zero Ҫ�Ƚ�����ʱ���ˡ�

    //�Ƚϵ�ַЭ��أ���ַ���˿�
    if (others.in6_addr_.sin6_family == this->in6_addr_.sin6_family &&
        0 == memcmp(&(others.in6_addr_.sin6_addr), &(this->in6_addr_.sin6_addr), sizeof(in6_addr))  &&
        others.in6_addr_.sin6_port == this->in6_addr_.sin6_port )
    {
        return true;
    }

    return false;
}

//�Ƚ�������ַ�Ƿ����
bool ZCE_Sockaddr_In6::operator != (const ZCE_Sockaddr_In6 &others) const
{
    return !(*this == others);
}

//���IP��ַ�Ƿ����,���Ӷ˿�
bool ZCE_Sockaddr_In6::is_ip_equal (const ZCE_Sockaddr_In6 &others) const
{
    //�Ƚϵ�ַЭ��أ���ַ
    if (others.in6_addr_.sin6_family == this->in6_addr_.sin6_family &&
        0 == memcmp(&(others.in6_addr_.sin6_addr), &(this->in6_addr_.sin6_addr), sizeof(in6_addr)) )
    {
        return true;
    }

    return false;
}

//ȡ��IP��ַ��ص�������Ϣ�����õ���getnameinfo
int ZCE_Sockaddr_In6::get_name_info(char *host_name, size_t name_len) const
{
    return ZCE_OS::getnameinfo(reinterpret_cast<const sockaddr *>(&in6_addr_),
                               sizeof(sockaddr_in6),
                               host_name,
                               name_len,
                               NULL,
                               0,
                               NI_NAMEREQD);
}

//ȡ��������ص�IP��ַ��Ϣ�����õ���getaddrinfo
int ZCE_Sockaddr_In6::get_addr_info(const char *hostname, uint16_t service_port)
{
    size_t only_one_addr = 1;
    return ZCE_OS::getaddrinfo_in6ary(hostname,
                                      service_port,
                                      &only_one_addr,
                                      &in6_addr_);
}

//���IPV6�ĵ�ַ�Ƿ���IPV4�ĵ�ַӳ���
bool ZCE_Sockaddr_In6::is_v4mapped() const
{
    return ZCE_OS::is_in6_addr_v4mapped(&(in6_addr_.sin6_addr));
}

//��һ��IPV4�ĵ�ַ�õ���Ӧӳ���IPV6�ĵ�ַ��
int ZCE_Sockaddr_In6::map_from_inaddr(const ZCE_Sockaddr_In &from)
{
    return ZCE_OS::sockin_map_sockin6((from), &(in6_addr_));
}

//������IPV6�ĵ�ַ��IPV4ӳ������ģ����仹ԭΪIPV4�ĵ�ַ
int ZCE_Sockaddr_In6::mapped_to_inaddr(ZCE_Sockaddr_In &to) const
{
    return ZCE_OS::mapped_sockin6_to_sockin(&(in6_addr_), (to));
}

//����sockaddr_in
ZCE_Sockaddr_In6::operator sockaddr_in6 () const
{
    return in6_addr_;
}

//�����ڲ�const sockaddr_in��ָ�룬�������Ա��޸ģ�
ZCE_Sockaddr_In6::operator const sockaddr_in6 *() const
{
    return &in6_addr_;
}

//�����ڲ�sockaddr_in��ָ�룬�����Ա��޸ģ�
ZCE_Sockaddr_In6::operator sockaddr_in6 *()
{
    return &in6_addr_;
}
