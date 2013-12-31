
#ifndef ZCE_LIB_BUS_MMAP_PIPE_H_
#define ZCE_LIB_BUS_MMAP_PIPE_H_

//�������������ר�ҡ�C���Լ�Unix֮����ͼ�齱����ߵ���˹�����棨Dennis Ritchie����2011��10��12���ڼ���ȥ��������70�ꡣ

#include "zce_boost_non_copyable.h"

namespace ZCE_LIB
{
class dequechunk_node;
class shm_dequechunk;
};

//MMAP�Ĺܵ�����Ҫ��ʼ�������ͳ�ʼ������
class ZCE_Bus_MMAPPipe : public ZCE_NON_Copyable
{

public:

    //����PIPE����������ʱд��512��,�о��󲿷�ʱ��Ƚ��˷ѣ��Ǻǣ��������ˣ�Ҳ�ͼ�K�ռ�
    static const size_t MAX_NUMBER_OF_PIPE = 512;

protected:

    //
    struct ZCE_BUS_PIPE_HEAD
    {
        ZCE_BUS_PIPE_HEAD();
        ~ZCE_BUS_PIPE_HEAD();

        //�����ֳ�
        uint32_t            size_of_sizet_;
        //
        //�ܵ�����
        size_t              number_of_pipe_;
        //�ܵ����ó���,2���ܵ������ó���,
        size_t              size_of_pipe_[MAX_NUMBER_OF_PIPE];
        //
        size_t              size_of_room_[MAX_NUMBER_OF_PIPE];
    };

public:

    //���캯��,
    ZCE_Bus_MMAPPipe();
    //��������
    ~ZCE_Bus_MMAPPipe();

public:

    //��ʼ�������ݲ�����
    int initialize(const char *bus_mmap_name,
                   size_t number_of_pipe,
                   size_t size_of_pipe[],
                   size_t max_frame_len,
                   bool if_restore);

    //��ʼ����ֻ�����ļ����г�ʼ��������ĳЩ���߶�MMAP�ļ����д�����ʱ��
    int initialize(const char *bus_mmap_name,
                   size_t max_frame_len);


    //-----------------------------------------------------------------
    //����TMD����ǿ��֢�����ṩ��ô��ӿڸ������һ���㹻���ˡ�
    bool is_exist_bus(size_t pipe_id);
    //��ܵ�д��֡
    inline int push_back_bus(size_t pipe_id, const ZCE_LIB::dequechunk_node *node);
    //�ӹܵ�POP��ȡ֡��������ɾ��
    inline int pop_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *node);
    //�ӹܵ���������һ��֡����
    inline int read_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *&node);
    //����һ��֡
    inline int pop_front_bus(size_t pipe_id);
    //ȡ�ܵ�ͷ��֡��
    inline int get_front_nodesize(size_t pipe_id, size_t &note_size);


    //�ܵ�Ϊ��
    inline bool is_full_bus(size_t pipe_id);
    //�ܵ��Ƿ�Ϊ��
    inline bool is_empty_bus(size_t pipe_id);
    //�ܵ��Ŀ���ռ�,
    inline void get_bus_freesize(size_t pipe_id, size_t &pipe_size, size_t &free_size);

    //-----------------------------------------------------------------
protected:

    //��ʼ�����е����ݹܵ�
    int init_all_pipe(size_t max_frame_len,
                      bool if_restore);

public:

    ///Ϊ��SingleTon��׼��
    ///ʵ���ĸ�ֵ
    static void instance(ZCE_Bus_MMAPPipe *);
    ///ʵ���Ļ��
    static ZCE_Bus_MMAPPipe *instance();
    ///���ʵ��
    static void clean_instance();



protected:

    ///BUS�����֣�Ҳ����MMAPӳ���ļ�����
    char                       bus_mmap_name_[PATH_MAX + 1];

    ///BUS�ļ���ͷ����Ϣ
    ZCE_BUS_PIPE_HEAD          bus_head_;

    ///N���ܵ�,������չܵ�,���͹ܵ�����,���MAX_NUMBER_OF_PIPE��
    ZCE_LIB::shm_dequechunk  *bus_pipe_pointer_[MAX_NUMBER_OF_PIPE];

    ///MMAP�ڴ��ļ���
    ZCE_ShareMem_Mmap          mmap_file_;


protected:
    //instance����ʹ�õĶ���
    static ZCE_Bus_MMAPPipe *instance_;
};


//�ܵ��Ƿ����
inline bool ZCE_Bus_MMAPPipe::is_exist_bus(size_t pipe_id)
{
    return bus_pipe_pointer_[pipe_id] == NULL ? false : true;
}
//�ܵ�Ϊ��
inline bool ZCE_Bus_MMAPPipe::is_full_bus(size_t pipe_id)
{
    return bus_pipe_pointer_[pipe_id]->full();
}

//�ܵ��Ƿ�Ϊ��
inline bool ZCE_Bus_MMAPPipe::is_empty_bus(size_t pipe_id)
{
    return bus_pipe_pointer_[pipe_id]->empty();
}

//�ܵ��Ŀ���ռ�,
inline void ZCE_Bus_MMAPPipe::get_bus_freesize(size_t pipe_id, size_t &pipe_size, size_t &free_size)
{
    pipe_size = bus_head_.size_of_pipe_[pipe_id];
    free_size = bus_pipe_pointer_[pipe_id]->freesize();
    return;
}


//��ܵ�д��֡
inline int ZCE_Bus_MMAPPipe::push_back_bus(size_t pipe_id, const ZCE_LIB::dequechunk_node *node)
{

    //ȡ��һ��֡
    bool bret = bus_pipe_pointer_[pipe_id]->push_end(node);

    //
    if (!bret)
    {
        ZLOG_ALERT("[zcelib] %u Pipe is full or data small?,Some data can't put to pipe. Please increase and check. nodesize=%u, freesize=%u",
                   pipe_id, node->size_of_node_,
                   (unsigned int)bus_pipe_pointer_[pipe_id]->freesize());
        return -1;
    }

    return 0;
}

//ȡ�ܵ�ͷ��֡��
inline int ZCE_Bus_MMAPPipe::get_front_nodesize(size_t pipe_id, size_t &note_size)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    note_size = bus_pipe_pointer_[pipe_id]->get_front_len();
    return 0;
}




/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006��4��14��
Function        : ZCE_Bus_MMAPPipe::pop_front_bus
Return          : inline int
Parameter List  :
  Param1: ZERG_PIPE_ID pipe_id �ܵ�ID
  Param2: dequechunk_node* const pframe ׼������nodeָ�룬ָ���ǲ����
Description     : �ӹܵ���ȡ֡
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
inline int ZCE_Bus_MMAPPipe::pop_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *node)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    //ȡ��һ��֡
    bus_pipe_pointer_[pipe_id]->pop_front(node);

    return 0;
}

//�ӹܵ���������һ��֡����
inline int ZCE_Bus_MMAPPipe::read_front_bus(size_t pipe_id, ZCE_LIB::dequechunk_node *&node)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    //ȡ��һ��֡
    bus_pipe_pointer_[pipe_id]->read_front(node);

    return 0;
}

//����һ��֡
inline int ZCE_Bus_MMAPPipe::pop_front_bus(size_t pipe_id)
{
    if (bus_pipe_pointer_[pipe_id]->empty())
    {
        return -1;
    }

    //ȡ��һ��֡
    bus_pipe_pointer_[pipe_id]->discard_frond();
    return 0;
}









#endif //ZCE_LIB_BUS_MMAP_PIPE_H_




