#include <QStyle>
#include <QCommonStyle>
#include <QPlastiqueStyle>

class QWebOSStyle : public QPlastiqueStyle
{
	public:
		int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
};
