// GestureBase�� ���ƻ��� ��������������ͷ�ļ�

#pragma once

// ����ʱ��API ��֤Ч�ʲ��ú���
#define GESTURE_GET_TIME(time) time = ::GetTickCount()


// ���Ʊ�Ҫ����
struct GestureDataRequired{
    union{
        // �ؽ�λ��
        Joint               jt[JointType_Count];
        // �ؽڷ���
        JointOrientation    jo[JointType_Count];
    };
    // ����״̬
    HandState   left_hand;
    // ����״̬
    HandState   right_hand;
};

// ��������
enum class GestureType{
    TYPE_NONE =0, // ��0
    TYPE_STATE, // ��ʾ״̬������
    TYPE_GROUP, // ��ʾ״̬������� 
};

// GestureBase��
class GestureBase{
public:
    // �� ��������
    virtual ~GestureBase(){};
    // ���� ˢ�º���
    // ����GestureDataRequired ������Ҫ�������ж���
    // ����true��ʾ���ƴ����ɹ�false��ʧ��
    virtual bool Update(GestureDataRequired*) = 0;
    // ��ȡ����
    virtual GestureType GetType() = 0;
    // ��ȡ��������
    virtual wchar_t* GetName() = 0;
};


// ��������
#include "GestureState.h"
#include "GesturePanzerVor00.h"


#include "GestureGroup.h"