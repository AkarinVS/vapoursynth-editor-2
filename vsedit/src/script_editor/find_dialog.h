#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

private:
    Ui::FindDialog *ui;
    QMap<QString, bool> m_flags;

    void setFlags();

signals:

    void signalFindText(const QString & a_text, const QMap<QString, bool> &a_flagsMap);
    void signalReplaceText(const QString & a_findText, const QString & a_replaceText,
                           const QMap<QString, bool> &a_flagsMap);
    void signalReplaceAllText(const QString & a_findText, const QString & a_replaceText,
                              const QMap<QString, bool> &a_flagsMap);

public slots:

    void slotSendFindSignal();
    void slotSendReplaceSignal();
    void slotSendReplaceAllSignal();
    void slotRegExOnly(int a_regExState);

};
#endif // FIND_DIALOG_H
