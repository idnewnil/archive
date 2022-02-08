#include "textoutputdialog.h"
#include "ui_textoutputdialog.h"

TextOutputDialog::TextOutputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextOutputDialog)
{
    ui->setupUi(this);

    QFont font;
    font.setFamilies({"JetBrains Mono", "Consolas", "Courier New", "monospace"});
    font.setPointSize(12);
    ui->TextOutputBrowser->setFont(font);
    ui->TextOutputBrowser->setWordWrapMode(QTextOption::NoWrap);
}

TextOutputDialog::~TextOutputDialog()
{
    delete ui;
}

void TextOutputDialog::setResult(const QString &result) {
    ui->TextOutputBrowser->setPlainText(result);
}
