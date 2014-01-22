
#ifndef SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_
#define SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_

#include "soar_svrd_application.h"

#define REGISTER_NOTIFYTRANS_CMD(notify_trans_class, cmd_word ) \
    { \
        int __ret_register_ = MT_NOTIFY_TRANS_MANGER::instance()->register_trans_cmd(cmd_word, \
                                                                                     new notify_trans_class(ZCE_Timer_Queue::instance(),MT_NOTIFY_TRANS_MANGER::instance())); \
        if (__ret_register_ !=  SOAR_RET::SOAR_RET_SUCC) \
        { \
            return __ret_register_; \
        } \
    }

class NotifyTrans_TaskBase;

class Comm_SvrdApp_NotifyTrans : public Comm_Svrd_Appliction
{

protected:

    //ʹ����������Application
    Comm_SvrdApp_NotifyTrans();
    ~Comm_SvrdApp_NotifyTrans();

public:

    //���ӵ���register_func_cmd
    virtual int init_instance();

    //���д���,
    virtual int run_instance();

    //�˳�����
    virtual int exit_instance();

protected:

    //ע��Ҫ�����������
    //����ʹ�ú�REGISTER_NOTIFYTRANS_CMDע������
    virtual int register_notifytrans_cmd() = 0;

    //ע��notify�Ĵ����߳�,����⼸��������ֵ�˾�OK
    //clone_task���鷳��newһ������
    //task_num ��Ҫ��̫����̣߳�������OK�������д���������
    //task_stack_size ����߳������϶࣬�мǿ��ƶ�ջ��С
    virtual int register_notify_task(NotifyTrans_TaskBase *&clone_task,
                                     size_t &task_num,
                                     size_t &task_stack_size) = 0;

};

#endif //#ifndef SOARING_LIB_SVRD_APP_NOTIFY_TRNAS_H_
