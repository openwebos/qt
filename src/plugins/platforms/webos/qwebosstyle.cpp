#include "qwebosstyle.h"
#include "QDebug"

int QWebOSStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    //qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
	switch(hint)
	{
		case QStyle::SH_RequestSoftwareInputPanel:
			return QStyle::RSIP_OnMouseClick;
			break;
		default:
			return QCommonStyle::styleHint(hint,option,widget,returnData);
	}
}
