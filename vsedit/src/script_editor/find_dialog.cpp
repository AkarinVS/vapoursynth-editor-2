#include "find_dialog.h"
#include "ui_find_dialog.h"
#include "../../common-src/settings/settings_definitions.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    ui->FindLineEdit->setFocus();
    connect(ui->findNextButton, &QPushButton::clicked, this, &FindDialog::slotSendFindSignal);
    connect(ui->replaceButton, &QPushButton::clicked, this, &FindDialog::slotSendReplaceSignal);
    connect(ui->replaceAllButton, &QPushButton::clicked, this, &FindDialog::slotSendReplaceAllSignal);
    connect(ui->regexCheckBox, &QCheckBox::stateChanged, this, &FindDialog::slotRegExOnly);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setFlags()
{
    m_flags[FIND_ID_WHOLE_WORDS] = ui->wholeWordOnlycheckBox->isChecked();
    m_flags[FIND_ID_MATCH_CASE] = ui->matchCaseCheckBox->isChecked();
    m_flags[FIND_ID_REGEX] = ui->regexCheckBox->isChecked();
}

void FindDialog::slotSendFindSignal()
{
    QString findText = ui->FindLineEdit->text();
    if (findText.isEmpty()) return;

    setFlags();
    emit signalFindText(findText, m_flags);
}

void FindDialog::slotSendReplaceSignal()
{
    QString findText = ui->FindLineEdit->text();
    if (findText.isEmpty()) return;

    setFlags();
    QString replaceText = ui->replaceLineEdit->text();
    emit signalReplaceText(findText, replaceText, m_flags);
}

void FindDialog::slotSendReplaceAllSignal()
{
    QString findText = ui->FindLineEdit->text();
    if (findText.isEmpty()) return;

    setFlags();
    QString replaceText = ui->replaceLineEdit->text();
    emit signalReplaceAllText(findText, replaceText, m_flags);
}

void FindDialog::slotRegExOnly(int a_regExState)
{
    if (ui->regexCheckBox->isChecked()) {
        ui->matchCaseCheckBox->setDisabled(true);
        ui->wholeWordOnlycheckBox->setDisabled(true);
    } else {
        ui->matchCaseCheckBox->setDisabled(false);
        ui->wholeWordOnlycheckBox->setDisabled(false);
    }
}
