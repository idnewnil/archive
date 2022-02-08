#ifndef TEXTOUTPUTDIALOG_H
#define TEXTOUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
class TextOutputDialog;
}

class TextOutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextOutputDialog(QWidget *parent = nullptr);
    ~TextOutputDialog();
    void setResult(const QString &result);

private:
    Ui::TextOutputDialog *ui;
};

#endif // TEXTOUTPUTDIALOG_H
