#ifndef TEXTUTILS_H_
#define TEXTUTILS_H_

#include <QObject>

namespace canadainc {

class TextUtils : public QObject
{
	Q_OBJECT

public:
	TextUtils(QObject* parent=NULL);
	virtual ~TextUtils();

	Q_INVOKABLE static QString zeroFill(int input, int numZeroes=3);

    /*
     * Utility function to strip html-formatting.
     * @param htmlString The string that is going to be stripped.
     */
    Q_INVOKABLE static QString plainText(QString const& htmlString);

    /**
     * @param A duration in milliseconds.
     * @return 15:12:04
     */
    Q_INVOKABLE static QString formatTime(unsigned int duration);

    Q_INVOKABLE static QString sanitize(QString const& original);

    Q_INVOKABLE static QString bytesToSize(qint64 bytes);

    static QString longestCommonSubstring(QString const& str1, QString const& str2);

    static int randInt(int low, int high);
};

} /* namespace canadainc */
#endif /* TEXTUTILS_H_ */
