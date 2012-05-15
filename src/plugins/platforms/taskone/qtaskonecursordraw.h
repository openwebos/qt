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

#ifndef QTASKONECURSORDRAW_H
#define QTASKONECURSORDRAW_H

#include <qobject.h>
#include <Qt>

#include "gfx_kadp.h"

#define TASKONE_SCREEN_WIDTH  1920
#define TASKONE_SCREEN_HEIGHT 1080

#define TASKONE_MOUSE_WIDTH  1280
#define TASKONE_MOUSE_HEIGHT 720


typedef struct
{
	LX_GFX_SURFACE_T* 	pCursorSurface;
	ulong				width;
	ulong				height;
	long				bitDepth;
	long				colorType;
	ulong				imageID;
	ulong				progrssImgIndex;
}CURSOR_DRAW_INFO_T;

typedef struct {
	void *pData;
	int leftLength;
}BUFFER_T;

typedef struct {
	char 	fileName[4];
	ulong 	width;
	ulong 	height;
	long	bitDepth;
	long 	colorType;
}CURSOR_FILE_INFO_T;


class QTaskOneCursorDraw : public QObject
{
public:
    QTaskOneCursorDraw();

	int setPosition(uint x, uint y, uint w, uint h,
						uint hotspot, uint gapx, uint gapy, float cursorWidthRatio,
						float cursorHeightRatio);

	int setVisible(bool visible);
	int setCursorImg(int index);

	
private:
	int Cursor_LoadPNG(CURSOR_FILE_INFO_T *pCursorFileList);
    void setConfigFBDev();
	int setClearFBDev(void);
	void initGfx();	
    void initFBDev();
    bool createCursorSurface(LX_GFX_SURFACE_T **ppSurface, 
                             unsigned int width, 
                             unsigned int height, 
                             signed int bitDepth, 
                             signed int colorType);
    bool setBackgroundColor(unsigned int x, unsigned int y,
                            unsigned int w, unsigned int h,
                            unsigned int color);
    bool update();

private:
    QString m_cursorImgPath;
};

#endif // QTASKONECURSORDRAW_H
