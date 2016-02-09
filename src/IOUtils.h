#ifndef IOUTILS_H_
#define IOUTILS_H_

#include <QRunnable>
#include <QMap>
#include <QStringList>

#define NEW_LINE "\r\n"
#define LINE_SEPARATOR "\n"
#define directory_local_shared "/accounts/1000/shared"
#define directory_sdcard "/accounts/1000/removable/sdcard"
#define directory_temp "/var/tmp"

namespace canadainc {

class IOUtils
{
public:
	static void startThread(QRunnable* qr, bool autoDelete=true);
	static bool writeFile(QString const& filePath, QByteArray const& data=QByteArray(), bool replace=true);

	/**
	 * @param replace If this is <code>false</code> then we will just append to the text file.
	 */
	static bool writeTextFile(QString const& filePath, QString contents, bool replace=true, bool correctNewLines=true, bool log=true);

	static QStringList executeCommand(QString const& command);

	static void clearAllCache();

	static bool removeDir(QString const& dirName);

	static QString getMd5(QByteArray const& input);
	static bool writeIfValidMd5(QString const& filePath, QString const& expectedMd5, QByteArray const& data, bool replace=true);
};

} /* namespace canadainc */
#endif /* IOUTILS_H_ */
