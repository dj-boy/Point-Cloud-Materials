#include "stdafx.h"
#include "included.h"


// UTF-8 to Unicode16
UINT32 UTF8toUnicode16(const char* pUTF8String, char16_t* pUnicode16String, UINT32 uBufferLength){
    UINT32 length = 0;
    unsigned char nowChar = *pUTF8String;
    while (nowChar){
        // 1.0000 - 007F: 0b0XXX_XXXX
        if (nowChar < 0x80){
            // ֱ��ת��
            *pUnicode16String = static_cast<char16_t>(nowChar);
        }
        // 2.0080 - 07FF: 0b110X_XXXX__10XX_XXXX
        else if (nowChar < 0xE0){
            // ��һλȡ��5λ
            char16_t temp_char = nowChar & 0x1F;
            // ����6λ
            temp_char <<= 6;
            // �ƽ�����
            ++pUTF8String;
            nowChar = *pUTF8String;
            // �ڶ�λȡ��6λ
            temp_char |= nowChar & 0x3F;
            //��ֵ
            *pUnicode16String = temp_char;
        }
        // 3.0800 - FFFF: 0b1110_XXXX__10XX_XXXX__10XX_XXXX
        else{
            // ��һλȡ��4λ
            char16_t temp_char = nowChar & 0x0F;
            // ����6λ
            temp_char <<= 6;
            // �ƽ�����
            ++pUTF8String;
            nowChar = *pUTF8String;
            // �ڶ�λȡ��6λ
            temp_char |= nowChar & 0x3F;
            // ����6λ
            temp_char <<= 6;
            // �ƽ�����
            ++pUTF8String;
            nowChar = *pUTF8String;
            // ����λȡ��6λ
            temp_char |= nowChar & 0x3F;
            //��ֵ
            *pUnicode16String = temp_char;
        }
        // �ƽ�����
        ++pUnicode16String;
        ++length;
        ++pUTF8String;
        nowChar = *pUTF8String;
        if (length > uBufferLength){
#ifdef _DEBUG
            assert(!"length > uBufferLength");
#endif
            length = 0;
            break;
        }
    }
    return length;
}

