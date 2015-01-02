#ifndef PIMCONTACTPICKERSHEET_H_
#define PIMCONTACTPICKERSHEET_H_

#include <QFutureWatcher>
#include <QVariant>

#include <bb/cascades/pickers/ContactSelectionMode>

namespace canadainc {

using namespace bb::cascades::pickers;

class PimContactPickerSheet : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode FINAL)

    int m_mode;
    QFutureWatcher<QVariantList> m_future;

public:
    PimContactPickerSheet(QObject* parent=NULL);
    virtual ~PimContactPickerSheet();

    Q_INVOKABLE void open();
    int mode() const;
    Q_SLOT void setMode(int mode);

signals:
	void finished(QVariantList const& result);

private slots:
    void canceled();
	void contactSelected(int);
	void contactsSelected(QList<int> const& contactIds);
	void onRenderComplete();
};

} /* namespace canadainc */

#endif /* PIMCONTACTPICKERSHEET_H_ */
