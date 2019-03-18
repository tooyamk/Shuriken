#include "String.h"

namespace aurora {
	int String::UnicodeToUtf8(const wchar_t * in, ui32 inLen, char* out, ui32 outLen) {
		if (out == nullptr || in == nullptr)return -1;

		ui32 totalNum = 0;
		for (ui32 i = 0; i < inLen; ++i) {//计算转换结果实际所需长度
			wchar_t unicode = in[i];
			if (unicode >= 0x0000 && unicode <= 0x007f) {
				++totalNum;
			} else if (unicode >= 0x0080 && unicode <= 0x07ff) {
				totalNum += 2;
			} else if (unicode >= 0x0800 && unicode <= 0xffff) {
				totalNum += 3;
			}
		}
		if (outLen < totalNum) return -1;//参数有效性判断！
		//------------------------------------------------

		ui32 resultSize = 0;//用来计数输出结果的实际大小！
		auto tmp = out;
		for (ui32 i = 0; i < inLen; ++i) {
			if (resultSize > outLen) return -1;

			wchar_t unicode = in[i];
			if (unicode >= 0x0000 && unicode <= 0x007f) {
				*tmp = (char)unicode;
				++tmp;
				++resultSize;
			} else if (unicode >= 0x0080 && unicode <= 0x07ff) {
				*tmp = 0xc0 | (unicode >> 6);
				++tmp;
				*tmp = 0x80 | (unicode & (0xff >> 2));
				++tmp;
				resultSize += 2;
			} else if (unicode >= 0x0800 && unicode <= 0xffff) {
				*tmp = 0xe0 | (unicode >> 12);
				++tmp;
				*tmp = 0x80 | (unicode >> 6 & 0x00ff);
				++tmp;
				*tmp = 0x80 | (unicode & (0xff >> 2));
				++tmp;
				resultSize += 3;
			}
		}
		return resultSize;
	}

	int String::Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen) {
		if (out == nullptr || in == nullptr) return -1;

		ui32 totalNum = 0;
		auto p = in;
		for (ui32 i = 0; i < inLen; ++i) {
			if (*p >= 0x00 && *p <= 0x7f) {//说明最高位为'0'，这意味着utf8编码只有1个字节！
				++p;
				++totalNum;
			} else if ((*p & (0xe0)) == 0xc0) {//只保留最高三位，看最高三位是不是110，如果是则意味着utf8编码有2个字节！
				p += 2;
				++totalNum;
			} else if ((*p & (0xf0)) == 0xe0) {//只保留最高四位，看最高三位是不是1110，如果是则意味着utf8编码有3个字节！
				p += 3;
				++totalNum;
			}
		}
		if (outLen < totalNum) return -1;//参数有效性判断！
		//------------------------------------------------
		i32 resultSize = 0;
		p = in;
		i8* tmp = (i8*)out;
		while (*p) {
			if (*p >= 0x00 && *p <= 0x7f) {//说明最高位为'0'，这意味着utf8编码只有1个字节！
				*tmp = *p;
				tmp += 2;
				++resultSize;
			} else if ((*p & 0xe0) == 0xc0) {//只保留最高三位，看最高三位是不是110，如果是则意味着utf8编码有2个字节！
				wchar_t t = 0;
				i8 t1 = 0, t2 = 0;

				t1 = *p & (0x1f);//高位的后5位！（去除了头部的110这个标志位）
				++p;
				t2 = *p & (0x3f);//低位的后6位！（去除了头部的10这个标志位）

				*tmp = t2 | ((t1 & (0x03)) << 6);
				++tmp;
				*tmp = t1 >> 2;//留下其保留的三位
				++tmp;
				++resultSize;
			} else if ((*p & (0xf0)) == 0xe0) {//只保留最高四位，看最高三位是不是1110，如果是则意味着utf8编码有3个字节！
				wchar_t t = 0, t1 = 0, t2 = 0, t3 = 0;
				t1 = *p & (0x1f);
				++p;
				t2 = *p & (0x3f);
				++p;
				t3 = *p & (0x3f);

				*tmp = ((t2 & (0x03)) << 6) | t3;
				++tmp;
				*tmp = (t1 << 4) | (t2 >> 2);
				++tmp;
				++resultSize;
			}
			++p;
		}
		/*不考虑结束符，如果考虑则打开此段！
		*tmp = '/0';
		tmp++;
		*tmp = '/0';
		resultsize += 2;
		*/
		return resultSize;
	}
}