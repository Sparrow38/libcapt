#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "captFontFile.h"

namespace libCapt
{

//-------------------------------------------------------------------------------------------------
FontFile::FontFile()
	: m_pGlyphBuf(0)
	, m_pCodeBuf(0)
{
	memset(&m_fileHead, 0, sizeof(Head));
}

//-------------------------------------------------------------------------------------------------
FontFile::~FontFile()
{
	release();
}

//--------------------------------------------------------------------------
void FontFile::release(void)
{
	//свободная память
	if(m_pCodeBuf)
	{
		delete[] m_pCodeBuf;
		m_pCodeBuf = 0;
	}
	if(m_pGlyphBuf)
	{
		delete[] m_pGlyphBuf;
		m_pGlyphBuf = 0;
	}

	//重置参数
	memset(&m_fileHead, 0, sizeof(Head));
}

//--------------------------------------------------------------------------
void FontFile::_pointRuntime(void)
{
	for(unsigned int i=0; i<m_fileHead.nCodeCounts; i++)
	{
		Code& code = m_pCodeBuf[i];
		if(code.wCode==0 || code.nGlyphCounts==0) continue;  //не используется

		CodeGlyph* codeGlyph = (CodeGlyph*)(m_pGlyphBuf+code.nFirstOffset);
		while(codeGlyph->nNextOffset !=0 )
		{
			codeGlyph->pNext = (CodeGlyph*)(m_pGlyphBuf + codeGlyph->nNextOffset);
			codeGlyph = codeGlyph->pNext;
		}

		code.pFirst = (CodeGlyph*)(m_pGlyphBuf+code.nFirstOffset);
	}
}

//--------------------------------------------------------------------------
bool FontFile::_readStrem(unsigned char* dest, unsigned int size, const unsigned char*& pStream, const unsigned char* pStreamEnd)
{
	if((unsigned int)(pStreamEnd-pStream)<size) return false;
	memcpy(dest, pStream, size);
	pStream += size;
	return true;
}

//--------------------------------------------------------------------------
bool FontFile::loadFromDataStream(const unsigned char* pStream, unsigned int streamSize)
{
	if(pStream==0 || streamSize<=sizeof(m_fileHead)) return false;

	//выход данных
	release();

	const unsigned char* p = pStream;
	const unsigned char* pStreamEnd = pStream+streamSize;
	//прочтать заголовок файла
	if(!_readStrem((unsigned char*)&m_fileHead, sizeof(m_fileHead), p, pStreamEnd))
	{
		return false;
	}
	if(m_fileHead.dwMagicCode != MAGIC_CODE)
	{
		return false;
	}

	//прочтать информацию о делах
	int codeCounts = getCodeCounts();
	m_pCodeBuf = new Code[codeCounts];
	if(!_readStrem((unsigned char*)m_pCodeBuf, sizeof(Code)*codeCounts, p, pStreamEnd))
	{
		return false;
	}

	//прочитать информацию об изображении
	m_pGlyphBuf = new unsigned char[m_fileHead.nGlyphSize];
	if(!_readStrem(m_pGlyphBuf, m_fileHead.nGlyphSize, p, pStreamEnd))
	{
		return false;
	}

	//время выполнения указателя
	_pointRuntime();

	return true;
}

//--------------------------------------------------------------------------
unsigned short FontFile::getCodeFromIndex(unsigned int index) const
{
	if(index<0 || index>=getCodeCounts()) return 0;
	//вернуть кодировку юникода
	return m_pCodeBuf[index].wCode;
}

//--------------------------------------------------------------------------
char* FontFile::getCodeGlyphFromIndex(unsigned int index) const
{
	if(index<0 || index>=getCodeCounts()) return 0;

	//получить информацию о коде
	const Code& codeInfo = m_pCodeBuf[index];
	//неподдерживаемые символы
	if(codeInfo.nGlyphCounts <= 0) return 0;
	//рандомно выбрать один
	int glyphIndex=0;
	if(codeInfo.nGlyphCounts > 1)
	{
		glyphIndex = rand()%(codeInfo.nGlyphCounts);
	}

	//???
	CodeGlyph* glyph = codeInfo.pFirst;
	for(int i=0; i<glyphIndex; i++) glyph=glyph->pNext;

	return (char*)glyph->data;
}

}
