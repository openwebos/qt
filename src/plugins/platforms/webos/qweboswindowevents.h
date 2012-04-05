#include <QObject>
#include <QEvent>

class QWebOSWindowEvents : public QObject
{
	Q_OBJECT

	protected:
		bool eventFilter(QObject* obj, QEvent* event);

    Q_SIGNALS:
        void signalInputFocusChanged(bool focused, QObject* obj);
};
