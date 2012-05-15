/****************************************************************************
**
** Copyright (C) 2012 TaskOne
** All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtaskonecursordraw.h"

#include "fbdev_kadp.h"
#include "gfx_kadp.h"
#include "gfx_kapi.h"
#if 1
#include "png_kadp.h"

#include <sys/stat.h>
#include "png.h"
#endif	

#include <QtGlobal>
#include <QDir>
#include <QDebug>

#define POSD_CURSOR_ID  LX_FBDEV_ID_CSR0

#ifdef __cplusplus
extern "C" {
#endif

extern void KADP_InitSystem();

#ifdef __cplusplus
}
#endif


#define DELETE_AND_NULL(x)		{if(x) {free(x); (x) = NULL; } }





static LX_GFX_SURFACE_T m_FBSurface[2];
static CURSOR_DRAW_INFO_T *_gpCursorDrawInfo;
static CURSOR_FILE_INFO_T *gcursorFileList = NULL;

static float gCursorWidthScale = 0;
static float gCursorHeightScale = 0;

QTaskOneCursorDraw::QTaskOneCursorDraw()
{
	KADP_InitSystem();
	
    m_cursorImgPath = QDir::homePath() + "/res/lglib/cursorTypeAszMstN.png";

	qDebug() << "[cursor] " << m_cursorImgPath;

	qDebug() << "[home path] " << QDir::homePath();

    initGfx();

}

void QTaskOneCursorDraw::setConfigFBDev()
{
    LX_FBDEV_CONFIG_T fbdev_cfg;

    KADP_FBDEV_GetConfiguration(POSD_CURSOR_ID, &fbdev_cfg);


    fbdev_cfg.input_win.w = 128;
    fbdev_cfg.input_win.h = 128;

    KADP_FBDEV_SetConfiguration(POSD_CURSOR_ID, &fbdev_cfg);
}

static void CURSOR_RESOURCE_ReadbufferPNG(png_structp pstruct, png_bytep pointer, png_size_t size)
{
	BUFFER_T *pb;

	pb = (BUFFER_T*)pstruct->io_ptr;

	if(pb->leftLength==0)
		return;

	if(pb->leftLength < size)
		size = pb->leftLength;

	memcpy(pointer, pb->pData, size);
	pb->pData += size, pb->leftLength -= size;
}

static void CURSOR_CustomErrorPNG(png_structp png_ptr, png_const_charp error_message)
{
	longjmp(png_ptr->jmpbuf, 1);
}

	
static int Cursor_PNG_DATA(void *pData, int nBytes, CURSOR_FILE_INFO_T *pCursorFileList)
{
	png_structp		state;
	png_infop		pnginfo;
	int bit_depth, colortype;
	ulong twidth, theight;
	BUFFER_T		src;

	//qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);

	src.pData = pData+8;
	src.leftLength = nBytes-8;


	if(png_sig_cmp((png_byte *)pData, 0, 8))
		qDebug("JCY : %s[%d] ERROR", __FUNCTION__,__LINE__);

	if(!(state = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, CURSOR_CustomErrorPNG, NULL)))
		qDebug("JCY : %s[%d] ERROR", __FUNCTION__,__LINE__);

	if(!(pnginfo = png_create_info_struct(state)))
	{
		png_destroy_read_struct(&state, NULL, NULL);
		qDebug("JCY : %s[%d] ERROR", __FUNCTION__,__LINE__);
	}

	png_set_read_fn(state, &src, CURSOR_RESOURCE_ReadbufferPNG);


	png_set_sig_bytes(state, 8);
	png_read_info(state, pnginfo);
	png_get_IHDR(state, pnginfo, &twidth, &theight, &bit_depth, &colortype, NULL, NULL, NULL);

	png_destroy_info_struct(state, &pnginfo);
	png_destroy_read_struct(&state, NULL, NULL);

	//qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);

	pCursorFileList->width = twidth;
	pCursorFileList->height = theight;
	pCursorFileList->bitDepth = bit_depth;
	pCursorFileList->colorType = colortype;

	//qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);

	return 0;
}

int QTaskOneCursorDraw::Cursor_LoadPNG(CURSOR_FILE_INFO_T *pCursorFileList)
{
	long 		fdPng;
	struct stat sbPng;
	size_t		nBytes;
	UINT8		*pBufferPng=NULL;
	
	//qDebug("JCY : %s[%d] ", __FUNCTION__,__LINE__);
    QByteArray ba = m_cursorImgPath.toLocal8Bit();
    qDebug("Img path load png: %s", ba.data());
	if((fdPng = open(ba.data(), O_RDONLY, 0777)) < 0)
	{
		qDebug("JCY : %s[%d] - png file open Error", __FUNCTION__,__LINE__);
	}

	if(fstat(fdPng, &sbPng)<0)
	{
		qDebug("JCY : %s[%d] - fstat error", __FUNCTION__,__LINE__);
	}


	nBytes = sbPng.st_size;
	
	pBufferPng = (UINT8 *)mmap(NULL, sbPng.st_size, PROT_READ, MAP_SHARED, fdPng, 0);
	if(pBufferPng == MAP_FAILED)
		qDebug("JCY : %s[%d] - MAP_FAILED", __FUNCTION__,__LINE__);


	if(Cursor_PNG_DATA(pBufferPng, nBytes, pCursorFileList) != 0)
	{
		qDebug("JCY : %s[%d] - Cursor_LoadPNG_DATA error", __FUNCTION__,__LINE__);
		return 1;
	}

	munmap(pBufferPng, sbPng.st_size);
	close(fdPng);

	return 0;

}


void QTaskOneCursorDraw::initGfx()
{
	KADP_GFX_Open();

	KADP_PNG_Open();

    KADP_FBDEV_Open(POSD_CURSOR_ID);


    setConfigFBDev();
    

	initFBDev();

	struct stat sbPng;
	int 		fdPng;
	void*		imgPtr=NULL;
	LX_MEM_BUF_T imgDecBuf;

	int i=0;


	//jcy : Set Mouse Cursor scale
	gCursorWidthScale = (float)TASKONE_SCREEN_WIDTH / TASKONE_MOUSE_WIDTH;
	gCursorHeightScale = (float)TASKONE_SCREEN_HEIGHT / TASKONE_MOUSE_HEIGHT;

	DELETE_AND_NULL(gcursorFileList);
	gcursorFileList = (CURSOR_FILE_INFO_T *)malloc(sizeof(CURSOR_FILE_INFO_T)*2);

	Cursor_LoadPNG(&gcursorFileList[0]);

	DELETE_AND_NULL(_gpCursorDrawInfo);
	_gpCursorDrawInfo = (CURSOR_DRAW_INFO_T *)malloc(sizeof(CURSOR_DRAW_INFO_T)*2);
	for(i=0;i<1;i++)
	{
		
		if(createCursorSurface(&_gpCursorDrawInfo[i].pCursorSurface, gcursorFileList[0].width, gcursorFileList[0].height, 0, 0) != TRUE)
		{
			qDebug("JCY : %s[%d] Error", __FUNCTION__,__LINE__);
		}

        QByteArray ba = m_cursorImgPath.toLocal8Bit();
        qDebug("Img path: %s", ba.data());
		if((fdPng = open(ba.data(), O_RDONLY, 0777)) < 0)
		{
			qDebug("JCY : %s[%d] - png file open Error", __FUNCTION__,__LINE__);
		}

		if(fstat(fdPng, &sbPng)<0)
		{
			qDebug("JCY : %s[%d] - fstat error", __FUNCTION__,__LINE__);
		}

		imgPtr = (void *)mmap(NULL, sbPng.st_size, PROT_READ, MAP_SHARED, fdPng, 0);
		if(imgPtr == MAP_FAILED)
			qDebug("JCY : %s[%d] - MAP_FAILED", __FUNCTION__,__LINE__);


		imgDecBuf.addr = (UINT32)(_gpCursorDrawInfo[i].pCursorSurface->phys_mem_info.phys_addr);

		imgDecBuf.length = _gpCursorDrawInfo[i].pCursorSurface->phys_mem_info.length;

		KADP_PNG_DecodeImage((UINT8 *)imgPtr, sbPng.st_size, &imgDecBuf);

		KADP_GFX_SimpleReorderColor(0, _gpCursorDrawInfo[i].pCursorSurface,LX_COC_16_23, LX_COC_08_15, LX_COC_00_07, LX_COC_24_31, FALSE);

		if(imgPtr)
			munmap(imgPtr, sbPng.st_size);
		if(fdPng > 0)
			close(fdPng);

	}	
}

void QTaskOneCursorDraw::initFBDev()
{
    KADP_FBDEV_GetSurface(POSD_CURSOR_ID, 0, &m_FBSurface[0]);
    KADP_FBDEV_GetSurface(POSD_CURSOR_ID, 1, &m_FBSurface[1]);
}

bool QTaskOneCursorDraw::createCursorSurface(LX_GFX_SURFACE_T **ppSurface, 
                                            unsigned int width,
                                            unsigned int height,
                                            signed int bitDepth,
                                            signed int colorType)
{
    LX_GFX_SURFACE_SETTING_T surfaceSettingOption;

    if( !ppSurface )
    {
	    qDebug("JCY : %s[%d] ERROR", __FUNCTION__,__LINE__);
        return false;
    }

    *ppSurface = (LX_GFX_SURFACE_T *)malloc(sizeof(LX_GFX_SURFACE_T));

    if( *ppSurface == NULL )
    {
		qDebug("JCY : %s[%d] ERROR", __FUNCTION__,__LINE__);
		return false;
    }

    KADP_GFX_MakeSurfaceSettingEx(&surfaceSettingOption,
                                  LX_GFX_SURFACE_TYPE_MEM_BUFFER,
                                  LX_GFX_PIXEL_FORMAT_ARGB8888,
                                  width,
                                  height,
                                  4,
                                  0,
                                  0x0,
                                  0x0);

    KADP_GFX_CreateSurface(ppSurface, &surfaceSettingOption);

    KADP_GFX_SimpleClear(0, *ppSurface, 0x0);

    return true;
}

bool QTaskOneCursorDraw::setBackgroundColor(unsigned int x,
                                           unsigned int y,
                                           unsigned int w,
                                           unsigned int h,
                                           unsigned int color)
{
    signed int drawIdx;
    LX_RECT_T rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    drawIdx = KADP_FBDEV_GetDrawScreenIdx(POSD_CURSOR_ID);

    if( drawIdx < 0 )
    {
        return false;
    }

    KADP_GFX_SimpleFillRect(0, &m_FBSurface[drawIdx], &rect, color);
    update();

    return true;
}

bool QTaskOneCursorDraw::update()
{
    KADP_FBDEV_FlipScreen(POSD_CURSOR_ID);

    return true;
}

int QTaskOneCursorDraw::setPosition(uint x, uint y, uint w, uint h,
						uint hotspot, uint gapx, uint gapy, float cursorWidthRatio,
						float cursorHeightRatio)
{
	LX_FBDEV_CURSOR_POSITION_T cursor;

	cursor.pos.x = (int)((gCursorWidthScale * x + 0.5) * cursorWidthRatio);
	cursor.pos.y = (int)((gCursorWidthScale * y + 0.5) * cursorHeightRatio);
	cursor.size.w = gcursorFileList[0].width;
	cursor.size.h = gcursorFileList[0].height;
	//cursor.hotspot = 1;
	cursor.hotspot.x = gapx;
	cursor.hotspot.y = gapy;

	KADP_FBDEV_MoveCursor(POSD_CURSOR_ID, &cursor);
	return 0;
}

int QTaskOneCursorDraw::setVisible(bool visible)
{
	KADP_FBDEV_SetVisible(POSD_CURSOR_ID, visible);
	return 0;
}

int QTaskOneCursorDraw::setClearFBDev(void)
{
	long drawIdx;
	LX_RECT_T rect;

	memset(&rect, 0, sizeof(LX_RECT_T));

	rect.w = gcursorFileList[0].width;
	rect.h = gcursorFileList[0].height;

	drawIdx = KADP_FBDEV_GetDrawScreenIdx(POSD_CURSOR_ID);

	if(drawIdx < 0)
	{
		qDebug("JCY : %s[%d] - KADP_FBDEV_GetDrawScreenIdx Error", __FUNCTION__,__LINE__);
		return 1;
	}

	KADP_GFX_SimpleFillRect(0,&m_FBSurface[drawIdx],&rect,0);

	return 0;
}


int QTaskOneCursorDraw::setCursorImg(int index)
{
	LX_RECT_T 	srcRect;
	LX_RECT_T 	dstRect;
	int 		drwIdx;
	LX_GFX_SW_SCALE_SETTINGS_T	settings;
	settings.scaler_mode = LX_GFX_SW_SCALER_MODE_BILINEAR;

	if(index > 1)
	{
		qDebug("JCY : %s[%d] - setCursorImg Error", __FUNCTION__,__LINE__);
		return 1;
	}

	memset(&srcRect, 0, sizeof(LX_RECT_T));
	memset(&dstRect, 0, sizeof(LX_RECT_T));

	srcRect.w = gcursorFileList[0].width;
	srcRect.h = gcursorFileList[0].height;

	dstRect.w = srcRect.w;
	dstRect.h = srcRect.h;
	
	if(setClearFBDev() != 0)
	{
		qDebug("JCY : %s[%d] - setClearFBDev Error", __FUNCTION__,__LINE__);
		return 1;
	}

	drwIdx = KADP_FBDEV_GetDrawScreenIdx(POSD_CURSOR_ID);

	if(drwIdx < 0)
	{
		qDebug("JCY : %s[%d] - KADP_FBDEV_GetDrawScreenIdx Error", __FUNCTION__,__LINE__);
		return 1;
	}

	if(KADP_GFX_SimpleStretchBlit(0, _gpCursorDrawInfo[0].pCursorSurface, &srcRect, &m_FBSurface[drwIdx], &dstRect, &settings)!=RET_OK)
	{
		qDebug("JCY : %s[%d] - KADP_GFX_SimpleStretchBlit Error", __FUNCTION__,__LINE__);
		return 1;
	}

	update();

	return 0;
}

