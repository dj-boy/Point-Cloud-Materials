// GesturePanzerVor00�� ����PanzerVor�ĵ�һ��״̬

#pragma once

class GesturePanzerVor00 : public GestureStateBase{
public:
    // GesturePanzerVor00
    // ��������
    virtual ~GesturePanzerVor00(){};
    // ��ȡ��������
    wchar_t* GetName(){ return L"GesturePanzerVor00"; };
    // ˢ�� ����: �ؽ�״̬
    bool Update(GestureDataRequired* data){
        return false;
    }
};